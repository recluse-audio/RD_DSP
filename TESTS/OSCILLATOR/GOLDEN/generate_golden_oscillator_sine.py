#!/usr/bin/env python3
"""
Generate golden oscillator sine output CSV for one cycle at a given frequency
and sample rate, by reading the golden Waveform sine CSV at the adjusted
phase increment with linear interpolation.

Mirrors C++ Oscillator: phase_increment = freq * waveform_size / sample_rate.

Output: one cycle of the oscillator at the requested frequency, sampled at
sample_rate. Number of output samples = round(sample_rate / freq).

Usage:
    python TESTS/OSCILLATOR/GOLDEN/generate_golden_oscillator_sine.py --freq 441 --sample-rate 44100
"""
from __future__ import annotations

import argparse
import csv
from pathlib import Path

SCRIPT_DIR     = Path(__file__).resolve().parent
WAVEFORM_CSV   = SCRIPT_DIR.parent.parent / "WAVEFORM" / "GOLDEN" / "SINE" / "GOLDEN_SINE_8096.csv"
OUTPUT_DIR     = SCRIPT_DIR / "SINE"


def load_waveform_amplitudes(csv_path: Path) -> list[float]:
    samples: list[float] = []
    with csv_path.open("r", newline="") as f:
        reader = csv.reader(f)
        next(reader)  # header
        for row in reader:
            # columns: index, normalized_phase, pi_radians, amplitude
            samples.append(float(row[3]))
    return samples


def linear_interp(s0: float, s1: float, frac: float) -> float:
    return s0 + (s1 - s0) * frac


def read_at(waveform: list[float], index: float) -> float:
    n = len(waveform)
    wrapped = index % n
    i0 = int(wrapped)
    i1 = (i0 + 1) % n
    frac = wrapped - i0
    return linear_interp(waveform[i0], waveform[i1], frac)


def generate(freq: float, sample_rate: float) -> Path:
    waveform = load_waveform_amplitudes(WAVEFORM_CSV)
    waveform_size = len(waveform)

    phase_increment = freq * waveform_size / sample_rate
    num_output = round(sample_rate / freq)  # samples for one cycle

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    freq_tag = f"{freq:g}".replace(".", "p")
    sr_tag   = f"{sample_rate:g}".replace(".", "p")
    out_path = OUTPUT_DIR / f"GOLDEN_OSC_SINE_{freq_tag}Hz_{sr_tag}SR.csv"

    with out_path.open("w", newline="") as f:
        f.write("index,phase_index,amplitude\n")
        for i in range(num_output):
            phase_index = (i * phase_increment) % waveform_size
            amplitude   = read_at(waveform, i * phase_increment)
            f.write(f"{i},{phase_index!r},{amplitude!r}\n")

    return out_path


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--freq",        type=float, required=True, help="oscillator frequency in Hz")
    ap.add_argument("--sample-rate", type=float, required=True, help="output sample rate in Hz")
    args = ap.parse_args()

    path = generate(args.freq, args.sample_rate)
    print(f"Wrote {path}")


if __name__ == "__main__":
    main()
