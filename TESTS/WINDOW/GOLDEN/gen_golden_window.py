#!/usr/bin/env python3
"""
Generate golden window CSVs used by Catch2 tests.

Format: single row, N comma-separated amplitudes, no header.

Shapes:
    none    -> all 1.0
    hanning -> 0.5 * (1 - cos(2*pi*n/(N-1)))
    tukey   -> cosine-tapered rectangular, alpha=0.8 (matches RD default)

Run:
    python TESTS/WINDOW/GOLDEN/gen_golden_window.py
"""
from __future__ import annotations

import math
from pathlib import Path

OUTPUT_DIR = Path(__file__).resolve().parent
TUKEY_ALPHA = 0.8


def hanning(num_samples: int) -> list[float]:
    if num_samples <= 1:
        return [0.0] * num_samples
    denom = float(num_samples - 1)
    return [0.5 * (1.0 - math.cos(2.0 * math.pi * i / denom)) for i in range(num_samples)]


def tukey(num_samples: int, alpha: float) -> list[float]:
    if num_samples <= 1:
        return [0.0] * num_samples
    if alpha <= 0.0:
        return [1.0] * num_samples

    N = float(num_samples - 1)
    taper_end = alpha * N * 0.5
    out: list[float] = []
    for i in range(num_samples):
        n = float(i)
        if n < taper_end:
            v = 0.5 * (1.0 + math.cos(math.pi * (n / taper_end - 1.0)))
        elif n <= N - taper_end:
            v = 1.0
        else:
            v = 0.5 * (1.0 + math.cos(math.pi * ((n - (N - taper_end)) / taper_end)))
        out.append(v)
    return out


def none_shape(num_samples: int) -> list[float]:
    return [1.0] * num_samples


def write_row(path: Path, values: list[float]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
        f.write(",".join(repr(v) for v in values) + "\n")


def main() -> None:
    sizes = (8096,)
    for n in sizes:
        none_path = OUTPUT_DIR / "NONE" / f"GOLDEN_WINDOW_NONE_{n}.csv"
        han_path  = OUTPUT_DIR / "HANNING" / f"GOLDEN_WINDOW_HANNING_{n}.csv"
        tuk_path  = OUTPUT_DIR / "TUKEY" / f"GOLDEN_WINDOW_TUKEY_{n}.csv"

        write_row(none_path, none_shape(n))
        write_row(han_path,  hanning(n))
        write_row(tuk_path,  tukey(n, TUKEY_ALPHA))

        print(f"Wrote {none_path}")
        print(f"Wrote {han_path}")
        print(f"Wrote {tuk_path}")


if __name__ == "__main__":
    main()
