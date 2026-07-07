# Waveform Golden Integration — SUMMARY

## What changes
| What | Why | Where |
|------|-----|-------|
| Fill `normalizeWaveform` | Add RMS-normalize + `tanh` peak-protect to match goldens | `SOURCE/WAVEFORM/WaveFactory.cpp:87` |
| Rewrite shape fills | Use harmonic-gain tables, drop analytic `4/pi` etc. | `SOURCE/WAVEFORM/Waveform.cpp:65-151` |
| New harmonic-constants header | Embed JSON ratios/gains (core lib can't read JSON) | new file under `SOURCE/WAVEFORM/` |
| Regenerate square + basic table + wavetable goldens | They were NOT regenerated; still stale | `TESTS/WAVEFORM/GOLDEN/WAVEFORM/`, `.../WAVETABLES/` |
| Precision decision | Meet strict 1e-6 test margins | `TESTS/WAVEFORM/test_Waveform.cpp:52` |

## The mismatch in one line
- Goldens now do: sum harmonics → RMS-normalize to `0.7071` → `0.95*tanh(s/0.95)`.
- C++ does: analytic Fourier coefficients, `normalizeWaveform` empty. So they diverge.

## Failure map (9) — two mechanisms
- **Bucket A — C++ generation change is the fix:** Waveform sine/tri/saw/square fills,
  WaveFactory sine (x2). (Square also needs its golden regenerated.)
- **Bucket B — no C++ generation runs; regenerate downstream goldens:**
  - WaveformFileManager: table sine row (stale) vs regenerated individual sine golden.
  - Synth: loads wavetable, compares to `SYNTH/GOLDEN/OUTPUT/*` (must regen from wavetable).
  - Pulsar: `gen_golden_pulsar.py` makes a PURE analytic sine*window, never loads the wavetable.

## tanh tension
- `0.95*tanh(1.0/0.95) ≈ 0.743` — protecting the sine drops its peak to ~0.743, not 1.0.
- Every downstream sine golden must be regenerated FROM the normalized wavetable, or sine must
  be exempted from `protect_peaks`. Decide before implementing (Open Q4).

## Acronyms
- **RMS** — Root Mean Square, an energy-based amplitude measure.
- **DSP** — Digital Signal Processing.
- **AUv3 / WASM** — Apple AudioUnit v3 / WebAssembly; restricted targets banning file I/O and deps.

## In repo vs. on machine
- **Repo (synced):** C++ sources, harmonic JSONs, golden CSVs, python generators, this plan.
- **Local-only (per machine):** `BUILD/` artifacts.

## How to use it
- Regenerate one shape: `python TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_waveform.py --harmonic_data <JSON>`
- Reassemble table: `python TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_wavetable.py <waveform_dir> --output <csv>`
- Build + test: `python HELPER_SCRIPTS/build_tests.py --run`
- Single tag: `./Tests "[WaveFactory]"`
