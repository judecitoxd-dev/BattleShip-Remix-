#!/usr/bin/env python3
"""Validate assumptions used by the native +EXTRA fighter motion importer.

Metadata-only diagnostic: it never writes or exports copyrighted ROM payloads.
Pinned to Smash Remix 2.0.1 + EXTRA 0.5.0.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

EXPECTED_SHA1 = "b9aa8800f9676e6edc454ec3cf9790ab2c229cac"
EXPECTED_SIZE = 80_312_584
PATCH_RAM_ROM_DELTA = 0x7CC00000
MOTION_RAW_BASE = 0x805C0000
MOTION_RAW_END = 0x80610000
RELOC_TABLE = 0x001AC870
RELOC_COUNT = 7479

FIGHTERS = [
    ("Birdo",       0x038E2120), ("CBKnuckles",  0x038E2DE0),
    ("CBMKnuckles", 0x038E3D50), ("Cloud",       0x038E4CC0),
    ("DKUlt",       0x038E5A70), ("Kazuya",      0x038E68B0),
    ("Ken",         0x038E78E0), ("Knuckles",    0x038E88F0),
    ("MKnuckles",   0x038E9860), ("MRGAW",       0x038EA7D0),
    ("MRGAWPLUS",   0x038EB5C0), ("MRGAWTHREED", 0x038EC3D0),
    ("MetaKnight",  0x038ED1E0), ("Rebecca",     0x038EE050),
    ("Ryu",         0x038EEE10), ("Snake",       0x038EFDC0),
    ("Spiderman",   0x038F10F0), ("Terry",       0x038F1EE0),
    ("YZelda",      0x038F2EB0),
]


def be32(data: bytes, off: int) -> int:
    return struct.unpack_from(">I", data, off)[0]


def ptr_to_rom(ptr: int) -> int:
    if ptr < PATCH_RAM_ROM_DELTA:
        raise ValueError(f"not a patch pointer: 0x{ptr:08X}")
    return ptr - PATCH_RAM_ROM_DELTA


def reloc_size(rom: bytes, file_id: int) -> int:
    if not 0 <= file_id < RELOC_COUNT:
        return 0
    entry = RELOC_TABLE + file_id * 12
    decompressed_words = struct.unpack_from(">H", rom, entry + 10)[0]
    return decompressed_words * 4


def classify_offset(value: int) -> str:
    if value == 0x80000000:
        return "sentinel"
    if value <= 0x00100000:
        return "relative"
    if 0x80000000 < value < 0x80400000:
        return "parent_absolute"
    if MOTION_RAW_BASE <= value < MOTION_RAW_END:
        return "remix_absolute"
    return "unsupported"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    args = ap.parse_args()

    rom = args.rom.read_bytes()
    got_sha1 = hashlib.sha1(rom).hexdigest()
    if len(rom) != EXPECTED_SIZE or got_sha1 != EXPECTED_SHA1:
        raise SystemExit(
            f"Unsupported ROM: size={len(rom)} sha1={got_sha1}; "
            f"expected {EXPECTED_SIZE} / {EXPECTED_SHA1}"
        )

    totals = {
        "sentinel": 0,
        "relative": 0,
        "parent_absolute": 0,
        "remix_absolute": 0,
        "unsupported": 0,
    }
    fighters: list[dict] = []

    for index, (name, struct_off) in enumerate(FIGHTERS):
        ids = struct.unpack_from(">9I", rom, struct_off)
        main_ptr = be32(rom, struct_off + 0x64)
        sub_ptr = be32(rom, struct_off + 0x68)
        main_count = be32(rom, struct_off + 0x6C)
        sub_count_ptr = be32(rom, struct_off + 0x70)
        sub_count = be32(rom, ptr_to_rom(sub_count_ptr))

        if main_count > 4096 or sub_count > 4096:
            raise SystemExit(f"{name}: unreasonable motion count")

        row_counts = {key: 0 for key in totals}
        largest_anim = 0

        for table_name, ptr, count in (
            ("mainmotion", main_ptr, main_count),
            ("submotion", sub_ptr, sub_count),
        ):
            base = ptr_to_rom(ptr)
            end = base + count * 12
            if base < 0 or end > len(rom):
                raise SystemExit(f"{name}: {table_name} outside ROM")

            for motion_index in range(count):
                off = base + motion_index * 12
                anim_file_id, motion_offset, anim_desc = struct.unpack_from(">III", rom, off)
                category = classify_offset(motion_offset)
                row_counts[category] += 1
                totals[category] += 1

                if category == "unsupported":
                    raise SystemExit(
                        f"{name}: {table_name}[{motion_index}] unsupported "
                        f"offset 0x{motion_offset:08X}"
                    )

                # FTANIM_FLAG_SHIELDPOSE is logical bit 1. For shieldpose rows
                # anim_file_id is not an ordinary RELOC animation file id.
                if anim_file_id and not (anim_desc & 0x2):
                    size = reloc_size(rom, anim_file_id)
                    if size == 0:
                        raise SystemExit(
                            f"{name}: {table_name}[{motion_index}] invalid "
                            f"animation RELOC 0x{anim_file_id:X}"
                        )
                    largest_anim = max(largest_anim, size)

        fighters.append(
            {
                "name": name,
                "fkind": 0x61 + index,
                "character_struct_rom_offset": struct_off,
                "file_ids": list(ids),
                "attributes_offset": be32(rom, struct_off + 0x60),
                "mainmotion_count": main_count,
                "submotion_count": sub_count,
                "largest_animation_bytes": largest_anim,
                "offset_classes": row_counts,
            }
        )

    expected_totals = {
        "sentinel": 700,
        "relative": 2415,
        "parent_absolute": 23,
        "remix_absolute": 1512,
        "unsupported": 0,
    }
    if totals != expected_totals:
        raise SystemExit(f"Profile totals changed: {totals} != {expected_totals}")

    meta = fighters[12]
    if (meta["name"], meta["fkind"], meta["mainmotion_count"], meta["submotion_count"]) != (
        "MetaKnight", 0x6D, 225, 15
    ):
        raise SystemExit("Meta Knight profile anchor mismatch")

    print(json.dumps({
        "profile": "Smash Remix 2.0.1 + EXTRA 0.5.0",
        "sha1": got_sha1,
        "motion_raw_range": [MOTION_RAW_BASE, MOTION_RAW_END],
        "fighter_count": len(fighters),
        "motion_rows": sum(totals.values()),
        "offset_classes": totals,
        "fighters": fighters,
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
