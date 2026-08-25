#include "remix_fighters.h"
#include "fighter_registry.h"
#include "port_log.h"
#include "remix_extra_game_bridge.h"
#include "remix_extra_motion.h"
#include "remix_extra_reloc.h"
#include "remix_extra_source.h"

#include <ft/fttypes.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

static const PortRemixExtraFighterInfo kExtraFighters[] = {
    { PORT_REMIX_FKIND_BIRDO,        PORT_VANILLA_FKIND_JIGGLYPUFF, "Birdo",       "Birdo",            6,  "Birdo",       45.0f, 0.75f, 180.0f, 0x154F, 0x00F6, 0, 0x1550, 0x0154, { 0x1551, 0,      0,      0      } },
    { PORT_REMIX_FKIND_CB_KNUCKLES,  PORT_VANILLA_FKIND_FOX,        "CBKnuckles",  "Knuckles",         6,  "Knuckles",    25.0f, 0.75f, 180.0f, 0x1579, 0x00D0, 0, 0x157A, 0x013A, { 0x157B, 0x015A, 0x00A1, 0x157C } },
    { PORT_REMIX_FKIND_CBM_KNUCKLES, PORT_VANILLA_FKIND_FOX,        "CBMKnuckles", "Knuckles",         6,  "Knuckles",    25.0f, 0.75f, 180.0f, 0x15E9, 0x00D0, 0, 0x15EA, 0x013A, { 0x15EB, 0x015A, 0x00A1, 0x15EC } },
    { PORT_REMIX_FKIND_CLOUD,        PORT_VANILLA_FKIND_LINK,       "Cloud",       "Cloud",            6,  "Cloud",       30.0f, 1.00f, 170.0f, 0x165A, 0x00E0, 0, 0x165B, 0x0147, { 0x165C, 0,      0,      0x165D } },
    { PORT_REMIX_FKIND_DK_ULT,       PORT_VANILLA_FKIND_DONKEY,     "DKUlt",       "Ultimate DK",      5,  "DK ULT",      30.0f, 1.00f, 180.0f, 0x16B5, 0x00D4, 0, 0x16B6, 0x013E, { 0,      0,      0,      0      } },
    { PORT_REMIX_FKIND_KAZUYA,       PORT_VANILLA_FKIND_CAPTAIN,    "Kazuya",      "Kazuya",           6,  "Kazuya",      25.0f, 0.78f, 175.0f, 0x16D6, 0x00EB, 0, 0x16D7, 0x014E, { 0,      0,      0,      0      } },
    { PORT_REMIX_FKIND_KEN,          PORT_VANILLA_FKIND_CAPTAIN,    "Ken",         "Ken",              8,  "Ken",         50.0f, 1.00f, 160.0f, 0x1723, 0x00EB, 0, 0x1724, 0x1728, { 0x1725, 0x1726, 0x014D, 0x1727 } },
    { PORT_REMIX_FKIND_KNUCKLES,     PORT_VANILLA_FKIND_FOX,        "Knuckles",    "Knuckles",         6,  "Knuckles",    25.0f, 0.75f, 180.0f, 0x1792, 0x00D0, 0, 0x1793, 0x013A, { 0x1794, 0x015A, 0x00A1, 0x1795 } },
    { PORT_REMIX_FKIND_M_KNUCKLES,   PORT_VANILLA_FKIND_FOX,        "MKnuckles",   "Knuckles",         6,  "Knuckles",    25.0f, 0.75f, 180.0f, 0x1802, 0x00D0, 0, 0x1803, 0x013A, { 0x1804, 0x015A, 0x00A1, 0x1805 } },
    { PORT_REMIX_FKIND_MR_GAW,       PORT_VANILLA_FKIND_MARIO,      "MRGAW",       "Mr. Game & Watch", 7,  "Mr. G&W",      25.0f, 0.80f, 180.0f, 0x1873, 0x00CA, 0, 0x1874, 0x012A, { 0x1875, 0x0164, 0x1876, 0      } },
    { PORT_REMIX_FKIND_MR_GAW_PLUS,  PORT_VANILLA_FKIND_MARIO,      "MRGAWPLUS",   "Mr. Game & Watch", 7,  "Mr. G&W",      25.0f, 0.50f, 180.0f, 0x1910, 0x00CA, 0, 0x1911, 0x012A, { 0x1912, 0x0164, 0x1913, 0      } },
    { PORT_REMIX_FKIND_MR_GAW_3D,    PORT_VANILLA_FKIND_MARIO,      "MRGAWTHREED", "Mr. Game & Watch", 12, "Mr. G&W",      25.0f, 0.40f, 180.0f, 0x19AE, 0x00CA, 0, 0x19AF, 0x012A, { 0x19B0, 0x0164, 0x19B1, 0      } },
    { PORT_REMIX_FKIND_META_KNIGHT,  PORT_VANILLA_FKIND_JIGGLYPUFF, "MetaKnight",  "Meta Knight",      6,  "Meta Knight",  20.0f, 0.55f, 180.0f, 0x1A4A, 0x00E8, 0, 0x1A4B, 0x014B, { 0,      0,      0,      0      } },
    { PORT_REMIX_FKIND_REBECCA,      PORT_VANILLA_FKIND_FOX,        "Rebecca",     "Rebecca",          5,  "Rebecca",     20.0f, 0.90f, 185.0f, 0x1AAB, 0x00D0, 0, 0x1AAC, 0x013A, { 0x1AAD, 0x1AAE, 0x1AAF, 0x1AB0 } },
    { PORT_REMIX_FKIND_RYU,          PORT_VANILLA_FKIND_CAPTAIN,    "Ryu",         "Ryu",              8,  "Ryu",         50.0f, 1.00f, 160.0f, 0x1AB1, 0x00EB, 0, 0x1AB2, 0x1AB5, { 0x1AB3, 0x015E, 0x014D, 0x1AB4 } },
    { PORT_REMIX_FKIND_SNAKE,        PORT_VANILLA_FKIND_CAPTAIN,    "Snake",       "Snake",            7,  "Snake",       30.0f, 1.00f, 170.0f, 0x1B19, 0x00EB, 0, 0x1B1A, 0x1B22, { 0x1B1B, 0x1B1C, 0,      0      } },
    { PORT_REMIX_FKIND_SPIDERMAN,    PORT_VANILLA_FKIND_CAPTAIN,    "Spiderman",   "Spider-Man",       8,  "Spider-Man",  20.0f, 0.65f, 185.0f, 0x1B93, 0x00EB, 0, 0x1B94, 0x1B97, { 0x1B95, 0x015E, 0x014D, 0x1B96 } },
    { PORT_REMIX_FKIND_TERRY,        PORT_VANILLA_FKIND_CAPTAIN,    "Terry",       "Terry",            7,  "Terry",       30.0f, 1.00f, 170.0f, 0x1C3B, 0x00EB, 0, 0x1C3C, 0x1C41, { 0x1C3D, 0x1C3E, 0x1C3F, 0x1C40 } },
    { PORT_REMIX_FKIND_YOUNG_ZELDA,  PORT_VANILLA_FKIND_FOX,        "YZelda",      "Young Zelda",      9,  "Young Zelda", 20.0f, 0.55f, 180.0f, 0x1CD5, 0x00D0, 0, 0x1CD6, 0x1CD9, { 0x1CD7, 0x015A, 0x00A1, 0x1CD8 } },
};

