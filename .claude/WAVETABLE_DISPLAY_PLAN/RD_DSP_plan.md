# RD_DSP — Wavetable Display Plan

**Mode:** `incremental_educational`. One increment per turn. Plan-first.

**Repo:** `RD_DSP` (executed in its own clone — not from WEB_SYNTH).

**On first turn:** create `.claude/WAVETABLE_DISPLAY_PLAN/` in this repo and copy this file there as `RD_DSP_plan.md`. Update checkboxes in that copy as increments land.

## Goal

Add a public, allocation-free method on `rd_dsp::Wavetable` that fills a caller-owned float buffer with the **currently interpolated** waveform shape. Decimated to any size the caller asks for. Audio-thread safe (same rules as the rest of the lib). No new third-party deps. Catch2 coverage.

## API target

In `SOURCE/WAVEFORM/Wavetable.h`:

```cpp
// Fills out[0..outSize) with the currently active interpolated waveform,
// stretched/decimated to outSize. Allocation-free, no exceptions.
// Caller owns the buffer. Returns immediately (writes nothing) if not ready
// or outSize <= 0.
void fillDisplayBuffer(float* out, int outSize) const noexcept;
```

Implementation: `for (i in 0..outSize) out[i] = getSampleAtIndex(i * (waveformSize / outSize));`

Notes:
- `getSampleAtIndex` already handles the interp between adjacent wavetable slots via `mNormalizedWavePos`. No duplication.
- Must be `const noexcept`. May require making `getSampleAtIndex` `const` — increment 2 handles that.
- Bound-check `mIsLoading` and `getNumWaveforms() > 0`; return early without touching `out` (caller pre-zeros or accepts stale).

## Increments

### [x] 1. Failing Catch2 test
- **FILES CHANGING:** `TESTS/WAVEFORM/test_Wavetable.cpp` (new file).
- **WHY:** Lock the contract before implementation. TDD-default per repo rules.
- Test cases (tag `[Wavetable]`):
  - Empty wavetable → `fillDisplayBuffer` no-op (buffer untouched / sentinel preserved).
  - Single sine waveform, outSize == waveformSize → output ≈ sine samples (tolerance ~1e-5).
  - Single sine, outSize == waveformSize / 16 → output length 128 for N=2048; first sample == sine[0], shape monotonic regions match.
  - Two waveforms (sine, saw), `setNormalizedWavePosition(0.0)` → ≈ sine; `(1.0)` → ≈ saw; `(0.5)` → midpoint mix at each index.
- Regenerate sources: `python HELPER_SCRIPTS/regenSource.py` (auto by build script).
- **Confirm RED.**

### [x] 2. Make `getSampleAtIndex` `const`
- **FILES CHANGING:** `SOURCE/WAVEFORM/Wavetable.h`, `Wavetable.cpp`.
- **WHY:** `fillDisplayBuffer` is `const`. Read-only sampling has no reason to mutate; if it currently does, that is a smell to fix here.
- If body uses any mutable cached state, mark `mutable` (justify in commit msg) or refactor.
- Build green, all existing tests still pass.

### [x] 3. Implement `fillDisplayBuffer`
- **FILES CHANGING:** `SOURCE/WAVEFORM/Wavetable.h` (decl), `SOURCE/WAVEFORM/Wavetable.cpp` (impl).
- **WHY:** Smallest possible decimation loop. No allocation. No exceptions.
- Guard: `if (mIsLoading || mWaveforms.empty() || outSize <= 0) return;`
- `const float waveSize = static_cast<float>(getWaveformSize());`
- `const float step = waveSize / static_cast<float>(outSize);`
- Loop: `out[i] = getSampleAtIndex(i * step);`
- **CONVENTION NOTES:** Classic nearest-friendly linear decimation. Source-of-truth interpolation already lives in `getSampleAtIndex` (and the per-`Waveform` `getInterpolatedSampleAtIndex`); we resample the *result*, not the underlying tables.
- Tests from increment 1 should go GREEN.

### [ ] 4. (Optional) Add box-average overload for visual smoothing
- **FILES CHANGING:** `Wavetable.h/.cpp`.
- **WHY:** When `outSize << waveformSize`, raw point-sampling can alias visibly on square/saw. Box-average over each output bin gives a cleaner curve without a real lowpass.
- API: `void fillDisplayBufferAveraged(float* out, int outSize) const noexcept;`
- Skip this increment if the naive version looks fine in WEB_SYNTH integration. **Decision deferred to after WEB_SYNTH wiring lands.**

### [ ] 5. Tag a version bump
- **FILES CHANGING:** `VERSION.txt`.
- **WHY:** WEB_SYNTH bumps the submodule pointer to a tagged commit; version bump makes the contract explicit.
- Bump patch version. Commit. Tag.

## Out of scope

- No JS, no wasm, no UI work. This repo stays a pure C++ DSP lib.
- No global state, no caching, no threading primitives.
- No file I/O.

## Hard rules reminder

- Zero third-party deps in `SOURCE/`.
- No allocation in `fillDisplayBuffer`. Stack locals only.
- No exceptions on this path.
- Allman braces for any new code.
- All headers under `SOURCE/` keep zero JUCE / Boost / `<filesystem>` / `<thread>`.
