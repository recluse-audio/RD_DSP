# WaveformFileManager parity — SUMMARY

## What changes
| What | Why | Where |
|------|-----|-------|
| Fix `rapidcsv::Document` header-row bug | Default `LabelParams` treats CSV row 0 as a header; the single-row golden CSV was silently dropped, so every loaded sample read back as `0.f` | `SOURCE/WAVEFORM/WaveformFileManager.h` (`fillFromCSV`) |
| New `loadWaveformFromCSV` static method | Gives `WaveformFileManager` a single-`Waveform` loader matching `WaveFactory::loadWaveformFromCSV` | `SOURCE/WAVEFORM/WaveformFileManager.h` |
| Two new `TEST_CASE`s | Mirror `WaveFactory`'s existing golden-sine and basic-table (4-waveform, wavePos-interpolated) coverage | `TESTS/WAVEFORM/test_WaveformFileManager.cpp` |

## Acronyms
- **CSV** — comma-separated values, the plain-text golden waveform file format used in tests.

## In repo vs. on machine
- No config/secrets/binaries involved — this is pure library source + test code, already
  vendored (`SUBMODULES/rapidcsv`) and already linked (`CMakeLists.txt`). Nothing new to
  install.

## How to use it
- `python HELPER_SCRIPTS/build_tests.py --run` — rebuilds and runs the full Catch2 suite
  (repo root).
- `./BUILD/Tests "[rapidcsv]"` — runs just the `WaveformFileManager` tests after building.
