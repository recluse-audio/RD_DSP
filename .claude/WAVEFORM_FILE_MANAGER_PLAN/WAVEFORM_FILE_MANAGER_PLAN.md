# WaveformFileManager parity — PLAN
Goal: bring `WaveformFileManager` (rapidcsv-based) to functional and test parity with
`WaveFactory`'s CSV-loading methods (hand-rolled `CsvLoader`-based), for both single-waveform
and wavetable loading. Scope is strictly additive to `WaveformFileManager` — `WaveFactory`,
`CsvLoader`, and their tests are untouched; no renames; cleanup/consolidation is deferred to
the user.

Effort: low (confirmed: narrow, additive-only scope)

## Status
- activeFocus: "WAVEFORM_FILE_MANAGER_PLAN :: PLAN_PHASE_0 :: PLAN_STEP_0_1"
- last commit: 1afffc6
- updated: 2026-07-02 11:19 CDT

## Objectives
- [must] `WaveformFileManager::fillFromCSV` correctly parses the single-row golden sine CSV
  (fixes the current `rapidcsv::Document` header-row bug); existing
  `test_WaveformFileManager.cpp` "loads golden csv" case goes green.
- [must] `WaveformFileManager` gains a single-`Waveform` loader (CSV -> one `Waveform`)
  matching `WaveFactory::loadWaveformFromCSV`'s behavior, with a mirrored test.
- [must] `WaveformFileManager::fillFromCSV` (the `Wavetable` path) gets test coverage
  mirroring `WaveFactory`'s basic-table test: 4 waveforms, `wavePos` 0/0.25/0.5/0.75 checked
  against golden sine/tri/square/saw.
- [must] Full `Tests` suite green; `WaveFactory.{h,cpp}`, `test_WaveFactory.cpp`, and
  `CsvLoader.{h,cpp}` remain byte-for-byte unchanged.

## Root cause of the current failure
`WaveformFileManager::fillFromCSV` (`SOURCE/WAVEFORM/WaveformFileManager.h:30`) builds
`rapidcsv::Document(csvPath)` with default `LabelParams` (`mColumnNameIdx = 0`,
`mRowNameIdx = -1` — see `SUBMODULES/rapidcsv/src/rapidcsv.h:350`). That treats CSV row 0 as
a column-name header. The golden sine CSV has exactly one data row, so it gets consumed as
the header, `GetRowCount()` becomes 0, no waveform is added, and
`Wavetable::getSampleAtIndex` (`SOURCE/WAVEFORM/Wavetable.cpp:26-30`) falls back to
returning `0.f` for every index (empty-wavetable guard). Sample index 0 coincidentally
matches because `sin(0) == 0`; index 1 does not, which is exactly the observed failure.

Fix: construct the `rapidcsv::Document` with `rapidcsv::LabelParams(-1, -1)` — no header row,
no row-label column — matching how `CsvLoader::load` treats the golden files today
(`skipHeader=false`).

## The interface(s) we wire to
- `rapidcsv::Document` ctor + `LabelParams(-1, -1)`, `GetRowCount()`, `GetRow<double>(idx)` —
  already vendored at `SUBMODULES/rapidcsv/src/rapidcsv.h`, already linked via
  `CMakeLists.txt:51,55` (target `rapidcsv`). No new dependency.
- `rd_dsp::Wavetable::addWaveform/clear/getNumWaveforms/setNormalizedWavePosition/
  getSampleAtIndex` (`SOURCE/WAVEFORM/Wavetable.h`) — reused as-is.
- `rd_dsp::CsvLoader::load` (`SOURCE/HELPERS/CsvLoader.h`) — reused only as the independent
  golden-file read path inside tests (as `test_WaveFactory.cpp` already does), not touched.

## Architecture / flow
`WaveformFileManager` (header-only, `SOURCE/WAVEFORM/WaveformFileManager.h`) after this plan:
- `fillFromCSV(Wavetable&, path)` — existing, `LabelParams` bug fixed. One row -> one
  `Waveform` -> `Wavetable`.
- `loadWaveformFromCSV(path)` — new. Same corrected `rapidcsv::Document` read; returns
  `nullptr` if 0 rows, else builds one `Waveform` from row 0 via `getWaveformFromRow`.
- `getWaveformFromRow(vector<double>)` — existing helper, reused by both of the above.

## Phased steps

- [ ] **PLAN_STEP_0_1** (effort: S) — traces to objective 1. In
  `SOURCE/WAVEFORM/WaveformFileManager.h::fillFromCSV`, change
  `rapidcsv::Document waveCSV(csvPath);` to pass `rapidcsv::LabelParams(-1, -1)` so no row is
  treated as a header and no column as row labels. Rebuild, run
  `./Tests "WaveformFileManager loads golden csv"`, confirm green.

- [ ] **PLAN_STEP_1_1** (effort: S) — traces to objective 2. Add
  `static std::unique_ptr<rd_dsp::Waveform> loadWaveformFromCSV(const std::string& csvPath)`
  to `WaveformFileManager`, using the same corrected `rapidcsv::Document` read: return
  `nullptr` if `GetRowCount() == 0`, else `getWaveformFromRow(GetRow<double>(0))`.

- [ ] **PLAN_STEP_1_2** (effort: S) — traces to objective 2. In
  `TESTS/WAVEFORM/test_WaveformFileManager.cpp`, add a `TEST_CASE` mirroring
  `test_WaveFactory.cpp`'s "WaveFactory loads golden sine CSV and samples match golden file":
  call `WaveformFileManager::loadWaveformFromCSV` on the golden sine path, compare every
  sample against the `CsvLoader`-read golden row (same `Catch::Approx` margin).

- [ ] **PLAN_STEP_2_1** (effort: S) — traces to objective 3. In the same test file, add a
  `TEST_CASE` mirroring "WaveFactory loads basic waveform table; wavePos=0 returns sine row":
  `fillFromCSV` a `Wavetable` from `GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv`, assert
  `getNumWaveforms() == 4`, then for `wavePos` in {0, 0.25, 0.5, 0.75} compare
  `getSampleAtIndex` against the matching golden sine/tri/square/saw waveform loaded via the
  new `loadWaveformFromCSV`.

- [ ] **PLAN_STEP_3_1** (effort: S) — traces to objective 4. Run
  `python HELPER_SCRIPTS/build_tests.py --run` from repo root; confirm all cases pass
  (previous 154 + the 2 new ones), and confirm `git diff` touches only
  `WaveformFileManager.h` and `test_WaveformFileManager.cpp`.

## Open questions
None blocking — scope was narrowed explicitly to additive-only parity work; no renames, no
`WaveFactory`/`CsvLoader` changes, no call-site changes.
