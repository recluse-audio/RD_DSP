#!/usr/bin/env python3
"""
Generate golden triangle-wave CSV used by Catch2 tests.

Format: single row, N comma-separated amplitudes, no header.

One full cycle across N samples. Triangle is band-limited via additive
synthesis of odd harmonics with 1/n^2 amplitude and alternating sign:

    amplitude[i] = (8/pi^2) * sum_{k=0..K} (-1)^k * sin(2*pi*(2k+1)*i/N) / (2k+1)^2

K fixed (mellow triangle) so playback aliasing stays low. Matches C++
Waveform::_fillWithTri() sample-for-sample.

Run:
    python TESTS/WAVEFORM/GOLDEN/gen_golden_triangle.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parent / "TRIANGLE"
NUM_HARMONICS = 31


def triangle_sample(i: int, num_samples: int) -> float:
    coeff = 8.0 / (math.pi * math.pi)
    acc = 0.0
    for k in range(NUM_HARMONICS + 1):
        n = 2 * k + 1
        sign = -1.0 if (k & 1) else 1.0
        acc += sign * math.sin(2.0 * math.pi * n * i / num_samples) / (n * n)
    return coeff * acc


def write_golden_triangle(num_samples: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_TRIANGLE_{num_samples}.csv"

    amplitudes = [triangle_sample(i, num_samples) for i in range(num_samples)]
    with out_path.open("w", newline="") as f:
        f.write(",".join(repr(a) for a in amplitudes) + "\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_golden_triangle(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
