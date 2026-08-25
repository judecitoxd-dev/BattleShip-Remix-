#include "remix_extra_game_bridge.h"

#include "remix_extra_reloc.h"

#include <cstdint>
#include <vector>

extern "C" void portRelocLoadFileFromBytes(
    unsigned int file_id,
    void* ram_dst,
    unsigned int bytes_num,
    int loc,
    const void* src_bytes,
    unsigned int src_size,
    unsigned short reloc_intern_offset,
    unsigned short reloc_extern_offset,
    const unsigned short* extern_file_ids,
    unsigned int extern_count,
    int force_figatree_fixup);

extern "C" size_t remix_extra_game_reloc_size(uint32_t file_id) {
    RemixExtraRelocInfo info{};
    if (!remix_extra_reloc_get_info(file_id, &info)) return 0;
    return info.decompressed_size;
}

extern "C" int remix_extra_game_load_reloc(uint32_t file_id,
                                             void* ram_dst,
                                             uint32_t bytes_num,
                                             int loc,
                                             int force_figatree_fixup) {
    if (ram_dst == nullptr) return 0;

    RemixExtraRelocInfo info{};
    if (!remix_extra_reloc_get_info(file_id, &info)) return 0;
    if (info.decompressed_size == 0 || bytes_num < info.decompressed_size) return 0;

    std::vector<uint8_t> data(info.decompressed_size);
    if (remix_extra_reloc_extract(file_id, data.data(), data.size()) != data.size()) {
        return 0;
    }

    std::vector<uint16_t> externIds(info.extern_count);
    if (info.extern_count != 0 &&
        remix_extra_reloc_get_extern_ids(file_id, externIds.data(), externIds.size()) != externIds.size()) {
        return 0;
    }

    portRelocLoadFileFromBytes(
        file_id,
        ram_dst,
        bytes_num,
        loc,
        data.data(),
        static_cast<unsigned int>(data.size()),
        info.reloc_intern_offset,
        info.reloc_extern_offset,
        externIds.empty() ? nullptr : externIds.data(),
        static_cast<unsigned int>(externIds.size()),
        force_figatree_fixup);
    return 1;
}
