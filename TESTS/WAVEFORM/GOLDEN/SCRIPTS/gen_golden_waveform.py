#!/usr/bin/env python3

"""
Generate golden waveforms from harmonic-data JSON.

For every harmonic-data JSON we emit FOUR scaling variants of the same summed waveform:

    RAW              harmonic sum only, no scaling (peaks may exceed 1.0)
    RMS_SCALED       RMS-normalize to target_rms only
    PEAK_SCALED      tanh soft-clip only            (ceiling * tanh(x / ceiling))
    RMS_PEAK_SCALED  RMS-normalize, THEN tanh soft-clip (order matters)

Each variant is written to its own subdirectory:

    WAVEFORMS/<VARIANT>/<stem>_<num_samples>.csv

The HARMONIC_DATA JSON is the single source of truth for BOTH this script and the C++
implementation (which parses the same JSON via nlohmann). Constants (target_rms, ceiling)
are duplicated in C++; keep them in sync.

Precision: the summing and scaling are done in Python float (float64 / double); each sample
is narrowed to float32 on store so the golden represents the exact C++ `float` value.

Run (one JSON, all four variants):
    python TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_waveform.py \
        --harmonic_data TESTS/WAVEFORM/GOLDEN/HARMONIC_DATA/GOLDEN_SineWave_HarmonicData.json
"""
from __future__ import annotations

import argparse
import math
from pathlib import Path
import json
import numpy

# Root of the four per-variant waveform output directories.
OUTPUT_DIR = Path(__file__).resolve().parent.parent / "WAVEFORMS"

TWO_PI = 6.28318530717958647692

# Shared with the C++ implementation; keep the values identical.
DEFAULT_TARGET_RMS = 0.7071067811865476   # 1 / sqrt(2)
PEAK_CEILING = 0.95


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


# --- Scaling primitives (each is pure and independent) ---------------------

# RMS is taken so we can normalize waves like square that are too "loud" and tri too "quiet".
# This is due to peak length (square waves sit at peak for most of the duty cycle).
def rms(samples):
    return math.sqrt(sum(s * s for s in samples) / len(samples))


def normalize_rms(waveform, target_rms):
    """RMS-normalize only. No peak protection."""
    current = rms(waveform)
    if current == 0.0:
        return list(waveform)
    scale = target_rms / current
    return [s * scale for s in waveform]


def protect_peaks(waveform, peak_ceiling=PEAK_CEILING):
    """tanh soft-clip only. No RMS."""
    return [peak_ceiling * math.tanh(s / peak_ceiling) for s in waveform]


# --- The four scaling variants ---------------------------------------------

def scale_raw(waveform, target_rms):
    return list(waveform)


def scale_rms(waveform, target_rms):
    return normalize_rms(waveform, target_rms)


def scale_peak(waveform, target_rms):
    return protect_peaks(waveform)


def scale_rms_peak(waveform, target_rms):
    return protect_peaks(normalize_rms(waveform, target_rms))


# Order here defines the on-disk subdirectory names.
VARIANTS = {
    "RAW": scale_raw,
    "RMS_SCALED": scale_rms,
    "PEAK_SCALED": scale_peak,
    "RMS_PEAK_SCALED": scale_rms_peak,
}


def write_waveform_csv(waveform, out_path: Path) -> Path:
    """Write waveform as a single row of comma-separated amplitudes, no header.

    Each sample is narrowed to float32 first so the stored text is the exact C++ `float`
    value, then repr'd as its double expansion.
    """
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", newline="") as f:
        f.write(",".join(repr(float(numpy.float32(a))) for a in waveform) + "\n")
    return out_path


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate the four scaling variants of a golden waveform.")
    parser.add_argument('--harmonic_data', type=str, required=True,
                        help='HARMONIC_DATA JSON to build the waveform from')
    parser.add_argument('--num_samples', type=int, default=8192, help='Waveform length in samples')
    parser.add_argument('--target_rms', type=float, default=DEFAULT_TARGET_RMS,
                        help='RMS target for the RMS_SCALED / RMS_PEAK_SCALED variants')
    parser.add_argument('--variant', type=str, choices=sorted(VARIANTS.keys()),
                        help='Emit only this one variant (default: all four)')
    parser.add_argument('--output_root', type=str, default=None,
                        help='Override the WAVEFORMS root; per-variant subdirs are created under it')
    args = parser.parse_args()

    with open(args.harmonic_data) as f:
        root = json.load(f)

    harmonics = root["HarmonicData"]

    # Build the RAW harmonic sum once; every variant scales a copy of it.
    base = [0.0] * args.num_samples
    for harmonic in harmonics:
        add_harmonic_to_waveform(base, harmonic)

    stem = Path(args.harmonic_data).stem
    output_root = Path(args.output_root) if args.output_root else OUTPUT_DIR

    selected = [args.variant] if args.variant else list(VARIANTS.keys())
    for variant in selected:
        scaled = VARIANTS[variant](base, args.target_rms)
        out_path = output_root / variant / f"{stem}_{args.num_samples}.csv"
        write_waveform_csv(scaled, out_path)
        print(f"Wrote {out_path}")


if __name__ == "__main__":
    main()
