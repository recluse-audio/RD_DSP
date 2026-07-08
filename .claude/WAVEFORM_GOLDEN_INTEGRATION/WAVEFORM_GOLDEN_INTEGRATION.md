# Waveform Golden Integration — PLAN
Goal: Generate four scaling variants (RAW, RMS_SCALED, PEAK_SCALED, RMS_PEAK_SCALED) of every
golden waveform and wavetable, output to a new per-variant directory layout, then match each
level in C++. C++ reads the harmonic JSON (nlohmann) and golden CSVs (rapidcsv) directly.
Effort: medium (confirmed by inference; say the word to widen to high)

## Status
- activeFocus: "WAVEFORM_GOLDEN_INTEGRATION :: PLAN_PHASE_4 :: PLAN_STEP_4_3"
- last commit: 61303ae (+ user checkpoint; PLAN_PHASE_4 changes uncommitted)
- updated: 2026-07-07 20:01 CDT / 2026-07-08T01:01Z
- FULL SUITE GREEN: 170/170 test cases, 406873 assertions. Legacy dirs retired into
  GOLDEN/DEPRECATED/. Pulsar generator moved to PULSAR/GOLDEN/SCRIPTS/. All objectives met.
  Optional follow-up: relocate the SYNTH generator under SYNTH/GOLDEN/SCRIPTS; regen stale
  M76/M81 synth goldens.

## Decisions (resolved this session)
- **Four scaling variants, not one.** Prior "RMS-only, no tanh" single-answer decision is
  ABANDONED. We now emit four independent golden sets and match each in C++:
  - `RAW` — harmonic sum only, no RMS, no peak. Peaks may exceed 1.0.
  - `RMS_SCALED` — RMS-normalize to `target_rms` only.
  - `PEAK_SCALED` — tanh soft-clip (`protect_peaks`, ceiling 0.95) only, no RMS.
  - `RMS_PEAK_SCALED` — RMS-normalize, THEN tanh soft-clip (the current committed behavior).
- **Peak operation = tanh soft-clip** `y = ceiling * tanh(x / ceiling)` (adds harmonics,
  compresses peaks). NOT peak-normalize-to-1.0, NOT a >1.0 clamp. (User-selected.)
- **Constants (shared Python + C++):** `target_rms = 1/sqrt(2) = 0.7071067811865476`,
  tanh `ceiling = 0.95`.
- **nlohmann/json + rapidcsv are permanent dependencies**, brought in via CMake FetchContent,
  usable anywhere including the core lib. The zero-dep / no-file-I/O concern is retired for this
  project; do not raise it again.
- **HARMONIC_DATA JSONs are the single source of truth for BOTH Python and C++.** C++ parses
  them with nlohmann and seeds the harmonic sum from the JSON `gain` values. The hardcoded 0.5
  fundamental gain in `WaveFactory::_initHarmonicData` is removed. (Q3 resolved.)
- **Precision: compute in double, narrow to float32 on store.** Both Python and the C++ golden
  path compute the sum + scaling in double, then store as float32 so the golden represents the
  exact C++ `float` value. Test tolerance can stay tight (1e-6). (Q4 resolved.)
- ❌ Single-variant golden set — abandoned (user wants all four).
- ❌ Peak-normalize-to-1.0 and >1.0 clamp — abandoned (peak op is tanh).
- ❌ Hardcoded 0.5 fundamental gain / widening tolerance to 1e-5 — abandoned.

## New directory layout (the "S" plural is intentional)
```
TESTS/WAVEFORM/GOLDEN/
  HARMONIC_DATA/                      (unchanged — amplitude source JSONs)
  SCRIPTS/                            (generators)
  WAVEFORMS/                          (NEW plural, replaces singular WAVEFORM/)
    RAW/              GOLDEN_<Shape>_..._8192.csv   (per shape)
    RMS_SCALED/       GOLDEN_<Shape>_..._8192.csv
    PEAK_SCALED/      GOLDEN_<Shape>_..._8192.csv
    RMS_PEAK_SCALED/  GOLDEN_<Shape>_..._8192.csv
  WAVETABLES/
    RAW/              GOLDEN_Wavetable_BasicShapes_8192.csv
    RMS_SCALED/       ...
    PEAK_SCALED/      ...
    RMS_PEAK_SCALED/  ...
```
Old `WAVEFORM/` (singular), `ALT_WAVEFORMS/`, `SAW/` are legacy; the current committed CSVs
were made with RMS+tanh, i.e. they map to `RMS_PEAK_SCALED`. Migrate then remove (PLAN_PHASE_4).

