#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Smash Remix 2.0.1 + EXTRA 0.5.0 fighter IDs.
 *
 * Smash Remix 2.0.1 currently occupies IDs through 0x60. +EXTRA's
 * character_appender.py sorts extra_characters/ alphabetically before
 * invoking Character.define_character(), so the 0.5.0 additions begin at
 * 0x61 in the order below.
 *
 * Keep these IDs explicit: they are part of the content ABI used by the
 * Remix port layer and must not depend on C++ container ordering.
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

/* Register built-in Remix/+EXTRA fighter descriptors after the vanilla
 * registry has been seeded. Unlike TCC MOD_INIT this path is compiled on
 * Android too. */
void port_remix_seed_fighters(void);

#ifdef __cplusplus
}
#endif
