# RD_DSP — Pulsar Activity Flash Plan

**Mode:** `incremental_educational`. One increment per turn. Plan-first.

**Repo:** `RD_DSP` (executed in its own clone — not from WEB_SYNTH).

**On first turn:** create `.claude/PULSAR_ACTIVITY_FLASH_PLAN/` in this repo and copy this file there as `RD_DSP_plan.md`. Update checkboxes in that copy as increments land.

**STATUS (2026-05-21): REWORK IN PROGRESS.** Increments 1-5 (the `isActive()` accessor) landed at `3aa18c5`, `VERSION.txt` 0.0.4. But manual test showed `isActive()` is unusable for the flash — see REWORK below. `isActive()` stays (harmless), but the flash now uses a per-emission report-once latch (increments 6-8).

## REWORK (2026-05-21) — report-once latch

**Why `isActive()` failed:** It reflects the duty-cycle window (`mIsActive` true for exactly one formant period per emission ≈ 2.3 ms at 440 Hz). That window is shorter than any GUI poll interval, so polling a *level* aliases — random catches, non-synchronous flicker. A min-flash hold on the JS side papered over it but still depends on catching the brief window.

**Fix:** Track emission as a discrete *event*, latched until the consumer draws it once. Lives in `PulsarTrain` because `_emitPulsar()` is the only place that sees a new pulsar fire.

```cpp
// PulsarTrain.h — new member
std::atomic<bool> mPulsarReportedToGUI { true };  // true = current pulsar already drawn

// PulsarTrain.h/.cpp — new public accessor
bool consumePulsarFlash() noexcept
{
    // Returns true exactly once per emitted pulsar, then false until next emit.
    // exchange = atomic read-modify-write: store true, return previous value.
    return ! mPulsarReportedToGUI.exchange (true, std::memory_order_relaxed);
}

// PulsarTrain.cpp _emitPulsar() — mark the fresh pulsar undrawn
mPulsarReportedToGUI.store (false, std::memory_order_relaxed);
```

Guarantees one flash per pulsar regardless of poll timing or duty cycle. No emission missed — flag persists from emit until consumed.

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

**LANDED:** `SOURCE/PULSAR/PulsarTrain.h:62` (decl), `SOURCE/PULSAR/PulsarTrain.cpp:162` (out-of-line body, `return mPulsar ? mPulsar->isActive() : false;`). `Pulsar::isActive()` at `Pulsar.cpp:84`.

- **FILES CHANGING:** `SOURCE/PULSAR/PulsarTrain.h`, optionally `.cpp`.
- **WHY:** WEB_SYNTH's C-ABI shim only holds a `PulsarTrain` (the public face); it does not reach into the owned `Pulsar`. Avoid leaking `Pulsar` to consumers.
- Body: `return mPulsar ? mPulsar->isActive() : false;`
- `const noexcept`. Same memory-order story; the atomic is in `Pulsar`.
- Catch2: extend or add a `PulsarTrain` test that calls `prepare`, `start`, advances some samples, asserts `isActive()` toggles across emission cycles.

### [x] 5. Version bump
- **FILES CHANGING:** `VERSION.txt` → 0.0.4.
- **WHY:** WEB_SYNTH bumps the submodule pointer; version bump makes the contract explicit.
- **DONE:** `VERSION.txt` at 0.0.4. No git tag cut — repo tracks version via `VERSION.txt`, not annotated tags. WEB_SYNTH pins commit `3aa18c5` directly.

### [x] 6. Failing Catch2 test for report-once latch
- **FILES CHANGING:** `TESTS/PULSAR/test_PulsarTrain_flash.cpp` (new) or extend existing PulsarTrain test.
- **WHY:** Lock the consume contract before adding state.
- Test cases (tag `[PulsarTrain]` or `[PulsarFlash]`):
  - Freshly constructed / before any emission → `consumePulsarFlash() == false`.
  - `prepare`, `start`, advance `process()` until the first emission fires → next `consumePulsarFlash() == true`.
  - Immediately call again without advancing → `consumePulsarFlash() == false` (consumed once).
  - Advance past one more emission period → `consumePulsarFlash() == true` again.
- Add the minimum decl/body (`return false;`) so it compiles and fails at the assertion (real RED, not compile-fail).

### [~] 7. Implement the latch
- **FILES CHANGING:** `SOURCE/PULSAR/PulsarTrain.h`, `SOURCE/PULSAR/PulsarTrain.cpp`.
- **WHY:** The actual report-once mechanism.
- Add member `std::atomic<bool> mPulsarReportedToGUI { true };`.
- In `_emitPulsar()`: `mPulsarReportedToGUI.store (false, std::memory_order_relaxed);` (after `mPulsar->emit(...)` is fine — order vs. emit doesn't matter, same thread).
- Add `bool consumePulsarFlash() noexcept { return ! mPulsarReportedToGUI.exchange (true, std::memory_order_relaxed); }`.
- **CONVENTION NOTES:** `exchange` is the lock-free single-consumer "take and clear" idiom. Producer (audio thread `_emitPulsar`) and consumer (also audio thread, via WEB_SYNTH worklet `process()`) are the same thread in practice, but the atomic keeps it correct even if a future consumer reads from another thread.
- Tests from increment 6 go GREEN.

### [ ] 8. Version bump
- **FILES CHANGING:** `VERSION.txt` → 0.0.5.
- **WHY:** New public API surface (`consumePulsarFlash`). WEB_SYNTH pins the new commit.

## Out of scope

- No JS, no wasm, no UI.
- No emission-rate getter changes — `getEmissionRate()` already exists at `PulsarTrain.h:52`.
- No edge-detection logic in DSP. The flag is the level; consumers detect transitions.

## Hard rules reminder

- Zero third-party deps in `SOURCE/`.
- No allocation, no exceptions on audio path.
- Allman braces for new code.
- Atomic operations stay `relaxed` unless we discover an ordering requirement (we won't — flag is independent state).
