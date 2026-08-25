#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Smash Remix 2.0.1 + EXTRA 0.5.0 stores generated action-script pointers as
 * absolute N64 virtual addresses. Android/PC cannot dereference those
 * addresses directly, so this module materializes the verified script window
 * into native memory and exposes it through the RelocPointerTable raw-address
 * alias bridge.
 *
 * The original EXTRA-only bring-up used 0x805C0000..0x80610000. Direct
 * validation of every generated Character row 0x1D..0x73 showed that Smash
 * Remix base additionally uses script pointers down to 0x805762FC. The
 * profile window is therefore rounded down to 0x80570000 so the same native
 * importer can cover Remix base, EXTRA and the Remix polygon rows.
 */
#define REMIX_EXTRA_PATCH_RAM_ROM_DELTA 0x7CC00000u
#define REMIX_EXTRA_MOTION_RAW_BASE     0x80570000u
#define REMIX_EXTRA_MOTION_RAW_END      0x80610000u
#define REMIX_EXTRA_MOTION_ROM_BASE     0x03970000u
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
