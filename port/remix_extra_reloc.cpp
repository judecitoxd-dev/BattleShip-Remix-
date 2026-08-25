#include "remix_extra_reloc.h"

#include "remix_extra_source.h"
#include "remix_extra_vpk0.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <mutex>
#include <vector>

namespace {

constexpr uint64_t kTableOffset = REMIX_EXTRA_RELOC_TABLE_ROM_OFFSET;
constexpr uint64_t kDataOffset = REMIX_EXTRA_RELOC_DATA_ROM_OFFSET;
constexpr uint32_t kFileCount = REMIX_EXTRA_RELOC_FILE_COUNT;
constexpr uint64_t kEntrySize = 12;
constexpr uint64_t kRomSize = 80312584ULL;
constexpr size_t kTableBytes = static_cast<size_t>(kFileCount + 1u) * static_cast<size_t>(kEntrySize);
static_assert(kTableOffset + kTableBytes == kDataOffset,
              "Remix + EXTRA table profile must end at RELOC data start");

std::once_flag gTableOnce;
std::array<uint8_t, kTableBytes> gTable{};
bool gTableValid = false;

uint16_t be16(const uint8_t* p) {
    return static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) |
                                  static_cast<uint16_t>(p[1]));
}

uint32_t be32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) |
            static_cast<uint32_t>(p[3]);
}

void loadTable() {
    if (!remix_extra_source_exists()) return;
    if (remix_extra_source_read(kTableOffset, gTable.data(), gTable.size()) != gTable.size()) return;

    // Cheap native profile validation. Android's importer will SHA-1 verify
    // on import; these invariants catch a replaced/truncated source before
    // any asset is passed to the normal relocation pipeline.
    const uint32_t firstWord = be32(gTable.data());
    const uint8_t* sentinel = gTable.data() + static_cast<size_t>(kFileCount) * kEntrySize;
    const uint32_t sentinelFirst = be32(sentinel + 0);
    const uint16_t sentinelRI = be16(sentinel + 4);
    const uint16_t sentinelCW = be16(sentinel + 6);
    const uint16_t sentinelRE = be16(sentinel + 8);
    const uint16_t sentinelDW = be16(sentinel + 10);

    if ((firstWord & 0x7FFFFFFFu) != 0u) return;
    if (sentinelFirst & 0x80000000u) return;
    if (sentinelRI != 0 || sentinelCW != 0 || sentinelRE != 0 || sentinelDW != 0) return;
    const uint64_t endOffset = kDataOffset + static_cast<uint64_t>(sentinelFirst & 0x7FFFFFFFu);
    if (endOffset > kRomSize) return;

    gTableValid = true;
}

bool ensureTable() {
    std::call_once(gTableOnce, loadTable);
    return gTableValid;
}

bool readTableEntry(uint32_t index, uint32_t& firstWord,
                    uint16_t& relocIntern, uint16_t& compressedWords,
                    uint16_t& relocExtern, uint16_t& decompressedWords) {
    if (index > kFileCount || !ensureTable()) return false; // +1 sentinel entry allowed
    const uint8_t* raw = gTable.data() + static_cast<size_t>(index) * kEntrySize;
    firstWord = be32(raw + 0);
    relocIntern = be16(raw + 4);
    compressedWords = be16(raw + 6);
    relocExtern = be16(raw + 8);
    decompressedWords = be16(raw + 10);
    return true;
}

