#pragma once

/**
 * RelocPointerTable — 32-bit token ↔ 64-bit pointer mapping.
 *
 * On N64, relocated pointer slots in file data are 4 bytes (sizeof(void*) == 4).
 * On 64-bit PC, void* is 8 bytes and won't fit in the 4-byte slots.
 *
 * The token system solves this:
 *   - During relocation, the bridge computes the real 64-bit pointer and
 *     registers it, getting back a 32-bit token.
 *   - The token is written into the 4-byte data slot (fits perfectly).
 *   - Game code resolves tokens back to pointers via RELOC_RESOLVE().
 *
 * Token 0 is reserved for NULL.
 * Tokens contain a generation plus an index into a flat array — resolution is
 * O(1), and stale tokens from earlier scene/setup generations are rejected.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Register a 64-bit pointer and get back a 32-bit token. NULL -> 0. */
uint32_t portRelocRegisterPointer(void *ptr);

/** Resolve a 32-bit token back to a 64-bit pointer. Token 0 -> NULL. */
void *portRelocResolvePointer(uint32_t token);
void *portRelocResolvePointerDebug(uint32_t token, const char *file, int line);

/**
 * Try to resolve without logging when the value is not registered.
 * Useful when a 32-bit field can be a token or another address encoding.
 */
void *portRelocTryResolvePointer(uint32_t token);

/**
 * Register a persistent raw 32-bit address range as an alias of native host
 * memory. This is for ROM-authored N64 virtual addresses which survive in
 * generated data (Smash Remix action scripts are the first consumer).
 *
 * A value raw_base + N passed to portRelocResolvePointer/PORT_RESOLVE resolves
 * to host_base + N. Raw aliases are checked before generational tokens so an
 * N64 address can never be mistaken for a coincidentally-shaped token.
 *
 * The caller owns host_base and must keep it alive for the lifetime of the
 * registration. Registrations are expected during boot/content init and are
 * read-only afterwards. Overlapping ranges are rejected. Returns 1 on success.
 */
int portRelocRegisterRawAddressRange(uint32_t raw_base,
                                     void *host_base,
                                     size_t size);

/** Hard wipe of the generational token table. Persistent raw aliases survive. */
void portRelocResetPointerTable(void);

/**
 * Selectively invalidate token slots whose stored host pointer falls in
 * [base, base+size). Persistent raw aliases are not scene-arena allocations
 * and therefore are not affected by this operation.
 */
void portRelocInvalidateRange(const void *base, size_t size);

#ifdef __cplusplus
}
#endif

#define RELOC_RESOLVE(token) portRelocResolvePointer((uint32_t)(token))
