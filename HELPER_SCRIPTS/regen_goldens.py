#!/usr/bin/env python3

"""
Regenerate the entire golden waveform + wavetable tree, all four scaling variants.

For every JSON in TESTS/WAVEFORM/GOLDEN/HARMONIC_DATA/ this:
    1. runs gen_golden_waveform.py to emit the four per-shape variant CSVs
       into WAVEFORMS/<VARIANT>/, then
    2. runs gen_golden_wavetable.py once per variant to assemble
       WAVETABLES/<VARIANT>/GOLDEN_Wavetable_BasicShapes_8192.csv

Goldens are the test oracle: only run this on an intentional DSP change.

Run:
    python3 HELPER_SCRIPTS/regen_goldens.py
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
GOLDEN = REPO_ROOT / "TESTS" / "WAVEFORM" / "GOLDEN"
HARMONIC_DATA = GOLDEN / "HARMONIC_DATA"
SCRIPTS = GOLDEN / "SCRIPTS"
WAVEFORMS = GOLDEN / "WAVEFORMS"
WAVETABLES = GOLDEN / "WAVETABLES"

VARIANTS = ["RAW", "RMS_SCALED", "PEAK_SCALED", "RMS_PEAK_SCALED"]
NUM_SAMPLES = 8192
WAVETABLE_NAME = f"GOLDEN_Wavetable_BasicShapes_{NUM_SAMPLES}.csv"


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True)


def main() -> None:
    py = sys.executable  # same interpreter that launched this driver

    jsons = sorted(HARMONIC_DATA.glob("*.json"))
    if not jsons:
        sys.exit(f"No harmonic-data JSON found in {HARMONIC_DATA}")

    # 1. Per-shape waveforms, all four variants each.
    for json_path in jsons:
        run([py, str(SCRIPTS / "gen_golden_waveform.py"),
             "--harmonic_data", str(json_path),
             "--num_samples", str(NUM_SAMPLES)])

    # 2. One wavetable per variant, assembled from that variant's per-shape CSVs.
    for variant in VARIANTS:
        src_dir = WAVEFORMS / variant
        out_path = WAVETABLES / variant / WAVETABLE_NAME
        out_path.parent.mkdir(parents=True, exist_ok=True)
        run([py, str(SCRIPTS / "gen_golden_wavetable.py"),
             str(src_dir), "--output", str(out_path)])

    print("Done: regenerated", len(jsons), "shapes x", len(VARIANTS), "variants + wavetables.")


if __name__ == "__main__":
    main()
