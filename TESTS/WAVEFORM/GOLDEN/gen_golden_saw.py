#!/usr/bin/env python3
"""
Generate golden saw-wave CSV used by Catch2 tests.

Format: single row, N comma-separated amplitudes, no header.

One full cycle across N samples. Saw is band-limited via additive synthesis
of all harmonics with 1/n amplitude and alternating sign (ascending saw):

    amplitude[i] = (2/pi) * sum_{n=1..N_H} (-1)^(n+1) * sin(2*pi*n*i/N) / n

N_H fixed (mellow saw) so playback aliasing stays low. Matches C++
Waveform::_fillWithSaw() sample-for-sample.

Run:
    python TESTS/WAVEFORM/GOLDEN/gen_golden_saw.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parent / "SAW"
NUM_HARMONICS = 32


def saw_sample(i: int, num_samples: int) -> float:
    coeff = 2.0 / math.pi
    acc = 0.0
    for n in range(1, NUM_HARMONICS + 1):
        sign = 1.0 if (n % 2 == 1) else -1.0
        acc += sign * math.sin(2.0 * math.pi * n * i / num_samples) / n
    return coeff * acc


def write_golden_saw(num_samples: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_SAW_{num_samples}.csv"

    amplitudes = [saw_sample(i, num_samples) for i in range(num_samples)]
    with out_path.open("w", newline="") as f:
        f.write(",".join(repr(a) for a in amplitudes) + "\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_golden_saw(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
