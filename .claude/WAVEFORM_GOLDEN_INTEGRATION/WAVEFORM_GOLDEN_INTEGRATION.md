# Waveform Golden Integration — PLAN
Goal: Make the C++ waveform generation path match the regenerated golden files (which now
apply RMS normalization plus a `tanh` peak-protection soft-clip), and bring the goldens that
were not yet regenerated into the same pipeline so all 9 failing tests pass for the right reason.
Effort: medium (confirmed by inference; say the word to widen to high)

## Status
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- last commit: 1fa28bc
- updated: 2026-07-07 12:59 CDT / 2026-07-07T17:59Z

## Decision (resolved)
Source of truth = the regenerated golden files. C++ is brought up to match them. `tanh`
peak protection becomes real DSP output, not just a golden-file artifact.
- ❌ Revert golden `tanh` — abandoned; user wants the soft-clip shipped.
- ❌ Investigate-only pass — folded into PLAN_PHASE_0 as a scoped diagnostic step.

## Two mechanisms (the 9 failures split cleanly)
- **Bucket A — C++ generation must match a regenerated golden** (a C++ code change is the fix):
  1. Waveform sine fill · 2. Waveform tri fill · 3. Waveform saw fill · 4. Waveform square fill
  (needs BOTH a C++ change AND square golden regen — square golden was NOT regenerated) ·
  5. WaveFactory fundamental-only sine · 6. Sine multi-harmonic == simple sine.
- **Bucket B — CSV-loading tests; NO C++ generation runs; fix is regenerating downstream goldens**
  in dependency order (a C++ waveform-gen change canNOT fix these):
  7. WaveformFileManager (`test_WaveformFileManager.cpp:122`) — compares the basic-table sine
     row (stale) to the regenerated individual sine golden. Fix: regen the table.
  8. Synth (`test_Synth.cpp:521`) — loads the wavetable, runs C++ `Synth.process`, compares to
     `SYNTH/GOLDEN/OUTPUT/*`. Fix: regen synth-output goldens from the current wavetable.
  9. Pulsar (`test_PulsarTrain.cpp:710`) — `gen_golden_pulsar.py` builds a PURE analytic
     `sine*window`, never loading the wavetable, while C++ windows the loaded wavetable sine.
     Fix: make the generator window the ACTUAL wavetable sine row (or reconcile amplitude).

## The tanh tension (must be resolved consciously)
`0.95*tanh(1.0/0.95) ≈ 0.743`. Peak-protecting the sine drops its peak from 1.0 to ~0.743.
Any downstream golden that assumes a unit-amplitude sine (pulsar generator, likely synth
generator) is invalidated unless regenerated FROM the normalized/protected wavetable. This is
why Bucket B cannot be a simple "cascade" — the golden generators themselves must derive from
the same source the C++ loads.

## Objectives (testable outcomes)
1. [must] C++ `WaveFactory::normalizeWaveform` performs RMS-normalize-to-target then
   `0.95 * tanh(s / 0.95)` peak protection, matching `gen_golden_waveform.py` step-for-step.
2. [must] `Waveform` shape fills (sine, tri, square, saw) use the same harmonic-gain tables
   as the GOLDEN harmonic JSONs, then run through the normalize+protect stage. (Bucket A)
3. [must] All four per-shape golden CSVs regenerated from the SAME `gen_golden_waveform.py`,
   then the basic table and BasicShapes wavetable reassembled from them via
   `gen_golden_wavetable.py` (square, basic table, wavetable were NOT regenerated). (Bucket B 7)
4. [must] Downstream golden generators (`gen_golden_pulsar.py`, the synth-output generator)
   derive their expected output from the regenerated wavetable, not a re-synthesized analytic
   sine, then their goldens are regenerated. (Bucket B 8, 9)
5. [must] The 9 currently failing tests pass, and the previously-passing 160 stay green.
6. [nice] Compute-precision policy is settled (double-internal vs float32) so the 1e-6 test
   margins are met without loosening them, or the margins are consciously widened with a note.

