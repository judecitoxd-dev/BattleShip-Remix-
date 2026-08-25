#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Exact profile for Smash Remix v2.0.1 + EXTRA 0.5.0. */
#define REMIX_EXTRA_RELOC_TABLE_ROM_OFFSET 0x001AC870ULL
#define REMIX_EXTRA_RELOC_DATA_ROM_OFFSET  0x001C2710ULL
#define REMIX_EXTRA_RELOC_FILE_COUNT       7479u
#define REMIX_EXTRA_VANILLA_RELOC_COUNT    2132u

typedef struct RemixExtraRelocInfo {
    uint32_t file_id;
    uint8_t is_compressed;
    uint64_t data_rom_offset;
    uint32_t compressed_size;
    uint32_t decompressed_size;
    uint16_t reloc_intern_offset;
    uint16_t reloc_extern_offset;
    uint32_t extern_count;
} RemixExtraRelocInfo;

/* Read/validate one entry from the expanded Remix relocation table. */
int remix_extra_reloc_get_info(uint32_t file_id, RemixExtraRelocInfo* out_info);

/* Number of reloc files added beyond vanilla's 0..2131 table. */
uint32_t remix_extra_reloc_new_file_count(void);

/* Probable figatree/animation classifier for the shared port fixup path. */
int remix_extra_reloc_is_figatree(uint32_t file_id);

/* Read the u16 external dependency IDs stored after the compressed payload. */
size_t remix_extra_reloc_get_extern_ids(uint32_t file_id,
                                        uint16_t* dst, size_t dst_count);

/*
 * Extract one reloc file into caller-owned memory in original N64 byte order.
 * Compressed entries are VPK0-decoded. Returns bytes written, 0 on failure.
 */
size_t remix_extra_reloc_extract(uint32_t file_id, void* dst, size_t dst_capacity);

#ifdef __cplusplus
}
#endif
