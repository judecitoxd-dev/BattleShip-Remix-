#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Smash Remix + EXTRA 0.5.0 stores many action-script pointers as absolute
 * N64 virtual addresses in the generated fighter motion tables. Android/PC
 * cannot dereference those addresses directly, so this module materializes
 * the known script window into native memory and exposes it through the
 * RelocPointerTable raw-address alias bridge.
 *
 * These constants are pinned to the verified target ROM profile.
 */
#define REMIX_EXTRA_PATCH_RAM_ROM_DELTA 0x7CC00000u
#define REMIX_EXTRA_MOTION_RAW_BASE     0x805C0000u
#define REMIX_EXTRA_MOTION_RAW_END      0x80610000u
#define REMIX_EXTRA_MOTION_ROM_BASE     0x039C0000u
#define REMIX_EXTRA_MOTION_ARENA_SIZE   (REMIX_EXTRA_MOTION_RAW_END - REMIX_EXTRA_MOTION_RAW_BASE)

/* Initialize the persistent BE->native motion-data arena and register its
 * N64 address range. Safe to call repeatedly. Returns 1 on success. */
int remix_extra_motion_init(void);

/* Resolve an absolute N64 address inside the registered motion arena.
 * Returns NULL for addresses outside the profile window or on init failure. */
void *remix_extra_motion_resolve(uint32_t n64_address);

/* True when an N64 address belongs to the materialized motion window. */
int remix_extra_motion_contains(uint32_t n64_address);

/* Size of the persistent arena, useful for diagnostics/tests. */
size_t remix_extra_motion_arena_size(void);

#ifdef __cplusplus
}
#endif
