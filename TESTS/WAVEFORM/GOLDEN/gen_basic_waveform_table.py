#!/usr/bin/env python3
"""
Generate a basic waveform-table CSV used by Catch2 tests.

Each row is one full-cycle waveform (N samples across columns):
    row 0 = sine
    row 1 = band-limited triangle
    row 2 = band-limited square
    row 3 = band-limited ascending saw

No header. Sample math mirrors SOURCE/WAVEFORM/Waveform.cpp fills.

Run:
    python TESTS/WAVEFORM/GOLDEN/gen_basic_waveform_table.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parent / "BASIC_TABLE"


def sine_sample(i: int, n: int) -> float:
    return math.sin(2.0 * math.pi * i / n)


def tri_sample(i: int, n: int) -> float:
    K = 31
    coeff = 8.0 / (math.pi * math.pi)
    acc = 0.0
    for k in range(K + 1):
        m = 2 * k + 1
        sign = -1.0 if (k & 1) else 1.0
        acc += sign * math.sin(2.0 * math.pi * m * i / n) / (m * m)
    return coeff * acc


def square_sample(i: int, n: int) -> float:
    K = 31
    coeff = 4.0 / math.pi
    acc = 0.0
    for k in range(K + 1):
        m = 2 * k + 1
        acc += math.sin(2.0 * math.pi * m * i / n) / m
    return coeff * acc


def saw_sample(i: int, n: int) -> float:
    K = 32
    coeff = 2.0 / math.pi
    acc = 0.0
    for m in range(1, K + 1):
        sign = 1.0 if (m % 2 == 1) else -1.0
        acc += sign * math.sin(2.0 * math.pi * m * i / n) / m
    return coeff * acc


WAVE_FNS = (sine_sample, tri_sample, square_sample, saw_sample)


def write_basic_table(num_samples: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_BASIC_WAVEFORM_TABLE_{num_samples}.csv"

    with out_path.open("w", newline="") as f:
        for fn in WAVE_FNS:
            amplitudes = [fn(i, num_samples) for i in range(num_samples)]
            f.write(",".join(repr(a) for a in amplitudes) + "\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_basic_table(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
