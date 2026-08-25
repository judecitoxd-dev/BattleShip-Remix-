#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return the decompressed byte count for a Remix + EXTRA reloc file. */
size_t remix_extra_game_reloc_size(uint32_t file_id);

/*
 * Extract a Remix + EXTRA reloc file and feed it through BattleShip's normal
 * relocation/byteswap pipeline.
 *
 * ram_dst / bytes_num follow portRelocLoadFileFromBytes semantics. `loc` is
 * one of the game's nLBFileLocation* values. Returns 1 on success, 0 on error.
 */
int remix_extra_game_load_reloc(uint32_t file_id,
                                void* ram_dst,
                                uint32_t bytes_num,
                                int loc,
                                int force_figatree_fixup);

#ifdef __cplusplus
}
#endif