/* Physical ROM offsets of Character.define_character's generated 0x78-byte
 * structs in the exact target ROM. These are profile metadata, not asset data. */
static const uint32_t kExtraCharacterStructRomOffsets[] = {
    0x038E2120u, 0x038E2DE0u, 0x038E3D50u, 0x038E4CC0u, 0x038E5A70u,
    0x038E68B0u, 0x038E78E0u, 0x038E88F0u, 0x038E9860u, 0x038EA7D0u,
    0x038EB5C0u, 0x038EC3D0u, 0x038ED1E0u, 0x038EE050u, 0x038EEE10u,
    0x038EFDC0u, 0x038F10F0u, 0x038F1EE0u, 0x038F2EB0u,
};

constexpr int kExtraFighterCount =
    static_cast<int>(sizeof(kExtraFighters) / sizeof(kExtraFighters[0]));
static_assert(kExtraFighterCount ==
              static_cast<int>(sizeof(kExtraCharacterStructRomOffsets) /
                               sizeof(kExtraCharacterStructRomOffsets[0])));

struct MotionImportStats {
    int sentinel = 0;
    int relative = 0;
    int parent_absolute = 0;
    int remix_absolute = 0;
};

struct NativeFTDataRuntime {
    FTData data{};
    void* file_main = nullptr;
    void* file_mainmotion = nullptr;
    void* file_submotion = nullptr;
    void* file_model = nullptr;
    void* file_special1 = nullptr;
    void* file_special2 = nullptr;
    void* file_special3 = nullptr;
    void* file_special4 = nullptr;
    s32 particle_bank = 0;
    s32 submotion_count = 0;
    std::vector<FTMotionDesc> mainmotion;
    std::vector<FTMotionDesc> submotion;
};

