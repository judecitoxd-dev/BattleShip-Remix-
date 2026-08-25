#!/usr/bin/env python3
"""Validate the native motion importer across the full generated roster.

Metadata-only diagnostic pinned to Smash Remix 2.0.1 + EXTRA 0.5.0. It walks
Character.STRUCT_TABLE rows 0x1D..0x73 (48 Remix + 19 EXTRA + 20 polygons),
checks the generated 0x78-byte FTData layout, validates animation RELOC ids and
classifies every motion pointer against the native importer rules.
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
STRUCT_TABLE_ROM = 0x00092610
FKIND_FIRST = 0x1D
FKIND_LAST = 0x73
MOTION_RAW_BASE = 0x80570000
MOTION_RAW_END = 0x80610000
RELOC_TABLE = 0x001AC870
RELOC_COUNT = 7479

BASE_NAMES = [
    "FALCO", "GND", "YLINK", "DRM", "WARIO", "DSAMUS", "ELINK", "JSAMUS",
    "JNESS", "LUCAS", "JLINK", "JFALCON", "JFOX", "JMARIO", "JLUIGI", "JDK",
    "EPIKA", "JPUFF", "EPUFF", "JKIRBY", "JYOSHI", "JPIKA", "ESAMUS", "BOWSER",
    "GBOWSER", "PIANO", "WOLF", "CONKER", "MTWO", "MARTH", "SONIC", "SANDBAG",
    "SSONIC", "SHEIK", "MARINA", "DEDEDE", "GOEMON", "PEPPY", "SLIPPY", "BANJO",
    "MLUIGI", "EBI", "DRAGONKING", "CRASH", "PEACH", "ROY", "DRL", "LANKY",
]
EXTRA_NAMES = [
    "Birdo", "CBKnuckles", "CBMKnuckles", "Cloud", "DKUlt", "Kazuya", "Ken",
    "Knuckles", "MKnuckles", "MRGAW", "MRGAWPLUS", "MRGAWTHREED", "MetaKnight",
    "Rebecca", "Ryu", "Snake", "Spiderman", "Terry", "YZelda",
]
POLYGON_NAMES = [
    "NWARIO", "NLUCAS", "NBOWSER", "NWOLF", "NDRM", "NSONIC", "NSHEIK",
    "NMARINA", "NFALCO", "NGND", "NDSAMUS", "NMARTH", "NMTWO", "NDEDEDE",
    "NYLINK", "NGOEMON", "NCONKER", "NBANJO", "NPEACH", "NCRASH",
]
NAMES = BASE_NAMES + EXTRA_NAMES + POLYGON_NAMES
assert len(NAMES) == FKIND_LAST - FKIND_FIRST + 1 == 87


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


def group_for_fkind(fkind: int) -> str:
    if fkind <= 0x4C:
        return "remix"
    if fkind <= 0x5F:
        return "extra"
    return "polygon"


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

    for index, name in enumerate(NAMES):
        fkind = FKIND_FIRST + index
        struct_ptr = be32(rom, STRUCT_TABLE_ROM + fkind * 4)
        struct_off = ptr_to_rom(struct_ptr)
        if struct_off < 0 or struct_off + 0x78 > len(rom):
            raise SystemExit(f"{name}: Character struct outside ROM")

        ids = struct.unpack_from(">9I", rom, struct_off)
        if any(fid >= RELOC_COUNT for fid in ids if fid):
            raise SystemExit(f"{name}: invalid generated file id")

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

                # FTANIM_FLAG_SHIELDPOSE is logical bit 1. Shieldpose rows do
                # not use anim_file_id as an ordinary animation RELOC id.
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
                "fkind": fkind,
                "group": group_for_fkind(fkind),
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
        "sentinel": 3550,
        "relative": 11651,
        "parent_absolute": 173,
        "remix_absolute": 5296,
        "unsupported": 0,
    }
    if totals != expected_totals:
        raise SystemExit(f"Profile totals changed: {totals} != {expected_totals}")

    meta = fighters[0x59 - FKIND_FIRST]
    if (meta["name"], meta["fkind"], meta["mainmotion_count"], meta["submotion_count"]) != (
        "MetaKnight", 0x59, 225, 15
    ):
        raise SystemExit("Meta Knight profile anchor mismatch")

    print(json.dumps({
        "profile": "Smash Remix 2.0.1 + EXTRA 0.5.0",
        "sha1": got_sha1,
        "fkind_range": [FKIND_FIRST, FKIND_LAST],
        "motion_raw_range": [MOTION_RAW_BASE, MOTION_RAW_END],
        "fighter_count": len(fighters),
        "group_counts": {"remix": 48, "extra": 19, "polygon": 20},
        "motion_rows": sum(totals.values()),
        "offset_classes": totals,
        "fighters": fighters,
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
