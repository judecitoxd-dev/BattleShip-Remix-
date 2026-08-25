#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Smash Remix 2.0.1 + EXTRA 0.5.0 fighter IDs.
 *
 * Smash Remix 2.0.1 occupies IDs through 0x60. +EXTRA 0.5.0 sorts
 * extra_characters/ alphabetically before invoking define_character(), so
 * these additions begin at 0x61 in the stable order below.
 */
typedef enum PortRemixExtraFighterKind {
    PORT_REMIX_FKIND_BIRDO        = 0x61,
    PORT_REMIX_FKIND_CB_KNUCKLES  = 0x62,
    PORT_REMIX_FKIND_CBM_KNUCKLES = 0x63,
    PORT_REMIX_FKIND_CLOUD        = 0x64,
    PORT_REMIX_FKIND_DK_ULT       = 0x65,
    PORT_REMIX_FKIND_KAZUYA       = 0x66,
    PORT_REMIX_FKIND_KEN          = 0x67,
    PORT_REMIX_FKIND_KNUCKLES     = 0x68,
    PORT_REMIX_FKIND_M_KNUCKLES   = 0x69,
    PORT_REMIX_FKIND_MR_GAW       = 0x6A,
    PORT_REMIX_FKIND_MR_GAW_PLUS  = 0x6B,
    PORT_REMIX_FKIND_MR_GAW_3D    = 0x6C,
    PORT_REMIX_FKIND_META_KNIGHT  = 0x6D,
    PORT_REMIX_FKIND_REBECCA      = 0x6E,
    PORT_REMIX_FKIND_RYU          = 0x6F,
    PORT_REMIX_FKIND_SNAKE        = 0x70,
    PORT_REMIX_FKIND_SPIDERMAN    = 0x71,
    PORT_REMIX_FKIND_TERRY        = 0x72,
    PORT_REMIX_FKIND_YOUNG_ZELDA  = 0x73,
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

/* Version-pinned asset IDs already verified against the target 0.5.0 ROM. */
enum {
    PORT_REMIX_FILE_META_KNIGHT_MAIN      = 6730,
    PORT_REMIX_FILE_META_KNIGHT_CHARACTER = 6731,
};

/* Source-of-truth row for the +EXTRA 0.5.0 roster. The declared costume
 * count is intentionally separate from FighterDescriptor::costume_count:
 * until a fighter's real model is wired, the descriptor stays in safe
 * parent-clone mode and must not request costume frames the parent lacks. */
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

    /* 0 until the exact 0.5.0 RELOC IDs have been verified. */
    int main_file_id;
    int character_file_id;
} PortRemixExtraFighterInfo;

/* Register built-in Remix/+EXTRA fighter descriptors after the vanilla
 * registry has been seeded. Unlike TCC MOD_INIT this path is compiled on
 * Android too. */
void port_remix_seed_fighters(void);

/* Table accessors used by CSS, asset binding and diagnostics. */
int port_remix_extra_fighter_count(void);
const PortRemixExtraFighterInfo* port_remix_extra_fighter_at(int index);
const PortRemixExtraFighterInfo* port_remix_extra_fighter_info(int fkind);
int port_remix_extra_is_fighter(int fkind);

#ifdef __cplusplus
}
#endif