## Objectives (testable outcomes)
0. [must] `gen_golden_waveform.py` refactored so the four scaling stages are independent and one
   invocation emits all four variant CSVs into `WAVEFORMS/<VARIANT>/`. `protect_peaks` and
   `normalize_rms` become composable, not hard-wired together.
1. [must] `gen_golden_wavetable.py` (or a driver) assembles one wavetable per variant into
   `WAVETABLES/<VARIANT>/` from that variant's per-shape CSVs.
2. [must] All four variants exist on disk for the four shapes (sine, tri, square, saw) and for
   the BasicShapes wavetable.
3. [must] nlohmann/json and rapidcsv added via CMake FetchContent and available to both the
   core lib and `Tests`. No guard, no zero-dep caveat.
4. [must] C++ exposes the four scaling stages (`raw` sum, `rmsScale`, `peakScale` tanh,
   `rmsPeakScale`) matching the Python constants and order step-for-step.
5. [must] Tests read HARMONIC_DATA JSON (nlohmann) + golden CSV (rapidcsv), generate each C++
   variant, and assert C++ variant == matching golden variant within tolerance.
6. [must] Full suite green; no previously-passing test regresses.
7. [nice] Compute-precision policy settled (double-internal vs float32) so 1e-6 margins hold.

## The interface(s) we wire to (existing tools reused)
- `TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_waveform.py`
  Current pipeline: sum harmonics (`add_harmonic_to_waveform`: `sin(TWO_PI*ratio*i/N + phase)*gain`)
  → `normalize_rms` (which today ALSO calls `protect_peaks` internally, `gen_golden_waveform.py:75`).
  Refactor target: decouple `normalize_rms` from `protect_peaks`; add a variant loop.
- `TESTS/WAVEFORM/GOLDEN/SCRIPTS/gen_golden_wavetable.py <dir> --output <csv>`
  Assembles sorted per-shape CSVs in a dir into one multi-row wavetable CSV. Run once per variant.
- Harmonic JSONs `TESTS/WAVEFORM/GOLDEN/HARMONIC_DATA/*.json` — `root["HarmonicData"]` list of
  `{ratio, gain, phaseOffset?}`. Read in C++ via nlohmann.
- C++ seams: `WaveFactory::normalizeWaveform` (`SOURCE/.../WaveFactory.cpp:87`),
  `WaveFactory::fillWaveformWithHarmonics` (`WaveFactory.cpp:59`),
  `Waveform::_fillWith{Sine,Tri,Square,Saw}` (`Waveform.cpp:65-151`).

## Architecture / flow
```
HARMONIC_DATA/*.json ──nlohmann──> harmonic sum ──> [RAW]
                                        │
                                        ├─ rmsScale ─────────────> [RMS_SCALED]
                                        ├─ peakScale(tanh) ──────> [PEAK_SCALED]
                                        └─ rmsScale ∘ peakScale ─> [RMS_PEAK_SCALED]
Python writes the 4 golden variants; C++ produces the same 4; rapidcsv loads goldens; assert ==.
```
Python and C++ must agree at every stage: harmonic sum, `target_rms`, tanh `ceiling`, and the
RMS-then-tanh ORDER for the combined variant.

## Config that lives in THIS repo
- Harmonic tables: `HARMONIC_DATA/*.json` (single source of truth, now read by BOTH Python and C++).
- Golden CSVs: `TESTS/WAVEFORM/GOLDEN/WAVEFORMS/<VARIANT>/` and `.../WAVETABLES/<VARIANT>/`.
- Shared constants (`target_rms=1/sqrt(2)`, `ceiling=0.95`): named + side-by-side-commented in
  Python and in a C++ constants header so drift is visible.

## Phased steps

### PLAN_PHASE_0 — Refactor generators, emit the four-variant golden tree (Python is source of truth)
- **PLAN_STEP_0_1** (effort: M, obj 0) In `gen_golden_waveform.py`, decouple the stages: keep
  `rms`/`normalize_rms` (RMS only) and `protect_peaks` (tanh only) as pure, independent functions.
  Add a variant map `{RAW: identity, RMS_SCALED: normalize_rms, PEAK_SCALED: protect_peaks,
  RMS_PEAK_SCALED: protect_peaks∘normalize_rms}`. Add `--all` (or `--variant`) that writes each
  variant CSV to `WAVEFORMS/<VARIANT>/<stem>_<N>.csv`. `OUTPUT_DIR` moves to `.../WAVEFORMS`.
- **PLAN_STEP_0_2** (effort: S, obj 1) Assemble wavetables per variant: run `gen_golden_wavetable.py`
  once per `WAVEFORMS/<VARIANT>/` dir, output to `WAVETABLES/<VARIANT>/GOLDEN_Wavetable_BasicShapes_8192.csv`.
