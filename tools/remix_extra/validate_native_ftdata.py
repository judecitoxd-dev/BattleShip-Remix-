#!/usr/bin/env python3
"""Validate the native FTData/motion import assumptions for EXTRA 0.5.0.

This is a metadata-only verifier. It reads the user-owned final ROM, discovers
all 19 generated Character structs, validates both motion tables for every
fighter, checks every animation file id against the expanded RELOC table and
classifies every N64 motion offset exactly the way port/remix_fighters.cpp does.
No ROM payload bytes are written anywhere.
"""
from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

from catalog_extra_fighter_structs import (
    EXPECTED_SHA1,
    EXPECTED_SIZE,
    EXTRA_FIGHTERS,
    RELOC_FILE_COUNT,
    find_structs,
    sha1,
)

RELOC_TABLE_ROM_OFFSET = 0x001AC870
RELOC_DATA_ROM_OFFSET = 0x001C2710
PATCH_RAM_ROM_DELTA = 0x7CC00000
MOTION_RAW_BASE = 0x805C0000
MOTION_RAW_END = 0x80610000
MAX_MOTION_COUNT = 4096


def be32(rom: bytes, off: int) -> int:
    return struct.unpack_from(">I", rom, off)[0]


def patch_ptr_to_rom(ptr: int) -> int | None:
    if ptr < PATCH_RAM_ROM_DELTA:
        return None
    return ptr - PATCH_RAM_ROM_DELTA


def reloc_decompressed_size(rom: bytes, file_id: int) -> int:
    if not (0 <= file_id < RELOC_FILE_COUNT):
        return 0
    _, _, _, _, decompressed_words = struct.unpack_from(
        ">IHHHH", rom, RELOC_TABLE_ROM_OFFSET + file_id * 12
    )
    return decompressed_words * 4


def classify_motion_offset(value: int) -> str:
    if value == 0x80000000:
        return "sentinel"
    if value <= 0x00100000:
        return "relative"
    if MOTION_RAW_BASE <= value < MOTION_RAW_END:
        return "remix_absolute"
    if 0x80000000 < value < 0x80400000:
        return "parent_absolute"
    return "unsupported"


def validate_motion_table(
    rom: bytes,
    name: str,
    table_name: str,
    ptr: int,
    count: int,
) -> tuple[dict, list[str]]:
    errors: list[str] = []
    counters = {
        "sentinel": 0,
        "relative": 0,
        "remix_absolute": 0,
        "parent_absolute": 0,
        "unsupported": 0,
        "nonzero_animation_ids": 0,
        "max_animation_file_id": 0,
        "max_animation_bytes": 0,
    }

    if count < 0 or count > MAX_MOTION_COUNT:
        return counters, [f"{name}: unreasonable {table_name} count {count}"]

    table_rom = patch_ptr_to_rom(ptr)
    if table_rom is None:
        return counters, [f"{name}: invalid {table_name} pointer 0x{ptr:08X}"]
    if table_rom + count * 12 > len(rom):
        return counters, [f"{name}: {table_name} table extends beyond ROM"]

    for i in range(count):
        anim_id, motion_offset, _anim_desc = struct.unpack_from(
            ">III", rom, table_rom + i * 12
        )
        if anim_id >= RELOC_FILE_COUNT:
            errors.append(
                f"{name}: {table_name}[{i}] invalid animation id {anim_id}"
            )
        elif anim_id != 0:
            counters["nonzero_animation_ids"] += 1
            counters["max_animation_file_id"] = max(
                counters["max_animation_file_id"], anim_id
            )
            counters["max_animation_bytes"] = max(
                counters["max_animation_bytes"],
                reloc_decompressed_size(rom, anim_id),
            )

        kind = classify_motion_offset(motion_offset)
        counters[kind] += 1
        if kind == "unsupported":
            errors.append(
                f"{name}: {table_name}[{i}] unsupported motion offset "
                f"0x{motion_offset:08X}"
            )

    return counters, errors


def merge_counts(a: dict, b: dict) -> dict:
    out = dict(a)
    for key, value in b.items():
        if key.startswith("max_"):
            out[key] = max(out.get(key, 0), value)
        else:
            out[key] = out.get(key, 0) + value
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    ap.add_argument("--json", type=Path)
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

    all_errors: list[str] = []
    fighters: list[dict] = []

    for index, (name, row) in enumerate(zip(EXTRA_FIGHTERS, structs)):
        off = row["rom_offset"]
        attributes_offset = be32(rom, off + 0x60)
        main_ptr = be32(rom, off + 0x64)
        sub_ptr = be32(rom, off + 0x68)
        main_count = be32(rom, off + 0x6C)
        sub_count_ptr = be32(rom, off + 0x70)

        sub_count_rom = patch_ptr_to_rom(sub_count_ptr)
        if sub_count_rom is None or sub_count_rom + 4 > len(rom):
            sub_count = 0
            fighter_errors = [
                f"{name}: invalid submotion count pointer 0x{sub_count_ptr:08X}"
            ]
        else:
            sub_count = be32(rom, sub_count_rom)
            fighter_errors = []

        main_stats, errors = validate_motion_table(
            rom, name, "mainmotion", main_ptr, main_count
        )
        fighter_errors.extend(errors)
        sub_stats, errors = validate_motion_table(
            rom, name, "submotion", sub_ptr, sub_count
        )
        fighter_errors.extend(errors)
        combined = merge_counts(main_stats, sub_stats)

        all_errors.extend(fighter_errors)
        fighters.append(
            {
                "name": name,
                "fkind": 0x61 + index,
                "character_struct_rom_offset": off,
                "attributes_offset": attributes_offset,
                "mainmotion_count": main_count,
                "submotion_count": sub_count,
                **combined,
                "errors": fighter_errors,
            }
        )

    result = {
        "profile": "smash-remix-2.0.1+extra-0.5.0",
        "rom_sha1": got_sha1,
        "fighter_count": len(fighters),
        "reloc_file_count": RELOC_FILE_COUNT,
        "motion_raw_base": MOTION_RAW_BASE,
        "motion_raw_end": MOTION_RAW_END,
        "all_fighters_supported": not all_errors,
        "error_count": len(all_errors),
        "fighters": fighters,
        "errors": all_errors,
    }

    text = json.dumps(result, indent=2)
    print(text)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(text + "\n", encoding="utf-8")

    return 0 if not all_errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
