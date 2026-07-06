#!/usr/bin/env python3

"""
This generates sine waves with harmonics according to a series of args

Run:
    python TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_waveform.py
"""
from __future__ import annotations

import argparse
import math
from pathlib import Path
import json
import numpy

OUTPUT_DIR = Path(__file__).resolve().parent.parent / "WAVEFORM"

def parse_json_file(file_path):
    try:
        with open(file_path, 'r') as json_file:
            data = json.load(json_file)
            print(f"Parsed JSON data:\n{data}")
    except FileNotFoundError:
        print(f"Error: The file at {file_path} does not exist.")
    except json.JSONDecodeError:
        print(f"Error: The file at {file_path} contains invalid JSON.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")



TWO_PI = 6.28318530717958647692

def add_harmonic_to_waveform(waveform, harmonic) -> None:
    """Additively sum one harmonic's sine into waveform in place.

    Matches C++ WaveFactory::_writeHarmonicToWaveform sample-for-sample:
        phasePos       = TWO_PI * ratio * i / N
        harmonicSample = sin(phasePos + phaseOffset) * gain
    """
    num_samples = len(waveform)
    ratio = harmonic["ratio"]
    gain = harmonic["gain"]
    # phaseOffset is optional in the JSON; C++ default is 0.0
    phase_offset = harmonic.get("phaseOffset", 0.0)

    for i in range(num_samples):
        phase_pos = (TWO_PI * ratio * i) / num_samples
        waveform[i] += math.sin(phase_pos + phase_offset) * gain


def write_waveform_csv(waveform, out_path: Path) -> Path:
    """Write waveform as a single row of comma-separated amplitudes, no header."""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", newline="") as f:
        f.write(",".join(repr(a) for a in waveform) + "\n")
    return out_path

# RMS is taken so we can normalize waves like square that are too "loud" and tri too "quiet"
# This is due to peak length, (Square waves are supposed to be at peak for most of duty cycle)
# This is a common thing to do apparently
def rms(samples):
    return math.sqrt(sum(s * s for s in samples) / len(samples))

def normalize_rms(waveform, target_rms):
    current = rms(waveform)
    if current == 0.0:
        return waveform
    scale = target_rms / current
    return [s * scale for s in waveform]

def main() -> None:
    parser = argparse.ArgumentParser(description='script that accepts arguments')
    parser.add_argument('--input', type=str, help='Input file path')
    parser.add_argument('--harmonic_data', type=str, help='These are the harmonics from which to make waveforms')
    parser.add_argument('--output', type=str, help='Output file path')
    parser.add_argument('--num_samples', type=int, default=8192, help='Waveform length in samples')
    parser.add_argument('--target_rms', type=float, default=0.7071067811865476, help='RMS target for whole waveform')
    args = parser.parse_args()

    if not args.harmonic_data:
        parser.error("--harmonic_data is required")

    with open(args.harmonic_data) as f:
        root = json.load(f)

    # get list of harmonic_data(s)
    harmonics = root["HarmonicData"]

    # start with silence, then sum each harmonic in
    waveform = [0.0] * args.num_samples
    for harmonic in harmonics:
        add_harmonic_to_waveform(waveform, harmonic)

    # do normalization of waveform
    waveform = normalize_rms(waveform, args.target_rms)

    if args.output:
        out_path = Path(args.output)
    else:
        # derive a name from the harmonic_data file, e.g. GOLDEN_TriangleWave_...
        stem = Path(args.harmonic_data).stem
        out_path = OUTPUT_DIR / f"{stem}_{args.num_samples}.csv"

    write_waveform_csv(waveform, out_path)
    print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
