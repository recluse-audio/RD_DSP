# Waveform Golden Integration — SUMMARY

## What gets built / added
| What | Why | Where |
|------|-----|-------|
| 4 golden variants per waveform | RAW, RMS_SCALED, PEAK_SCALED, RMS_PEAK_SCALED oracles | `TESTS/WAVEFORM/GOLDEN/WAVEFORMS/<VARIANT>/*.csv` |
| 4 golden variants per wavetable | same four scalings, assembled | `TESTS/WAVEFORM/GOLDEN/WAVETABLES/<VARIANT>/*.csv` |
| `gen_golden_waveform.py` refactor | decouple RMS from tanh; emit all four variants | `TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_waveform.py` |
| regen driver | one command rebuilds all variants × shapes + wavetables | `HELPER_SCRIPTS/` (new) |
| nlohmann/json (header lib) | C++ reads HARMONIC_DATA JSON | CMake FetchContent, linked to `Tests` |
| rapidcsv (header lib) | C++ reads golden CSV rows | CMake FetchContent, linked to `Tests` |
| 4 C++ scaling stages | raw / rmsScale / peakScale(tanh) / rmsPeakScale | `SOURCE/WAVEFORM/WaveFactory.*` |
| per-variant tests | assert C++ == golden for each level | `TESTS/WAVEFORM/*` |

## Scaling definitions
- **RAW** — harmonic sum only. No scaling. Peaks may exceed 1.0.
- **RMS_SCALED** — scale so RMS == 0.7071 (1/sqrt(2)).
- **PEAK_SCALED** — tanh soft-clip only: `0.95 * tanh(x / 0.95)`.
- **RMS_PEAK_SCALED** — RMS-scale, THEN tanh. Order matters. Matches current committed goldens.

## Acronyms
- **RMS** — Root Mean Square, an amplitude/loudness measure.
- **tanh** — hyperbolic tangent, the soft-clip curve.
- **CSV** — Comma-Separated Values.
- **AUv3 / WASM** — Apple audio-unit extension / WebAssembly; the restrictive no-file-I/O targets.

## In repo vs. on machine
- **Repo (synced):** the four-variant `WAVEFORMS/` and `WAVETABLES/<VARIANT>/` CSVs, the
  refactored scripts, HARMONIC_DATA JSONs, CMake fetch declarations.
- **Local-only (per machine):** `BUILD/` and the FetchContent download cache (nlohmann, rapidcsv).

## Dependencies (permanent)
- nlohmann/json + rapidcsv are permanent project deps via FetchContent, usable anywhere including
  the core lib. No zero-dep caveat, no build guard.
- Downstream consumers (Synth, Pulsar, WaveformFileManager) use the `RMS_PEAK_SCALED` variant.

## How to use it
- Regenerate all goldens: run the new `HELPER_SCRIPTS/` driver (regens 4 variants × 4 shapes + 4 wavetables).
- Reassemble one variant table: `python TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_wavetable.py TESTS/WAVEFORM/GOLDEN/WAVEFORMS/<VARIANT> --output TESTS/WAVEFORM/GOLDEN/WAVETABLES/<VARIANT>/GOLDEN_Wavetable_BasicShapes_8192.csv`
- Build + run tests: `python HELPER_SCRIPTS/build_tests.py --run`.