NativeFTDataRuntime sNativeFTData[kExtraFighterCount]{};

static uint32_t ReadBE32(const uint8_t* p)
{
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) |
           static_cast<uint32_t>(p[3]);
}

static bool PatchPointerToRomOffset(uint32_t pointer, uint64_t* out_rom)
{
    if (out_rom == nullptr || pointer < REMIX_EXTRA_PATCH_RAM_ROM_DELTA) {
        return false;
    }
    *out_rom = static_cast<uint64_t>(pointer - REMIX_EXTRA_PATCH_RAM_ROM_DELTA);
    return true;
}

const PortRemixExtraFighterInfo* FindFighter(int fkind)
{
    if (fkind < PORT_REMIX_FKIND_BIRDO || fkind > PORT_REMIX_FKIND_YOUNG_ZELDA) {
        return nullptr;
    }
    const int index = fkind - PORT_REMIX_FKIND_BIRDO;
    if (index < 0 || index >= kExtraFighterCount) {
        return nullptr;
    }
    const PortRemixExtraFighterInfo* info = &kExtraFighters[index];
    return (info->fkind == fkind) ? info : nullptr;
}

int FighterIndex(int fkind)
{
    const PortRemixExtraFighterInfo* info = FindFighter(fkind);
    return (info == nullptr) ? -1 : (fkind - PORT_REMIX_FKIND_BIRDO);
}

int ResolveAssetFileId(const PortRemixExtraFighterInfo& info, int slot)
{
    switch (slot) {
        case PORT_REMIX_ASSET_MAIN:      return info.main_file_id;
        case PORT_REMIX_ASSET_PRIMARY:   return info.primary_file_id;
        case PORT_REMIX_ASSET_SECONDARY: return info.secondary_file_id;
        case PORT_REMIX_ASSET_CHARACTER: return info.character_file_id;
        case PORT_REMIX_ASSET_SHIELD:    return info.shield_file_id;
        case PORT_REMIX_ASSET_MISC0:     return info.misc_file_id[0];
        case PORT_REMIX_ASSET_MISC1:     return info.misc_file_id[1];
        case PORT_REMIX_ASSET_MISC2:     return info.misc_file_id[2];
        case PORT_REMIX_ASSET_MISC3:     return info.misc_file_id[3];
        default:                         return -1;
    }
}

bool ValidateRelocId(const PortRemixExtraFighterInfo& info,
                     const char* slot_name, int file_id)
{
    if (file_id == 0) {
        return true;
    }
    RemixExtraRelocInfo reloc{};
    if (!remix_extra_reloc_get_info(static_cast<uint32_t>(file_id), &reloc)) {
        port_log("SSB64 Remix: %s has invalid %s RELOC id 0x%04X\n",
                 info.display_name, slot_name, file_id);
        return false;
    }
    if (reloc.file_id != static_cast<uint32_t>(file_id) || reloc.decompressed_size == 0) {
        port_log("SSB64 Remix: %s %s RELOC 0x%04X has invalid metadata\n",
                 info.display_name, slot_name, file_id);
        return false;
    }
    return true;
}

bool ValidateRowAssets(const PortRemixExtraFighterInfo& info)
{
    bool ok = true;
    ok &= ValidateRelocId(info, "main", info.main_file_id);
    ok &= ValidateRelocId(info, "primary", info.primary_file_id);
    ok &= ValidateRelocId(info, "secondary", info.secondary_file_id);
    ok &= ValidateRelocId(info, "character", info.character_file_id);
    ok &= ValidateRelocId(info, "shield", info.shield_file_id);
    for (int i = 0; i < 4; ++i) {
        ok &= ValidateRelocId(info, "misc", info.misc_file_id[i]);
    }
    return ok;
}

