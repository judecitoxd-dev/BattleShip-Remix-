#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Smash Remix 2.0.1 + EXTRA 0.5.0 fighter IDs.
 *
 * IMPORTANT: Character.ADD_CHARACTERS is reserved table capacity, not the
 * number of Character.define_character calls that already exist. +EXTRA
 * inserts its alphabetically-sorted definitions immediately after the last
 * actually-created Smash Remix fighter. In the verified final 0.5.0 ROM the
 * 19 EXTRA rows occupy Character.STRUCT_TABLE indices 0x4D..0x5F.
 *
 * These IDs are verified against the final ROM's table at ROM 0x00092610;
 * do not derive them from NUM_CHARACTERS/ADD_CHARACTERS.
 */
typedef enum PortRemixExtraFighterKind {
    PORT_REMIX_FKIND_BIRDO        = 0x4D,
    PORT_REMIX_FKIND_CB_KNUCKLES  = 0x4E,
    PORT_REMIX_FKIND_CBM_KNUCKLES = 0x4F,
    PORT_REMIX_FKIND_CLOUD        = 0x50,
    PORT_REMIX_FKIND_DK_ULT       = 0x51,
    PORT_REMIX_FKIND_KAZUYA       = 0x52,
    PORT_REMIX_FKIND_KEN          = 0x53,
    PORT_REMIX_FKIND_KNUCKLES     = 0x54,
    PORT_REMIX_FKIND_M_KNUCKLES   = 0x55,
    PORT_REMIX_FKIND_MR_GAW       = 0x56,
    PORT_REMIX_FKIND_MR_GAW_PLUS  = 0x57,
    PORT_REMIX_FKIND_MR_GAW_3D    = 0x58,
    PORT_REMIX_FKIND_META_KNIGHT  = 0x59,
    PORT_REMIX_FKIND_REBECCA      = 0x5A,
    PORT_REMIX_FKIND_RYU          = 0x5B,
    PORT_REMIX_FKIND_SNAKE        = 0x5C,
    PORT_REMIX_FKIND_SPIDERMAN    = 0x5D,
    PORT_REMIX_FKIND_TERRY        = 0x5E,
    PORT_REMIX_FKIND_YOUNG_ZELDA  = 0x5F,
} PortRemixExtraFighterKind;

/* Vanilla FTKind values used as the temporary native parent while a synth's
 * own FTData/status functions are being ported. These match the decomp enum. */
typedef enum PortVanillaFighterKind {
    PORT_VANILLA_FKIND_MARIO       = 0,
    PORT_VANILLA_FKIND_FOX         = 1,
    PORT_VANILLA_FKIND_DONKEY      = 2,
    PORT_VANILLA_FKIND_SAMUS       = 3,
    PORT_VANILLA_FKIND_LUIGI       = 4,
    PORT_VANILLA_FKIND_LINK        = 5,
    PORT_VANILLA_FKIND_YOSHI       = 6,
    PORT_VANILLA_FKIND_CAPTAIN     = 7,
    PORT_VANILLA_FKIND_KIRBY       = 8,
    PORT_VANILLA_FKIND_PIKACHU     = 9,
    PORT_VANILLA_FKIND_JIGGLYPUFF  = 10,
    PORT_VANILLA_FKIND_NESS        = 11,
} PortVanillaFighterKind;

/* Stable native slots for the nine file IDs emitted by Character.define_character. */
typedef enum PortRemixExtraFighterAssetSlot {
    PORT_REMIX_ASSET_MAIN = 0,
    PORT_REMIX_ASSET_PRIMARY,
    PORT_REMIX_ASSET_SECONDARY,
    PORT_REMIX_ASSET_CHARACTER,
    PORT_REMIX_ASSET_SHIELD,
    PORT_REMIX_ASSET_MISC0,
    PORT_REMIX_ASSET_MISC1,
    PORT_REMIX_ASSET_MISC2,
    PORT_REMIX_ASSET_MISC3,
    PORT_REMIX_ASSET_COUNT,
} PortRemixExtraFighterAssetSlot;

/* Source-of-truth row for the +EXTRA 0.5.0 roster. The nine file IDs mirror
 * Character.define_character's N64 file layout, but are stored in native
 * metadata instead of reusing the original 32-bit-pointer FTData blob. */
typedef struct PortRemixExtraFighterInfo {
    int fkind;
    int parent_fkind;
    const char* source_name;
    const char* display_name;
    int declared_costume_count;

    const char* results_name;
    float results_name_lx;
    float results_name_scale;
    float results_wins_lx;

    int main_file_id;
    int primary_file_id;
    int secondary_file_id;
    int character_file_id;
    int shield_file_id;
    int misc_file_id[4];
} PortRemixExtraFighterInfo;

/* Register built-in Remix/+EXTRA fighter descriptors after the vanilla
 * registry has been seeded. Unlike TCC MOD_INIT this path is compiled on
 * Android too. No EXTRA rows are exposed when the reference ROM is absent. */
void port_remix_seed_fighters(void);

/* Table accessors used by CSS, asset binding and diagnostics. */
int port_remix_extra_fighter_count(void);
const PortRemixExtraFighterInfo* port_remix_extra_fighter_at(int index);
const PortRemixExtraFighterInfo* port_remix_extra_fighter_info(int fkind);
int port_remix_extra_is_fighter(int fkind);

/* Resolve one Character.define_character file slot for a synth fighter.
 * Returns 0 when the slot is intentionally absent and -1 for invalid input. */
int port_remix_extra_fighter_asset_file_id(int fkind, int asset_slot);

/* Query/validate/load a synth fighter file through the normal BattleShip
 * relocation pipeline. These calls never expose raw ROM pointers. */
size_t port_remix_extra_fighter_asset_size(int fkind, int asset_slot);
int port_remix_extra_fighter_asset_available(int fkind, int asset_slot);
int port_remix_extra_fighter_load_asset(int fkind,
                                        int asset_slot,
                                        void* destination,
                                        uint32_t destination_size,
                                        int file_location,
                                        int force_figatree_fixup);

/* Bit i is set when asset slot i exists in the target ROM and has a non-zero
 * decompressed size. Useful for bulk bring-up and diagnostics. */
uint32_t port_remix_extra_fighter_asset_mask(int fkind);

/* Validate every non-zero file ID used by the 19 target fighters against the
 * expanded 0.5.0 RELOC table. Returns the number of completely valid rows. */
int port_remix_extra_validate_fighter_assets(void);

#ifdef __cplusplus
}
#endif
