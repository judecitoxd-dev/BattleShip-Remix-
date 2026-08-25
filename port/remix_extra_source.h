#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Read-only bridge to the user-installed Smash Remix + EXTRA reference ROM.
 *
 * The file is installed by the Android importer before SDL starts. Nothing
 * here interprets or executes N64 MIPS code; native-port modules use this
 * bridge only to fetch known data ranges while they are converted into the
 * normal BattleShip resource/runtime structures.
 */
const char *remix_extra_source_get_path(void);
int remix_extra_source_exists(void);
size_t remix_extra_source_read(uint64_t offset, void *dst, size_t size);
int remix_extra_source_read_be32(uint64_t offset, uint32_t *value_out);

#ifdef __cplusplus
}
#endif
