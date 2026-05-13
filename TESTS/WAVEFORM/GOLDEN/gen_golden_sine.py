#!/usr/bin/env python3
"""
Generate golden sine-wave CSV used by Catch2 tests.

Format: single row, N comma-separated amplitudes, no header.
    amplitude[i] = sin(2*pi*i/N)

Run:
    python TESTS/WAVEFORM/GOLDEN/gen_golden_sine.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parent / "SINE"


def write_golden_sine(num_samples: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_SINE_{num_samples}.csv"

    amplitudes = [math.sin(2.0 * math.pi * i / num_samples) for i in range(num_samples)]
    with out_path.open("w", newline="") as f:
        f.write(",".join(repr(a) for a in amplitudes) + "\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_golden_sine(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
