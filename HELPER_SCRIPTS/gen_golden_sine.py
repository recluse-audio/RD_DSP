#!/usr/bin/env python3
"""
Generate a golden sine-wave CSV used by Catch2 tests.

Format matches the RD project's golden CSVs:
    index,normalized_phase,pi_radians,amplitude

One full cycle across N samples. amplitude[i] = sin(2*pi*i/N).
Last sample is NOT zero (it is sin(2*pi*(N-1)/N)).

Run:
    python HELPER_SCRIPTS/gen_golden_sine.py
"""
from __future__ import annotations

import math
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = REPO_ROOT / "TESTS" / "GOLDEN" / "SINE"


def write_golden_sine(num_samples: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_SINE_{num_samples}.csv"

    with out_path.open("w", newline="") as f:
        f.write("index,normalized_phase,pi_radians,amplitude\n")
        for i in range(num_samples):
            normalized_phase = i / num_samples
            pi_radians = 2.0 * normalized_phase
            amplitude = math.sin(2.0 * math.pi * i / num_samples)
            f.write(f"{i},{normalized_phase!r},{pi_radians!r},{amplitude!r}\n")

    return out_path


def main() -> None:
    for n in (8096,):
        path = write_golden_sine(n)
        print(f"Wrote {path}")


if __name__ == "__main__":
    main()
