#!/usr/bin/env python3
"""
gen_objects.py - Generate objects.json from jeff's build/config.json

Reads the split unit list produced by `jeff xex split` and emits a starter
objects.json suitable for use with configure.py.  All units are set to
"MISSING" (not yet decompiled).  Re-run whenever you re-split to pick up
new units; existing "COMPLETE" entries are preserved.

Usage:
    python3 gen_objects.py [-v VERSION] [--build BUILD_DIR] [--out OBJECTS_JSON]
                           [--mw-version VERSION] [--cflags-key KEY]
                           [--progress-category CAT]
"""

import argparse
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

from tools.defines_common import DEFAULT_VERSION, VERSIONS


def is_gap_fill(name: str) -> bool:
    """Return True for auto-generated gap-fill units (not from PDB)."""
    return re.match(r"^auto_\d{2}_[0-9A-Fa-f]{8}_", name) is not None


def unit_to_src_path(obj_name: str) -> str:
    """Convert 'ke_d/trap.obj' -> 'src/ke_d/trap.cpp'."""
    path = Path(obj_name)
    return str(Path("src") / path.with_suffix(".cpp"))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-v", "--version",
        choices=VERSIONS,
        type=str.upper,
        default=VERSIONS[DEFAULT_VERSION],
        help=f"version to build (default: {VERSIONS[DEFAULT_VERSION]})",
    )
    parser.add_argument("--build", default=None, help="Build directory (default: build/<version>)")
    parser.add_argument("--out", default=None, help="Output path (default: config/<version>/objects.json)")
    parser.add_argument("--mw-version", default="X360/16.00.11886.00", help="Compiler version string")
    parser.add_argument("--cflags-key", default="base", help="cflags key from config.json (default: base)")
    parser.add_argument("--progress-category", default="main", help="Progress category (default: main)")
    parser.add_argument("--include-gaps", action="store_true", help="Also include auto_* gap-fill units")
    args = parser.parse_args()

    if args.build is None:
        args.build = f"build/{args.version}"
    if args.out is None:
        args.out = f"config/{args.version}/objects.json"

    build_config = Path(args.build) / "config.json"
    if not build_config.exists():
        sys.exit(f"ERROR: {build_config} not found. Run `jeff xex split` first.")

    with open(build_config, encoding="utf-8") as f:
        data = json.load(f)

    units: list[dict] = data.get("units", [])

    # Load existing objects.json to preserve any "COMPLETE" / custom statuses
    out_path = Path(args.out)
    existing: dict = {}
    if out_path.exists():
        with open(out_path, encoding="utf-8") as f:
            existing = json.load(f)

    # Collect existing object statuses so we don't overwrite progress
    existing_statuses: dict[str, str] = {}
    for lib_data in existing.values():
        for src_path, status in lib_data.get("objects", {}).items():
            if isinstance(status, str):
                existing_statuses[src_path] = status
            elif isinstance(status, dict):
                existing_statuses[src_path] = status.get("status", "MISSING")

    # Group named units by their top-level directory (= library name)
    lib_objects: dict[str, list[str]] = defaultdict(list)
    skipped_gaps = 0

    for unit in units:
        name: str = unit["name"]

        if not args.include_gaps and is_gap_fill(name):
            skipped_gaps += 1
            continue

        # Group by first path component
        parts = name.split("/")
        lib = parts[0] if len(parts) > 1 else "_misc"
        # Strip .obj from lib name if it ended up there (e.g. "__Linker__.obj")
        if lib.endswith(".obj"):
            lib = "_misc"

        lib_objects[lib].append(name)

    # Build output structure
    result: dict = {}
    for lib in sorted(lib_objects):
        objects_dict: dict[str, str] = {}
        for obj_name in lib_objects[lib]:
            src = unit_to_src_path(obj_name)
            # Preserve existing status
            objects_dict[src] = existing_statuses.get(src, "MISSING")

        result[lib] = {
            "progress_category": args.progress_category,
            "mw_version": args.mw_version,
            "cflags": args.cflags_key,
            "objects": objects_dict,
        }

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=4)
        f.write("\n")

    named_count = sum(len(v) for v in lib_objects.values())
    print(f"Wrote {out_path}: {named_count} units in {len(result)} libraries")
    if skipped_gaps:
        print(f"  (skipped {skipped_gaps} auto-generated gap-fill units; use --include-gaps to include)")


if __name__ == "__main__":
    main()