- **PLAN_STEP_0_3** (effort: S, obj 2) Add a small driver (extend `HELPER_SCRIPTS/` or a shell-free
  Python) that regenerates all four shapes × four variants + the four wavetables in one command.
  Run it; commit the new `WAVEFORMS/` and `WAVETABLES/<VARIANT>/` tree.

### PLAN_PHASE_1 — CMake: add nlohmann/json + rapidcsv  [DONE]
- **PLAN_STEP_1_1** [DONE] (effort: M, obj 3) Both are git submodules under `SUBMODULES/`, wired via
  `add_subdirectory` (not FetchContent), linked to `RD_DSP`, `Standalone`, and `Tests`
  (`CMakeLists.txt:50-56, 82-86, 103-108`). Submodule self-tests disabled (`JSON_BuildTests`,
  `RAPIDCSV_BUILD_TESTS`). Tests get `RD_DSP_TESTS_DIR` define to locate goldens at runtime.
  Verified with a clean `build_tests.py`.

### PLAN_PHASE_2 — C++ produces the RAW harmonic sum; scaling is test-side
DECISION (user): the core `WaveFactory` is NOT responsible for scaling. It keeps only the setters
(`setHarmonicDataValues`) and `fillWaveformWithHarmonics`, which produce the RAW waveform. No
ScalingVariant enum, no rms/peak methods on the class. The rms/peak scaling (taking float target
+ ceiling args) lives in a TEST-SIDE helper and is applied to the generated waveform before it is
compared to the matching golden variant.
- **PLAN_STEP_2_1** [DONE] (effort: S, obj 4) Real `WaveFactory` methods (NOT a test helper):
  `applyScaleRMS(Waveform&, float targetRms=kDefaultTargetRMS)` and
  `applyPeakNormalization(Waveform&, float ceiling=kDefaultPeakCeiling)`, matching Python
  step-for-step: RMS in double, scale, then `ceiling*tanh(s/ceiling)`; guard RMS==0; narrow to
  float32 on store. Replaced the empty `normalizeWaveform` stub. Tests call these directly.
- **PLAN_STEP_2_2** (effort: M, obj 4) `fillWaveformWithHarmonics` produces the RAW harmonic sum
  seeded from the JSON `gain` values (set via `setHarmonicDataValues` from the test's nlohmann parse),
  accumulated in double and narrowed to float32 once on store (so it matches the RAW golden). Remove
  the hardcoded 0.5 default in `_initHarmonicData` if it interferes.

### PLAN_PHASE_3 — Tests assert each level
- **PLAN_STEP_3_1** (effort: M, obj 5) Test helper: load `HARMONIC_DATA/*.json` via nlohmann, set the
  harmonics, build the RAW C++ waveform, then for each variant apply the matching test-side scaling
  helper, load the golden CSV via rapidcsv, and assert equality within tolerance. One `TEST_CASE`
  per shape, `SECTION` per variant against `WAVEFORMS/<VARIANT>/`.
- **PLAN_STEP_3_2** (effort: S, obj 5) Wavetable-level test: same pattern against
  `WAVETABLES/<VARIANT>/`.
- **PLAN_STEP_3_3** (effort: S, obj 6) Full suite green; nothing regressed.

### PLAN_PHASE_4 — Migrate legacy goldens + downstream consumers
- **PLAN_STEP_4_1** (effort: S, obj 6) Point existing consumers (WaveformFileManager, Synth, Pulsar
  tests) at the `RMS_PEAK_SCALED` variant (matches the current committed CSVs).
- **PLAN_STEP_4_2** (effort: S, obj 6) Remove legacy `WAVEFORM/` (singular), `ALT_WAVEFORMS/`, `SAW/`
  once nothing references them. Re-run the suite.
- **PLAN_STEP_4_3** (effort: S, obj 6) Full suite green. Record commit in PLAN_HISTORY.

## Open questions
- None currently open. Q1-Q4 resolved (see Decisions). Raise new ones here as steps surface them.

## Local-only (NOT tracked)
- `BUILD/` output. FetchContent downloads (nlohmann, rapidcsv) cached under `BUILD/`.

## Per-new-device checklist
- Python 3 + `numpy` for the golden scripts.
- CMake + C++20 toolchain; Catch2, nlohmann/json, rapidcsv fetched by the build (network needed
  on first configure).
- Regenerate goldens only on an intentional DSP change (they are the oracle). One driver command
  regenerates all four variants × shapes + wavetables.
