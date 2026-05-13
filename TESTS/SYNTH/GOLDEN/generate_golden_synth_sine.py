#!/usr/bin/env python3
"""
Generate golden Synth output CSV for one cycle of a chosen base waveform
played at a given MIDI note and sample rate.

Mirrors C++ SynthVoice path:
    freq = 440 * 2^((midiNote - 69) / 12)
    phase_increment = freq * waveform_size / sample_rate

Output: one cycle at the freq corresponding to the MIDI note, sampled at
sample_rate. num samples = round(sample_rate / freq).

Usage:
    python TESTS/SYNTH/GOLDEN/generate_golden_synth_sine.py --midi 69 --sample-rate 44100
    python TESTS/SYNTH/GOLDEN/generate_golden_synth_sine.py --midi 69 --sample-rate 44100 --waveform triangle
"""
from __future__ import annotations

import argparse
from pathlib import Path

SCRIPT_DIR     = Path(__file__).resolve().parent
WAVEFORM_ROOT  = SCRIPT_DIR.parent.parent / "WAVEFORM" / "GOLDEN"
OUTPUT_DIR     = SCRIPT_DIR.parent / "OUTPUT"

WAVEFORM_FILES = {
    "sine":     WAVEFORM_ROOT / "SINE"     / "GOLDEN_SINE_8096.csv",
    "triangle": WAVEFORM_ROOT / "TRIANGLE" / "GOLDEN_TRIANGLE_8096.csv",
    "square":   WAVEFORM_ROOT / "SQUARE"   / "GOLDEN_SQUARE_8096.csv",
    "saw":      WAVEFORM_ROOT / "SAW"      / "GOLDEN_SAW_8096.csv",
}


def midi_to_hertz(midi_note: int) -> float:
    return 440.0 * (2.0 ** ((midi_note - 69) / 12.0))


def load_waveform_amplitudes(csv_path: Path) -> list[float]:
    # Horizontal layout: one line, 8096 comma-separated amplitudes, no header.
    text = csv_path.read_text().strip()
    return [float(tok) for tok in text.split(",") if tok.strip()]


def linear_interp(s0: float, s1: float, frac: float) -> float:
    return s0 + (s1 - s0) * frac


def read_at(waveform: list[float], index: float) -> float:
    n = len(waveform)
    wrapped = index % n
    i0 = int(wrapped)
    i1 = (i0 + 1) % n
    frac = wrapped - i0
    return linear_interp(waveform[i0], waveform[i1], frac)


def generate(midi_note: int, sample_rate: float, shape: str) -> Path:
    waveform = load_waveform_amplitudes(WAVEFORM_FILES[shape])
    waveform_size = len(waveform)

    freq = midi_to_hertz(midi_note)
    phase_increment = freq * waveform_size / sample_rate
    num_output = round(sample_rate / freq)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    sr_tag = f"{sample_rate:g}".replace(".", "p")
    out_path = OUTPUT_DIR / f"GOLDEN_SYNTH_{shape.upper()}_M{midi_note}_{sr_tag}SR.csv"

    with out_path.open("w", newline="") as f:
        f.write("index,phase_index,amplitude\n")
        for i in range(num_output):
            phase_index = (i * phase_increment) % waveform_size
            amplitude   = read_at(waveform, i * phase_increment)
            f.write(f"{i},{phase_index!r},{amplitude!r}\n")

    return out_path


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--midi",        type=int,   required=True, help="MIDI note number")
    ap.add_argument("--sample-rate", type=float, required=True, help="output sample rate in Hz")
    ap.add_argument("--waveform",    type=str,   default="sine",
                    choices=sorted(WAVEFORM_FILES.keys()), help="source waveform shape")
    args = ap.parse_args()

    path = generate(args.midi, args.sample_rate, args.waveform)
    print(f"Wrote {path}")


if __name__ == "__main__":
    main()
