#!/usr/bin/env python3
"""
Generate golden phase-increment CSV for the Oscillator.

Mirrors C++ Oscillator::_calculatePhaseIncrement:
    phase_increment = freq * waveform_size / sample_rate

The CSV records (freq, sample_rate, waveform_size, phase_increment) tuples.
The C++ test reads it and calls Oscillator::_calculatePhaseIncrement via
OscillatorTester (friend) using the waveform_size value from the CSV row,
so this script never needs to know the C++ default size.

Output columns:
    freq,sample_rate,waveform_size,phase_increment

Usage:
    # default table of cases at default waveform size:
    python TESTS/OSCILLATOR/GOLDEN/generate_golden_phase_increment.py

    # override:
    python TESTS/OSCILLATOR/GOLDEN/generate_golden_phase_increment.py \
        --freq 441 --sample-rate 44100 --waveform-size 8096
"""
from __future__ import annotations

import argparse
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
OUTPUT_DIR = SCRIPT_DIR / "PHASE_INCREMENT"

DEFAULT_WAVEFORM_SIZE = 8096

# (freq_hz, sample_rate_hz)
DEFAULT_CASES: list[tuple[float, float]] = [
    (   100.0, 44100.0),
    (   220.0, 44100.0),
    (   440.0, 44100.0),
    (   441.0, 44100.0),
    (  1000.0, 44100.0),
    ( 10000.0, 44100.0),
    (   440.0, 48000.0),
    (   440.0, 96000.0),
    (   440.0, 22050.0),
    (     1.0, 44100.0),
]


def phase_increment(freq: float, sample_rate: float, waveform_size: int) -> float:
    return freq * waveform_size / sample_rate


def write_table(cases: list[tuple[float, float]], waveform_size: int) -> Path:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GOLDEN_PHASE_INCREMENT_{waveform_size}.csv"

    with out_path.open("w", newline="") as f:
        f.write("freq,sample_rate,waveform_size,phase_increment\n")
        for freq, sr in cases:
            inc = phase_increment(freq, sr, waveform_size)
            f.write(f"{freq!r},{sr!r},{waveform_size},{inc!r}\n")

    return out_path


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--freq",          type=float, default=None, help="oscillator frequency in Hz (single case)")
    ap.add_argument("--sample-rate",   type=float, default=None, help="sample rate in Hz (single case)")
    ap.add_argument("--waveform-size", type=int,   default=DEFAULT_WAVEFORM_SIZE, help="waveform table size")
    args = ap.parse_args()

    if (args.freq is None) ^ (args.sample_rate is None):
        ap.error("--freq and --sample-rate must be supplied together")

    if args.freq is not None:
        cases = [(args.freq, args.sample_rate)]
    else:
        cases = DEFAULT_CASES

    path = write_table(cases, args.waveform_size)
    print(f"Wrote {path}  (waveform_size={args.waveform_size})")


if __name__ == "__main__":
    main()
