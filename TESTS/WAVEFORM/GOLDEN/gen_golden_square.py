#!/usr/bin/env python3
"""
Generate golden square-wave CSV used by Catch2 tests.

Format matches sine golden CSV:
    index,normalized_phase,pi_radians,amplitude

One full cycle across N samples. Square is band-limited via additive synthesis
of odd harmonics with 1/n amplitude (all positive):

    amplitude[i] = (4/pi) * sum_{k=0..K} sin(2*pi*(2k+1)*i/N) / (2k+1)

K fixed (mellow square) so playback aliasing stays low. Matches C++
Waveform::_fillWithSquare() sample-for-sample.

Run:
    python TESTS/WAVEFORM/GOLDEN/gen_golden_square.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR    = Path(__file__).resolve().parent / "SQUARE"
NUM_HARMONICS = 31  # K — k=0..K, odd harmonics 1,3,...,2K+1


def square_sample(i: int, num_samples: int) -> float:
    coeff = 4.0 / math.pi
    acc = 0.0
    for k in range(NUM_HARMONICS + 1):
        n    = 2 * k + 1
        acc += math.sin(2.0 * math.pi * n * i / num_samples) / n
    return coeff * acc


def write_golden_square(num_samples: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_SQUARE_{num_samples}.csv"

    with out_path.open("w", newline="") as f:
        f.write("index,normalized_phase,pi_radians,amplitude\n")
        for i in range(num_samples):
            normalized_phase = i / num_samples
            pi_radians       = 2.0 * normalized_phase
            amplitude        = square_sample(i, num_samples)
            f.write(f"{i},{normalized_phase!r},{pi_radians!r},{amplitude!r}\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_golden_square(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