static intptr_t ImportMotionOffset(const PortRemixExtraFighterInfo& info,
                                   const char* table_name,
                                   int motion_index,
                                   uint32_t raw_offset,
                                   const FTMotionDescArray* parent_table,
                                   int parent_count,
                                   MotionImportStats* stats,
                                   bool* ok)
{
    if (raw_offset == 0x80000000u) {
        if (stats != nullptr) stats->sentinel++;
        return static_cast<intptr_t>(raw_offset);
    }
    if (raw_offset <= 0x00100000u) {
        if (stats != nullptr) stats->relative++;
        return static_cast<intptr_t>(raw_offset);
    }

    if (remix_extra_motion_contains(raw_offset)) {
        void* host = remix_extra_motion_resolve(raw_offset);
        if (host == nullptr) {
            port_log("SSB64 Remix: %s %s[%d] cannot resolve motion address 0x%08X\n",
                     info.display_name, table_name, motion_index, raw_offset);
            if (ok != nullptr) *ok = false;
            return 0;
        }
        if (stats != nullptr) stats->remix_absolute++;
        return reinterpret_cast<intptr_t>(host);
    }

    /* The generated arrays retain a handful of vanilla absolute script
     * symbols (~0x8039xxxx). BattleShip already has native versions of those
     * rows in the parent descriptor, so reuse the parent's translated offset
     * instead of ever exposing an N64 code/data address to the host. */
    if (raw_offset > 0x80000000u && raw_offset < 0x80400000u &&
        parent_table != nullptr && motion_index >= 0 && motion_index < parent_count) {
        if (stats != nullptr) stats->parent_absolute++;
        return parent_table->motion_desc[motion_index].offset;
    }

    port_log("SSB64 Remix: %s %s[%d] unsupported motion offset 0x%08X\n",
             info.display_name, table_name, motion_index, raw_offset);
    if (ok != nullptr) *ok = false;
    return 0;
}

static bool ImportMotionTable(const PortRemixExtraFighterInfo& info,
                              const char* table_name,
                              uint32_t patch_pointer,
                              int count,
                              const FTMotionDescArray* parent_table,
                              int parent_count,
                              std::vector<FTMotionDesc>* output,
                              MotionImportStats* stats,
                              size_t* largest_anim)
{
    if (output == nullptr || count < 0 || count > 4096) {
        return false;
    }
    output->clear();
    if (count == 0) {
        return true;
    }

    uint64_t rom_offset = 0;
    if (!PatchPointerToRomOffset(patch_pointer, &rom_offset)) {
        port_log("SSB64 Remix: %s %s table has invalid patch pointer 0x%08X\n",
                 info.display_name, table_name, patch_pointer);
        return false;
    }

    std::vector<uint8_t> raw(static_cast<size_t>(count) * 12u);
    if (remix_extra_source_read(rom_offset, raw.data(), raw.size()) != raw.size()) {
        port_log("SSB64 Remix: %s %s table short read ptr=0x%08X count=%d\n",
                 info.display_name, table_name, patch_pointer, count);
        return false;
    }

    output->resize(static_cast<size_t>(count));
    bool ok = true;
    for (int i = 0; i < count; ++i) {
        const uint8_t* row = raw.data() + static_cast<size_t>(i) * 12u;
        const uint32_t anim_file_id = ReadBE32(row + 0);
        const uint32_t raw_offset = ReadBE32(row + 4);
        const uint32_t anim_desc = ReadBE32(row + 8);

        FTMotionDesc& dst = (*output)[static_cast<size_t>(i)];
        dst.anim_file_id = anim_file_id;
        dst.offset = ImportMotionOffset(info, table_name, i, raw_offset,
                                        parent_table, parent_count, stats, &ok);
        dst.anim_desc.word = anim_desc;

        if (largest_anim != nullptr && anim_file_id != 0 &&
            !dst.anim_desc.flags.is_use_shieldpose) {
            const size_t anim_size = remix_extra_game_reloc_size(anim_file_id);
            *largest_anim = std::max(*largest_anim, anim_size);
        }
    }
    return ok;
}