bool getInfo(uint32_t fileId, RemixExtraRelocInfo& out,
             uint32_t* nextDataRelative = nullptr) {
    if (!remix_extra_source_exists() || fileId >= kFileCount) return false;

    uint32_t first = 0, nextFirst = 0;
    uint16_t ri = 0, cw = 0, re = 0, dw = 0;
    uint16_t dummy16 = 0;
    if (!readTableEntry(fileId, first, ri, cw, re, dw)) return false;
    if (!readTableEntry(fileId + 1, nextFirst, dummy16, dummy16, dummy16, dummy16)) return false;

    const uint32_t dataRelative = first & 0x7FFFFFFFu;
    const uint32_t nextRelative = nextFirst & 0x7FFFFFFFu;
    const uint64_t dataRom = kDataOffset + static_cast<uint64_t>(dataRelative);
    const uint32_t compressedBytes = static_cast<uint32_t>(cw) * 4u;
    const uint32_t decompressedBytes = static_cast<uint32_t>(dw) * 4u;

    if (nextRelative < dataRelative) return false;
    if (compressedBytes > nextRelative - dataRelative) return false;
    if (dataRom > kRomSize || compressedBytes > kRomSize - dataRom) return false;

    const uint32_t trailerBytes = nextRelative - dataRelative - compressedBytes;
    if ((trailerBytes & 1u) != 0) return false;

    out.file_id = fileId;
    out.is_compressed = (first & 0x80000000u) ? 1 : 0;
    out.data_rom_offset = dataRom;
    out.compressed_size = compressedBytes;
    out.decompressed_size = decompressedBytes;
    out.reloc_intern_offset = ri;
    out.reloc_extern_offset = re;
    out.extern_count = trailerBytes / 2u;
    if (nextDataRelative != nullptr) *nextDataRelative = nextRelative;
    return true;
}

} // namespace

extern "C" int remix_extra_reloc_get_info(uint32_t file_id,
                                            RemixExtraRelocInfo* out_info) {
    if (out_info == nullptr) return 0;
    RemixExtraRelocInfo info{};
    if (!getInfo(file_id, info)) return 0;
    *out_info = info;
    return 1;
}

extern "C" uint32_t remix_extra_reloc_new_file_count(void) {
    return REMIX_EXTRA_RELOC_FILE_COUNT - REMIX_EXTRA_VANILLA_RELOC_COUNT;
}

extern "C" int remix_extra_reloc_is_figatree(uint32_t file_id) {
    RemixExtraRelocInfo info{};
    if (!getInfo(file_id, info)) return 0;
    return (!info.is_compressed &&
            info.reloc_extern_offset == 0xFFFFu &&
            info.reloc_intern_offset <= 2u) ? 1 : 0;
}

extern "C" size_t remix_extra_reloc_get_extern_ids(uint32_t file_id,
                                                     uint16_t* dst,
                                                     size_t dst_count) {
    RemixExtraRelocInfo info{};
    if (!getInfo(file_id, info)) return 0;
    if (info.extern_count == 0) return 0;
    if (dst == nullptr || dst_count < info.extern_count) return 0;

    const uint64_t externOffset = info.data_rom_offset + info.compressed_size;
    std::vector<uint8_t> raw(static_cast<size_t>(info.extern_count) * 2u);
    if (remix_extra_source_read(externOffset, raw.data(), raw.size()) != raw.size()) return 0;

    for (uint32_t i = 0; i < info.extern_count; ++i) {
        dst[i] = be16(raw.data() + static_cast<size_t>(i) * 2u);
        if (dst[i] >= REMIX_EXTRA_RELOC_FILE_COUNT) return 0;
    }
    return info.extern_count;
}

extern "C" size_t remix_extra_reloc_extract(uint32_t file_id,
                                              void* dst,
                                              size_t dst_capacity) {
    if (dst == nullptr) return 0;

    RemixExtraRelocInfo info{};
    if (!getInfo(file_id, info)) return 0;
    if (info.decompressed_size == 0 || dst_capacity < info.decompressed_size) return 0;

    std::vector<uint8_t> packed(info.compressed_size);
    if (packed.empty()) return 0;
    if (remix_extra_source_read(info.data_rom_offset, packed.data(), packed.size()) != packed.size()) {
        return 0;
    }

    if (info.is_compressed) {
        const size_t decoded = remix_extra_vpk0_decode(
            packed.data(), packed.size(), dst, dst_capacity);
        return decoded == info.decompressed_size ? decoded : 0;
    }

    if (info.compressed_size < info.decompressed_size) return 0;
    std::copy_n(packed.data(), info.decompressed_size, static_cast<uint8_t*>(dst));
    return info.decompressed_size;
}