## The interface(s) we wire to (existing tools reused)
- `TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_waveform.py`
  `--harmonic_data <JSON>` `--num_samples 8192` `--target_rms 0.7071067811865476`
  Pipeline: sum harmonics (`sin(TWO_PI*ratio*i/N + phaseOffset)*gain`) → `normalize_rms`
  → `protect_peaks` (`0.95*tanh(s/0.95)`) → single-row CSV of `repr(float)`.
- `TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_wavetable.py <dir> --output <csv>`
  assembles the per-shape CSVs (sorted) into one multi-row wavetable CSV.
- Harmonic JSONs in `TESTS/WAVEFORM/GOLDEN/HARMONIC_DATA/` are the amplitude source
  (e.g. Square: ratios 1,3,5..15 with gains 1, 0.333.., 0.2 ..). C++ must embed equivalents;
  the core lib may NOT read these files (zero-dep, no file I/O for AUv3/WASM).
- C++ seams: `WaveFactory::normalizeWaveform` (empty stub, `WaveFactory.cpp:87`),
  `WaveFactory::fillWaveformWithHarmonics` (`WaveFactory.cpp:59`),
  `Waveform::_fillWith{Sine,Tri,Square,Saw}` (`Waveform.cpp:65-151`, analytic today).

## Architecture / flow
```
HARMONIC_DATA/*.json  ──(python)──>  gen_golden_waveform.py ──> per-shape GOLDEN csv
        │  (same ratios/gains embedded as C++ constants)              │
        ▼                                                             ▼ (assemble)
Waveform::_fillWith* ── sum harmonics ── WaveFactory::normalizeWaveform ──> compare
                                          (rms-normalize + tanh protect)   in tests
```
The Python and the C++ must agree at every stage: harmonic sum, RMS target, tanh ceiling.

## Config that lives in THIS repo
- Harmonic tables (ratios/gains per shape): source of truth = `HARMONIC_DATA/*.json`
  (test fixtures) mirrored into a C++ constants header under `SOURCE/WAVEFORM/`.
- Golden CSVs: `TESTS/WAVEFORM/GOLDEN/WAVEFORM/` and `.../WAVETABLES/`.
- Normalization constants (`target_rms=1/sqrt(2)`, `peak_ceiling=0.95`) duplicated in Python
  and C++; keep them named and side-by-side-commented so drift is visible.

## Phased steps

### PLAN_PHASE_0 — Diagnose and lock precision (before touching DSP)
- **PLAN_STEP_0_1** (effort: S, obj 4/5) Quantify all 9 failures: for each, dump generated
  vs golden at the failing index and classify as (a) tanh/RMS layer, tiny ~1e-6, or
  (b) method gap, large. Confirm the square/basic-table/wavetable goldens are pre-regen.
- **PLAN_STEP_0_2** (effort: S, obj 5) Decide compute precision. Python is float64 end to end,
  writing full `repr`. C++ `Waveform` stores float32. Pick: compute fills+normalize in double
  and only narrow to float32 on store (keeps 1e-6 reachable), OR widen the 1e-6 margins in
  `test_Waveform.cpp:52` to 1e-5 with a comment. Record the choice in PLAN_HISTORY.

### PLAN_PHASE_1 — Port normalization into C++
- **PLAN_STEP_1_1** (effort: M, obj 1) Implement `WaveFactory::normalizeWaveform`: compute RMS
  over the buffer, scale to `target_rms` (guard RMS==0), then apply `0.95*tanh(s/0.95)`
  per sample. No allocation, no exceptions (hot-path rules). Add a `[WaveFactory]` unit test
  that feeds a known buffer and checks the two-stage result.
- **PLAN_STEP_1_2** (effort: S, obj 1) Route `fillWaveformWithHarmonics` callers through
  `normalizeWaveform` so the WaveFactory sine tests (`test_WaveFactory.cpp`) match.

### PLAN_PHASE_2 — Rework Waveform shape fills to the harmonic method
- **PLAN_STEP_2_1** (effort: M, obj 2) Add a C++ harmonic-table constants header mirroring the
  four GOLDEN JSONs (sine=fundamental; square/tri/saw ratios+gains). Values must equal the JSON.