FTData* BuildNativeFTData(const PortRemixExtraFighterInfo& info,
                          const FighterDescriptor& parent)
{
    const int index = FighterIndex(info.fkind);
    if (index < 0 || parent.ft_data == nullptr || !remix_extra_motion_init()) {
        return nullptr;
    }

    uint8_t raw_struct[0x78]{};
    const uint32_t struct_rom = kExtraCharacterStructRomOffsets[index];
    if (remix_extra_source_read(struct_rom, raw_struct, sizeof(raw_struct)) != sizeof(raw_struct)) {
        port_log("SSB64 Remix: cannot read Character struct for %s at 0x%08X\n",
                 info.display_name, struct_rom);
        return nullptr;
    }

    /* Refuse to build against a mismatched ROM/profile even if a stale source
     * happened to pass a superficial size check. */
    const int expected_ids[9] = {
        info.main_file_id, info.primary_file_id, info.secondary_file_id,
        info.character_file_id, info.shield_file_id,
        info.misc_file_id[0], info.misc_file_id[1],
        info.misc_file_id[2], info.misc_file_id[3]
    };
    for (int i = 0; i < 9; ++i) {
        if (ReadBE32(raw_struct + i * 4) != static_cast<uint32_t>(expected_ids[i])) {
            port_log("SSB64 Remix: %s Character struct file-id mismatch slot=%d\n",
                     info.display_name, i);
            return nullptr;
        }
    }

    const uint32_t particle_script_lo = ReadBE32(raw_struct + 0x50);
    const uint32_t particle_script_hi = ReadBE32(raw_struct + 0x54);
    const uint32_t particle_texture_lo = ReadBE32(raw_struct + 0x58);
    const uint32_t particle_texture_hi = ReadBE32(raw_struct + 0x5C);
    const uint32_t attributes_offset = ReadBE32(raw_struct + 0x60);
    const uint32_t mainmotion_pointer = ReadBE32(raw_struct + 0x64);
    const uint32_t submotion_pointer = ReadBE32(raw_struct + 0x68);
    const uint32_t mainmotion_count_u32 = ReadBE32(raw_struct + 0x6C);
    const uint32_t submotion_count_pointer = ReadBE32(raw_struct + 0x70);

    if (mainmotion_count_u32 > 4096u) {
        port_log("SSB64 Remix: %s mainmotion count is unreasonable: %u\n",
                 info.display_name, mainmotion_count_u32);
        return nullptr;
    }

    uint64_t submotion_count_rom = 0;
    uint32_t submotion_count_u32 = 0;
    if (!PatchPointerToRomOffset(submotion_count_pointer, &submotion_count_rom) ||
        !remix_extra_source_read_be32(submotion_count_rom, &submotion_count_u32) ||
        submotion_count_u32 > 4096u) {
        port_log("SSB64 Remix: %s invalid submotion count pointer 0x%08X\n",
                 info.display_name, submotion_count_pointer);
        return nullptr;
    }

    NativeFTDataRuntime& runtime = sNativeFTData[index];
    runtime = NativeFTDataRuntime{};
    runtime.data = *parent.ft_data;

    const int parent_main_count = std::max(parent.ft_data->mainmotion_array_count, 0);
    const int parent_sub_count =
        (parent.ft_data->submotion_array_count != nullptr)
            ? std::max(*parent.ft_data->submotion_array_count, 0)
            : 0;

    MotionImportStats stats{};
    size_t largest_anim = 0;
    if (!ImportMotionTable(info, "mainmotion", mainmotion_pointer,
                           static_cast<int>(mainmotion_count_u32),
                           parent.ft_data->mainmotion, parent_main_count,
                           &runtime.mainmotion, &stats, &largest_anim) ||
        !ImportMotionTable(info, "submotion", submotion_pointer,
                           static_cast<int>(submotion_count_u32),
                           parent.ft_data->submotion, parent_sub_count,
                           &runtime.submotion, &stats, &largest_anim)) {
        port_log("SSB64 Remix: native motion import failed for %s\n", info.display_name);
        return nullptr;
    }

    /* File slots 1-5 are safe to bind immediately. special1..4 deliberately
     * remain parent-owned while their native status handlers are still being
     * ported; their exact EXTRA IDs remain accessible through the asset API. */
    runtime.data.file_main_id = static_cast<u32>(info.main_file_id);
    runtime.data.file_mainmotion_id = static_cast<u32>(info.primary_file_id);
    runtime.data.file_submotion_id = static_cast<u32>(info.secondary_file_id);
    runtime.data.file_model_id = static_cast<u32>(info.character_file_id);
    runtime.data.file_shieldpose_id = static_cast<u32>(info.shield_file_id);

    runtime.data.file_main_size = remix_extra_game_reloc_size(static_cast<uint32_t>(info.main_file_id));
    runtime.data.file_anim_size = (largest_anim != 0) ? largest_anim : parent.ft_data->file_anim_size;

    runtime.data.p_file_main = &runtime.file_main;
    runtime.data.p_file_mainmotion = &runtime.file_mainmotion;
    runtime.data.p_file_submotion = &runtime.file_submotion;
    runtime.data.p_file_model = &runtime.file_model;
    /* The decomp treats p_file_shieldpose as the loaded file pointer itself
     * after ftManagerSetupFilesKind, despite the historical FTData declaration
     * being void**. Leave it null so that setup fills the real status-buffer
     * pointer exactly as vanilla does. */
    runtime.data.p_file_shieldpose = nullptr;
    runtime.data.p_file_special1 = &runtime.file_special1;
    runtime.data.p_file_special2 = &runtime.file_special2;
    runtime.data.p_file_special3 = &runtime.file_special3;
    runtime.data.p_file_special4 = &runtime.file_special4;
    runtime.data.p_particle = &runtime.particle_bank;

    runtime.data.particles_script_lo = particle_script_lo;
    runtime.data.particles_script_hi = particle_script_hi;
    runtime.data.particles_texture_lo = particle_texture_lo;
    runtime.data.particles_texture_hi = particle_texture_hi;
    runtime.data.o_attributes = static_cast<intptr_t>(attributes_offset);

    runtime.data.mainmotion = reinterpret_cast<FTMotionDescArray*>(runtime.mainmotion.data());
    runtime.data.submotion = reinterpret_cast<FTMotionDescArray*>(runtime.submotion.data());
    runtime.data.mainmotion_array_count = static_cast<s32>(runtime.mainmotion.size());
    runtime.submotion_count = static_cast<s32>(runtime.submotion.size());
    runtime.data.submotion_array_count = &runtime.submotion_count;

    port_log("SSB64 Remix: native FTData %-16s main=%zu sub=%zu attr=0x%X anim_heap=%zu offsets[s=%d r=%d p=%d x=%d]\n",
             info.display_name,
             runtime.mainmotion.size(), runtime.submotion.size(), attributes_offset,
             runtime.data.file_anim_size,
             stats.sentinel, stats.relative, stats.parent_absolute,
             stats.remix_absolute);

    return &runtime.data;
}

