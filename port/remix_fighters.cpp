#include "remix_fighters.h"
#include "fighter_registry.h"
#include "port_log.h"
#include "remix_extra_game_bridge.h"
#include "remix_extra_reloc.h"
#include "remix_extra_source.h"

#include <cstddef>
#include <cstdint>

namespace {

/* Exact Smash Remix + EXTRA 0.5.0 roster metadata. Parent/costume values come
 * from the 0.5.0 sources. File IDs were recovered from the final target ROM's
 * generated Character structs, so they describe the exact 0.5.0 ROM target. */
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

constexpr int kExtraFighterCount =
    static_cast<int>(sizeof(kExtraFighters) / sizeof(kExtraFighters[0]));

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
    if (reloc.file_id != static_cast<uint32_t>(file_id) ||
        reloc.decompressed_size == 0) {
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

void SeedBringupRow(const PortRemixExtraFighterInfo& info)
{
    const FighterDescriptor* parent = port_fighter_descriptor(info.parent_fkind);
    if (parent == nullptr) {
        port_log("SSB64 Remix: cannot seed %s (fkind=0x%02X); parent %d is missing\n",
                 info.display_name, info.fkind, info.parent_fkind);
        return;
    }

    FighterDescriptor desc = *parent;

    /* The synth has its exact FTKind and asset binding immediately. Native
     * FTData/status functions remain inherited until their N64 32-bit data is
     * reconstructed into BattleShip's native pointer layout. */
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

    port_log("SSB64 Remix: seeded %-16s fkind=0x%02X parent=%d main=0x%04X model=0x%04X costumes=%d mask=0x%03X\n",
             info.display_name, info.fkind, info.parent_fkind,
             info.main_file_id, info.character_file_id,
             info.declared_costume_count,
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

    port_log("SSB64 Remix: +EXTRA 0.5.0 registry ready (%d/%d rows, %d/%d asset layouts valid)\n",
             seeded, kExtraFighterCount, valid_assets, kExtraFighterCount);
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
    const int file_id = port_remix_extra_fighter_asset_file_id(fkind, asset_slot);
    if (file_id <= 0) {
        return 0;
    }
    return remix_extra_game_reloc_size(static_cast<uint32_t>(file_id)) > 0 ? 1 : 0;
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
