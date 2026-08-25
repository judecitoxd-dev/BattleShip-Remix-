#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Minimal, allocation-free-at-the-API VPK0 decoder used by the Remix + EXTRA
 * ROM reader. The implementation mirrors SSB64's VPK0 format but is kept
 * independent from Torch so Android can read newly-added Remix reloc files at
 * runtime even though Torch is only used for the vanilla BattleShip.o2r pass.
 *
 * Returns the number of bytes written, or 0 on malformed input/capacity error.
 */
size_t remix_extra_vpk0_decode(const void *src, size_t src_size,
                               void *dst, size_t dst_capacity);

/* Returns the big-endian decoded size stored in a VPK0 header, or 0 if invalid. */
size_t remix_extra_vpk0_decoded_size(const void *src, size_t src_size);

#ifdef __cplusplus
}
#endif