void SeedBringupRow(const PortRemixExtraFighterInfo& info)
{
    const FighterDescriptor* parent = port_fighter_descriptor(info.parent_fkind);
    if (parent == nullptr) {
        port_log("SSB64 Remix: cannot seed %s (fkind=0x%02X); parent %d is missing\n",
                 info.display_name, info.fkind, info.parent_fkind);
        return;
    }

    FighterDescriptor desc = *parent;
    FTData* native_data = BuildNativeFTData(info, *parent);
    if (native_data == nullptr) {
        port_log("SSB64 Remix: cannot materialize native FTData for %s\n", info.display_name);
        return;
    }
    desc.ft_data = native_data;

    /* Costume frames are present in the custom model, but CSS color mapping
     * is enabled later together with the expanded CSS. */
    desc.costume_count = 0;
    desc.default_costumes = nullptr;
    desc.default_costumes_count = 0;
    for (unsigned char& costume : desc.team_costume) {
        costume = 0xFF;
    }

    desc.results_announce_fgm = 0;
    desc.results_name = info.results_name;
    desc.results_name_lx = info.results_name_lx;
    desc.results_name_scale = info.results_name_scale;
    desc.results_wins_lx = info.results_wins_lx;
    desc.results_emblem_valid = 0;
    desc.results_emblem_dobjdesc = 0;
    desc.results_emblem_mobjsub = 0;
    desc.results_emblem_matanim = 0;

    port_fighter_register(info.fkind, &desc);

    port_log("SSB64 Remix: seeded %-16s fkind=0x%02X parent=%d main=0x%04X model=0x%04X native-motion=1 mask=0x%03X\n",
             info.display_name, info.fkind, info.parent_fkind,
             info.main_file_id, info.character_file_id,
             port_remix_extra_fighter_asset_mask(info.fkind));
}

} // namespace

