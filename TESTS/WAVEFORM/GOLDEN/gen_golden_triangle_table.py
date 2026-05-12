#!/usr/bin/env python3
"""
Generate golden triangle-wave table CSV used by Catch2 tests.

Format matches sine golden CSV:
    index,normalized_phase,pi_radians,amplitude

One full cycle across N samples. Triangle is band-limited via additive
synthesis of odd harmonics with 1/n^2 amplitude and alternating sign:

    amplitude[i] = (8/pi^2) * sum_{k=0..K} (-1)^k * sin(2*pi*(2k+1)*i/N) / (2k+1)^2

K is fixed (mellow triangle) so playback aliasing stays low across the
oscillator's frequency range. Matches the C++ Waveform::_fillWithTri()
implementation sample-for-sample.

Run:
    python TESTS/WAVEFORM/GOLDEN/gen_golden_triangle_table.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR     = Path(__file__).resolve().parent / "TRIANGLE"
NUM_HARMONICS  = 31  # K — number of odd harmonics summed (mellow)
NUM_WAVES = 12

def triangle_sample(i: int, num_samples: int, wave_num: int) -> float:
    coeff = 8.0 / (math.pi * math.pi)
    acc = 0.0
    num_harmonics = 5 + (wave_num * 2)
    if num_harmonics > NUM_HARMONICS:
        num_harmonics = NUM_HARMONICS

        
    for k in range(num_harmonics + 1):
        n    = 2 * k + 1
        sign = -1.0 if (k & 1) else 1.0
        acc += sign * math.sin(2.0 * math.pi * n * i / num_samples) / (n * n)
    return coeff * acc


def write_golden_triangle_table(num_samples: int, num_waves: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_TRIANGLE_TABLE_{num_samples}.csv"
        
    with out_path.open("w", newline="") as f:
        for wave_num in range(num_waves):
            amplitudes = [triangle_sample(i, num_samples, wave_num) for i in range(num_samples)]
            row = ",".join(repr(a) for a in amplitudes)
            f.write(row + "\n")

    return out_path


def main() -> None:
    for sample_num in (8096,):
        path = write_golden_triangle_table(sample_num, NUM_WAVES)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
