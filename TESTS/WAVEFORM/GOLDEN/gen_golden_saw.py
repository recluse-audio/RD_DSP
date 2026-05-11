#!/usr/bin/env python3
"""
Generate golden saw-wave CSV used by Catch2 tests.

Format matches sine golden CSV:
    index,normalized_phase,pi_radians,amplitude

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

OUTPUT_DIR    = Path(__file__).resolve().parent / "SAW"
NUM_HARMONICS = 32  # n=1..NUM_HARMONICS


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

    with out_path.open("w", newline="") as f:
        f.write("index,normalized_phase,pi_radians,amplitude\n")
        for i in range(num_samples):
            normalized_phase = i / num_samples
            pi_radians       = 2.0 * normalized_phase
            amplitude        = saw_sample(i, num_samples)
            f.write(f"{i},{normalized_phase!r},{pi_radians!r},{amplitude!r}\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_golden_saw(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