- **PLAN_STEP_2_2** (effort: M, obj 2) Rewrite `_fillWithSine/Tri/Square/Saw` to sum those
  harmonics (matching `add_harmonic_to_waveform`) then call `normalizeWaveform`. Delete the
  analytic `4/pi`, `8/pi^2`, `2/pi` coefficient loops. This is what closes the large square gap.

### PLAN_PHASE_3 — Regenerate the golden chain in dependency order (Bucket B, upstream half)
Order matters: individual waveforms → wavetable/basic table → downstream outputs.
- **PLAN_STEP_3_1** (effort: S, obj 3) Regenerate square, and re-confirm sine/tri/saw, via
  `gen_golden_waveform.py` so every per-shape CSV carries the new tanh layer. Fixes Bucket A
  square golden half.
- **PLAN_STEP_3_2** (effort: S, obj 3) Reassemble `GOLDEN_Wavetable_BasicShapes_8192.csv` and the
  `BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8192.csv` with `gen_golden_wavetable.py` from the
  regenerated per-shape CSVs. Fixes WaveformFileManager (failure 7).
- **PLAN_STEP_3_3** (effort: S, obj 5) Run the suite; confirm the six waveform-family failures
  (Bucket A + failure 7) are green and nothing regressed.

### PLAN_PHASE_4 — Fix the downstream generators to derive from the wavetable (Bucket B, failures 8-9)
- **PLAN_STEP_4_1** (effort: M, obj 4) Pulsar: `gen_golden_pulsar.py` currently synthesizes
  `sine(2*pi*i*f/sr) * window(i)` with NO wavetable input (confirmed, script lines ~5-8). Change
  it to LOAD the sine row from `GOLDEN_Wavetable_BasicShapes_8192.csv` and window THAT, mirroring
  `PulsarTrain` C++, so the golden and the C++ input share one source. Regenerate
  `PULSAR/GOLDEN/OUTPUT/GOLDEN_PULSAR_TUKEY_48000SR_200Hz_1C.csv`. Re-run `[PulsarTrain]`.
- **PLAN_STEP_4_2** (effort: M, obj 4) Synth: locate the generator for `SYNTH/GOLDEN/OUTPUT/*`
  (find it first — may not exist as a script yet). Ensure it resamples the regenerated wavetable,
  not an analytic shape, then regenerate the four `GOLDEN_SYNTH_*` CSVs. Re-run `[Synth]`.
- **PLAN_STEP_4_3** (effort: S, obj 5) Full suite green: 169/169. Record commit in PLAN_HISTORY.

## Open questions (resolve before the step that needs them)
- Q1 (blocks PLAN_STEP_0_2): double-internal compute vs widen 1e-6 margins? Lean double-internal
  to preserve the strict goldens, but confirm the buffer stays float32-backed.
- Q2 (blocks PLAN_STEP_4_2): does a script for `SYNTH/GOLDEN/OUTPUT/*` exist, or were those
  goldens produced ad hoc? Find it before deciding regenerate-vs-write-new-generator.
- Q3 (blocks PLAN_STEP_2_1): the JSON gains are un-normalized amplitudes (sq fundamental gain
  1.0), unlike `WaveFactory::_initHarmonicData` which uses 0.5. Confirm the fills seed from the
  JSON-equivalent gains, not the existing 0.5 default.
- Q4 (blocks PLAN_PHASE_2 acceptance): confirm tanh peak-protection is applied to the SINE too.
  It drops the sine peak to ~0.743, which is audible and forces every downstream sine golden to
  be regenerated from the wavetable. If sine should stay unit-amplitude, `protect_peaks` must be
  conditional (only shapes that clip), which changes both the Python and the C++ normalize stage.

## Local-only (NOT tracked)
- `BUILD/` output. Nothing secret here.

## Per-new-device checklist
- Python 3 with `numpy` for the golden scripts (`gen_golden_waveform.py` imports numpy).
- CMake + a C++20 toolchain; Catch2 fetched by the build.
- Regenerate goldens only on an intentional DSP change, never casually (they are the oracle).
