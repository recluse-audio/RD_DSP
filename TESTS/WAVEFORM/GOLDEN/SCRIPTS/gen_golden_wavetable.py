#!/usr/bin/env python3

"""
This doesn't generate a wavetable per-se,
rather it assembles one out of already generated
waveforms.

Usage: python3 gen_golden_wavetable.py /waveform/dir/path --output /wavetable/output/path
"""
from __future__ import annotations
import argparse
import csv
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser(description="Combine multiple single-row Waveform csv into one Wavetable csv.")
    parser.add_argument("directory", type=str, help="Directory to search for Waveform csv files")
    parser.add_argument("--output", type=str, default="Wavetable.csv", help="Output CSV path")
    parser.add_argument("--order", type=str, default=None,
                        help="Comma-separated CSV basenames giving explicit row order "
                             "(e.g. wavePosition order sine,tri,square,saw); overrides sorted glob")
    args = parser.parse_args()

    directory = Path(args.directory)
    if not directory.is_dir():
        parser.error(f"Not a directory: {directory}")

    if args.order:
        # Explicit row order: rows come out exactly as listed, not alphabetically.
        csv_paths = [directory / name.strip() for name in args.order.split(",")]
        for path in csv_paths:
            if not path.is_file():
                parser.error(f"Ordered file not found: {path}")
    else:
        # sorted() so row order is deterministic, not filesystem-dependent
        csv_paths = sorted(directory.glob("*.csv"))
        if not csv_paths:
            parser.error(f"No CSV files found in {directory}")

    rows = []
    for path in csv_paths:
        with path.open(newline="") as f:
            reader = csv.reader(f)
            first_row = next(reader, None)   # take the single row
            if first_row is None:
                print(f"Skipping empty file: {path.name}")
                continue
            rows.append(first_row)
            print(f"Loaded {path.name} ({len(first_row)} values)")

    out_path = Path(args.output)
    with out_path.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(rows)

    print(f"Wrote {len(rows)} rows to {out_path}")


if __name__ == "__main__":
    main()
