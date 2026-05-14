#!/usr/bin/env python3
"""
Generate golden windowed-sine CSV for PulsarTrain tests.

Produces N = cycles * round(sample_rate / formant_freq) samples of:
    sine(2*pi*i*formant_freq / sample_rate) * window(i)
where window(i) is sampled across its full length over the N samples.

Defaults match the PulsarTrain end-to-end test setup:
    sample_rate = 48000
    window      = tukey
    cycles      = 1
    formant     = 200 Hz

Output: TESTS/PULSAR/GOLDEN/OUTPUT/GOLDEN_PULSAR_<WINDOW>_<SR>SR_<F>Hz_<C>C.csv
Format: header row "amplitude", then one amplitude per row (BufferFiller-compatible).

Usage:
    python TESTS/PULSAR/GOLDEN/gen_golden_pulsar.py
    python TESTS/PULSAR/GOLDEN/gen_golden_pulsar.py --sample-rate 44100 --window hanning --cycles 2
"""
from __future__ import annotations

import argparse
import math
from pathlib import Path

OUTPUT_DIR  = Path(__file__).resolve().parent / "OUTPUT"
TUKEY_ALPHA = 0.8

WINDOWS = ("none", "hanning", "tukey")


def hanning(num_samples: int) -> list[float]:
    if num_samples <= 1:
        return [0.0] * num_samples
    denom = float(num_samples - 1)
    return [0.5 * (1.0 - math.cos(2.0 * math.pi * i / denom)) for i in range(num_samples)]


def tukey(num_samples: int, alpha: float = TUKEY_ALPHA) -> list[float]:
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


def window_values(name: str, num_samples: int) -> list[float]:
    if name == "none":
        return none_shape(num_samples)
    if name == "hanning":
        return hanning(num_samples)
    if name == "tukey":
        return tukey(num_samples)
    raise ValueError(f"unknown window: {name}")


def windowed_sine(sample_rate: float, formant_freq: float, cycles: int, window_name: str) -> list[float]:
    n_samples = cycles * round(sample_rate / formant_freq)
    win       = window_values(window_name, n_samples)
    out: list[float] = []
    for i in range(n_samples):
        sine = math.sin(2.0 * math.pi * i * formant_freq / sample_rate)
        out.append(sine * win[i])
    return out


def write_column(path: Path, values: list[float]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
        f.write("amplitude\n")
        for v in values:
            f.write(f"{v!r}\n")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--sample-rate", type=float, default=48000.0,                 help="sample rate in Hz")
    ap.add_argument("--window",      type=str,   default="tukey", choices=WINDOWS, help="window shape")
    ap.add_argument("--cycles",      type=int,   default=1,                        help="number of formant cycles")
    ap.add_argument("--formant",     type=float, default=200.0,                    help="formant frequency in Hz")
    args = ap.parse_args()

    values = windowed_sine(args.sample_rate, args.formant, args.cycles, args.window)

    sr_tag = f"{args.sample_rate:g}".replace(".", "p")
    f_tag  = f"{args.formant:g}".replace(".", "p")
    out    = OUTPUT_DIR / f"GOLDEN_PULSAR_{args.window.upper()}_{sr_tag}SR_{f_tag}Hz_{args.cycles}C.csv"

    write_column(out, values)
    print(f"Wrote {out} ({len(values)} samples)")


if __name__ == "__main__":
    main()
