# RD_DSP — Pulsar Activity Flash Plan

**Mode:** `incremental_educational`. One increment per turn. Plan-first.

**Repo:** `RD_DSP` (executed in its own clone — not from WEB_SYNTH).

**On first turn:** create `.claude/PULSAR_ACTIVITY_FLASH_PLAN/` in this repo and copy this file there as `RD_DSP_plan.md`. Update checkboxes in that copy as increments land.

## Goal

Expose `Pulsar`'s per-emission active state as a thread-safe, allocation-free getter usable from non-audio threads (and through a C-ABI shim in WEB_SYNTH). No behavior change to audio processing — the flag is already written by `Pulsar::emit()` / `Pulsar::processSingleSample()`. Only the storage type and a read-only accessor change.

## API target

In `SOURCE/PULSAR/Pulsar.h`:

```cpp
bool isActive() const noexcept;
```

In `SOURCE/PULSAR/PulsarTrain.h`:

```cpp
bool isActive() const noexcept;  // pass-through to mPulsar->isActive()
```

Internal storage change in `Pulsar.h`:

```cpp
std::atomic<bool> mIsActive { false };  // was: bool mIsActive = false;
```

`std::atomic<bool>` is already in use elsewhere in `PulsarTrain.h:69-74`. Same precedent applies.

## Increments

### [x] 1. Failing Catch2 test
- **FILES CHANGING:** `TESTS/PULSAR/test_Pulsar_active.cpp` (new file). May need `TESTS/PULSAR/` dir created.
- **WHY:** Lock the contract before changing storage. TDD-default per repo rules.
- Test cases (tag `[Pulsar]` or `[PulsarActivity]`):
  - Freshly constructed `Pulsar` → `isActive() == false`.
  - After `emit(formantFreq=440, dutyCycleSamples=10)` → `isActive() == true`.
  - After calling `processSingleSample()` past the duty cycle window → `isActive() == false`.
- Regen sources: `python HELPER_SCRIPTS/regenSource.py` (auto via build script).
- **Confirm RED** — `Pulsar::isActive()` doesn't exist yet, so add the minimum declaration + body returning `false` to make the test compile and fail at the assertion (per `incremental_educational` rule: RED is a real assertion failure, not a compile failure).

### [x] 2. Promote `mIsActive` to `std::atomic<bool>`
- **FILES CHANGING:** `SOURCE/PULSAR/Pulsar.h`, `SOURCE/PULSAR/Pulsar.cpp`.
- **WHY:** Allow lock-free reads from a non-audio thread. Audio thread continues to write — `.store(true, std::memory_order_relaxed)` is a single instruction, same cost as a plain bool write on x86/ARM.
- **CONVENTION NOTES:** `std::atomic<bool>` with `memory_order_relaxed` for writes and reads is the standard "I just need atomicity, no ordering" pattern. Safe here because the flag is independent of any other state — readers don't need to see other side effects synchronized with it.
- Replace all `mIsActive = X;` assignments with `mIsActive.store(X, std::memory_order_relaxed);`.
- Replace all `if (mIsActive)` reads with `if (mIsActive.load(std::memory_order_relaxed))`.
- Constructor init list: `mIsActive{ false }` (atomic does not aggregate-init).
- Note: `Pulsar` already declares move ctor; `std::atomic` is not movable. May need to drop `noexcept` move ctor or initialize the atomic explicitly in the move body. Confirm `Pulsar` move ctor is actually used; if only the moved-from default state matters, an init-list `mIsActive{ other.mIsActive.load() }` works.

### [x] 3. Implement `Pulsar::isActive()`
- **FILES CHANGING:** `SOURCE/PULSAR/Pulsar.h` (decl already there from increment 1), `SOURCE/PULSAR/Pulsar.cpp` (or inline in header).
- **WHY:** Public, const, noexcept read accessor.
- Body: `return mIsActive.load(std::memory_order_relaxed);`
- Tests from increment 1 should go GREEN.

### [x] 4. Add `PulsarTrain::isActive()` pass-through
- **FILES CHANGING:** `SOURCE/PULSAR/PulsarTrain.h`, optionally `.cpp`.
- **WHY:** WEB_SYNTH's C-ABI shim only holds a `PulsarTrain` (the public face); it does not reach into the owned `Pulsar`. Avoid leaking `Pulsar` to consumers.
- Body: `return mPulsar ? mPulsar->isActive() : false;`
- `const noexcept`. Same memory-order story; the atomic is in `Pulsar`.
- Catch2: extend or add a `PulsarTrain` test that calls `prepare`, `start`, advances some samples, asserts `isActive()` toggles across emission cycles.

### [ ] 5. Tag a version bump
- **FILES CHANGING:** `VERSION.txt`.
- **WHY:** WEB_SYNTH bumps the submodule pointer to a tagged commit; version bump makes the contract explicit.
- Bump patch version. Commit. Tag. Push.

## Out of scope

- No JS, no wasm, no UI.
- No emission-rate getter changes — `getEmissionRate()` already exists at `PulsarTrain.h:52`.
- No edge-detection logic in DSP. The flag is the level; consumers detect transitions.

## Hard rules reminder

- Zero third-party deps in `SOURCE/`.
- No allocation, no exceptions on audio path.
- Allman braces for new code.
- Atomic operations stay `relaxed` unless we discover an ordering requirement (we won't — flag is independent state).