extern "C" {

void port_remix_seed_fighters(void)
{
    if (!remix_extra_source_exists()) {
        port_log("SSB64 Remix: +EXTRA source absent; synth fighter registry not enabled\n");
        return;
    }
    if (!remix_extra_motion_init()) {
        port_log("SSB64 Remix: +EXTRA motion arena unavailable; synth registry disabled\n");
        return;
    }

    const int valid_assets = port_remix_extra_validate_fighter_assets();
    int seeded = 0;
    for (const PortRemixExtraFighterInfo& info : kExtraFighters) {
        if (!ValidateRowAssets(info)) {
            continue;
        }
        SeedBringupRow(info);
        if (port_fighter_descriptor(info.fkind) != nullptr) {
            ++seeded;
        }
    }

    port_log("SSB64 Remix: +EXTRA 0.5.0 registry ready (%d/%d native rows, %d/%d asset layouts valid, motion_arena=%zu)\n",
             seeded, kExtraFighterCount, valid_assets, kExtraFighterCount,
             remix_extra_motion_arena_size());
}

int port_remix_extra_fighter_count(void)
{
    return kExtraFighterCount;
}

const PortRemixExtraFighterInfo* port_remix_extra_fighter_at(int index)
{
    if (index < 0 || index >= kExtraFighterCount) {
        return nullptr;
    }
    return &kExtraFighters[index];
}

const PortRemixExtraFighterInfo* port_remix_extra_fighter_info(int fkind)
{
    return FindFighter(fkind);
}

int port_remix_extra_is_fighter(int fkind)
{
    return FindFighter(fkind) != nullptr ? 1 : 0;
}

int port_remix_extra_fighter_asset_file_id(int fkind, int asset_slot)
{
    const PortRemixExtraFighterInfo* info = FindFighter(fkind);
    if (info == nullptr) {
        return -1;
    }
    return ResolveAssetFileId(*info, asset_slot);
}

size_t port_remix_extra_fighter_asset_size(int fkind, int asset_slot)
{
    const int file_id = port_remix_extra_fighter_asset_file_id(fkind, asset_slot);
    if (file_id <= 0) {
        return 0;
    }
    return remix_extra_game_reloc_size(static_cast<uint32_t>(file_id));
}

int port_remix_extra_fighter_asset_available(int fkind, int asset_slot)
{
    return port_remix_extra_fighter_asset_size(fkind, asset_slot) > 0 ? 1 : 0;
}

int port_remix_extra_fighter_load_asset(int fkind,
                                        int asset_slot,
                                        void* destination,
                                        uint32_t destination_size,
                                        int file_location,
                                        int force_figatree_fixup)
{
    if (destination == nullptr) {
        return 0;
    }
    const int file_id = port_remix_extra_fighter_asset_file_id(fkind, asset_slot);
    if (file_id <= 0) {
        return 0;
    }
    const size_t required = remix_extra_game_reloc_size(static_cast<uint32_t>(file_id));
    if (required == 0 || required > destination_size) {
        port_log("SSB64 Remix: asset load rejected fkind=0x%02X slot=%d file=0x%04X required=%zu destination=%u\n",
                 fkind, asset_slot, file_id, required, destination_size);
        return 0;
    }
    return remix_extra_game_load_reloc(static_cast<uint32_t>(file_id),
                                       destination,
                                       destination_size,
                                       file_location,
                                       force_figatree_fixup);
}

uint32_t port_remix_extra_fighter_asset_mask(int fkind)
{
    uint32_t mask = 0;
    for (int slot = 0; slot < PORT_REMIX_ASSET_COUNT; ++slot) {
        if (port_remix_extra_fighter_asset_available(fkind, slot)) {
            mask |= (1u << static_cast<uint32_t>(slot));
        }
    }
    return mask;
}

int port_remix_extra_validate_fighter_assets(void)
{
    if (!remix_extra_source_exists()) {
        return 0;
    }
    int valid = 0;
    for (const PortRemixExtraFighterInfo& info : kExtraFighters) {
        if (ValidateRowAssets(info)) {
            ++valid;
        }
    }
    return valid;
}

} // extern "C"
