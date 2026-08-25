#!/usr/bin/env python3
"""Recover Smash Remix + EXTRA 0.5.0 fighter file layouts from a user ROM.

The final ROM contains one generated N64 Character/FTData-like struct for each
EXTRA fighter. This tool identifies those structs by their stable layout and
then verifies each one against Character.STRUCT_TABLE, so reserved
ADD_CHARACTERS capacity can never be mistaken for actually-created FTKind rows.
It emits metadata only and never writes asset payload bytes.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SHA1 = "b9aa8800f9676e6edc454ec3cf9790ab2c229cac"
EXPECTED_SIZE = 80_312_584
RELOC_FILE_COUNT = 7_479

# Character.asm constants / target-profile mapping.
CHARACTER_STRUCT_TABLE_ROM = 0x0009_2610
PATCH_RAM_ROM_DELTA = 0x7CC0_0000
EXTRA_FKIND_FIRST = 0x4D
EXTRA_FKIND_LAST = 0x5F

# EXTRA source/patch code for this exact build lives in this ROM window. Using
# the window avoids an unnecessary 80 MiB Python word-by-word scan while the
# structural checks below still uniquely select the 19 character structs.
SEARCH_BEGIN = 0x0380_0000
SEARCH_END = 0x0400_0000

EXTRA_FIGHTERS = [
    "Birdo", "CBKnuckles", "CBMKnuckles", "Cloud", "DKUlt", "Kazuya",
    "Ken", "Knuckles", "MKnuckles", "MRGAW", "MRGAWPLUS", "MRGAWTHREED",
    "MetaKnight", "Rebecca", "Ryu", "Snake", "Spiderman", "Terry", "YZelda",
]

assert EXTRA_FKIND_LAST - EXTRA_FKIND_FIRST + 1 == len(EXTRA_FIGHTERS)


def sha1(data: bytes) -> str:
    return hashlib.sha1(data).hexdigest()


def be32(data: bytes, off: int) -> int:
    return struct.unpack_from(">I", data, off)[0]


def is_runtime_ptr(value: int) -> bool:
    return 0x8000_0000 <= value < 0x8100_0000


def struct_runtime_address(rom_offset: int) -> int:
    return rom_offset + PATCH_RAM_ROM_DELTA


def verify_struct_table_entry(rom: bytes, fkind: int, rom_offset: int) -> None:
    table_off = CHARACTER_STRUCT_TABLE_ROM + fkind * 4
    if table_off + 4 > len(rom):
        raise SystemExit(f"FTKind 0x{fkind:02X}: STRUCT_TABLE read outside ROM")
    table_ptr = be32(rom, table_off)
    expected = struct_runtime_address(rom_offset)
    if table_ptr != expected:
        raise SystemExit(
            f"FTKind 0x{fkind:02X}: STRUCT_TABLE mismatch: "
            f"got 0x{table_ptr:08X}, expected 0x{expected:08X} "
            f"for ROM struct 0x{rom_offset:08X}"
        )


def find_structs(rom: bytes) -> list[dict]:
    out: list[dict] = []
    end = min(len(rom), SEARCH_END)

    for off in range(SEARCH_BEGIN, end - 0x50, 4):
        main, primary, secondary, character, shield = struct.unpack_from(
            ">IIIII", rom, off
        )

        if not (0x154F <= main < RELOC_FILE_COUNT):
            continue
        if character != main + 1 or secondary != 0:
            continue
        if primary >= RELOC_FILE_COUNT or shield >= RELOC_FILE_COUNT:
            continue

        # In the generated 32-bit N64 struct, words 10/11/13 are writable
        # pointer-slot addresses and word 12 is NULL for the unused submotion
        # slot. This eliminates action arrays and coincidental file-ID runs.
        p_main, p_primary, p_secondary, p_character = struct.unpack_from(
            ">IIII", rom, off + 0x28
        )
        if not (is_runtime_ptr(p_main) and is_runtime_ptr(p_primary)):
            continue
        if p_secondary != 0 or not is_runtime_ptr(p_character):
            continue

        ids = struct.unpack_from(">9I", rom, off)
        if any(fid >= RELOC_FILE_COUNT for fid in ids if fid != 0):
            continue

        out.append(
            {
                "rom_offset": off,
                "main_file_id": ids[0],
                "primary_file_id": ids[1],
                "secondary_file_id": ids[2],
                "character_file_id": ids[3],
                "shield_file_id": ids[4],
                "misc_file_ids": list(ids[5:9]),
            }
        )

    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    args = ap.parse_args()

    rom = args.rom.read_bytes()
    got_sha1 = sha1(rom)
    if len(rom) != EXPECTED_SIZE or got_sha1 != EXPECTED_SHA1:
        raise SystemExit(
            "Unsupported ROM: expected Smash Remix 2.0.1 + EXTRA 0.5.0 "
            f"({EXPECTED_SHA1}), got size={len(rom)} sha1={got_sha1}"
        )

    structs = find_structs(rom)
    if len(structs) != len(EXTRA_FIGHTERS):
        raise SystemExit(
            f"Expected {len(EXTRA_FIGHTERS)} EXTRA fighter structs, found {len(structs)}"
        )

    for index, (name, row) in enumerate(zip(EXTRA_FIGHTERS, structs)):
        fkind = EXTRA_FKIND_FIRST + index
        verify_struct_table_entry(rom, fkind, row["rom_offset"])
        row["name"] = name
        row["fkind"] = fkind
        row["struct_table_rom_offset"] = CHARACTER_STRUCT_TABLE_ROM + fkind * 4
        row["struct_runtime_address"] = struct_runtime_address(row["rom_offset"])

    # Strong anchor from the independently recovered Meta Knight source files.
    meta = structs[EXTRA_FIGHTERS.index("MetaKnight")]
    if meta["fkind"] != 0x59:
        raise SystemExit("Meta Knight FTKind anchor mismatch")
    if (meta["main_file_id"], meta["character_file_id"]) != (6730, 6731):
        raise SystemExit("Meta Knight file anchor mismatch; refusing ambiguous catalog")

    print(
        json.dumps(
            {
                "profile": "Smash Remix 2.0.1 + EXTRA 0.5.0",
                "sha1": got_sha1,
                "fighter_count": len(structs),
                "fkind_first": EXTRA_FKIND_FIRST,
                "fkind_last": EXTRA_FKIND_LAST,
                "character_struct_table_rom": CHARACTER_STRUCT_TABLE_ROM,
                "fighters": structs,
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
