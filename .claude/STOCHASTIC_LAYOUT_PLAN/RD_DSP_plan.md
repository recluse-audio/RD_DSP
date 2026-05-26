# RD_DSP — Stochastic Spread for PulsarTrain (v2 prereq)

**Mode:** `incremental_educational`. One increment per turn. Plan-first. Real RED (runtime assertion failure, not compile failure).

**Scope (this repo only):** `SOURCE/PULSAR/PulsarTrain.{h,cpp}` + `TESTS/PULSAR/test_PulsarTrain.cpp` + `VERSION.txt`. Out of scope: JS, wasm, UI, C-ABI shim, RECLUSE_UI.

**Cross-repo contract (source of truth, do not edit from here):**
`C:\REPOS\PROJECTS\WEB_SYNTH\.claude\STOCHASTIC_LAYOUT_PLAN\WEB_SYNTH_plan.md` → "## v2 — wire stochastic spread".

---

## Problem

Stochastic machinery exists (`Randomizer`, `RandomizedParam`, `Range`, `PulsarData`) and is embedded in `PulsarTrain` (`mEmissionRateRandom`, `mPulsarData`), but is **not reachable or applied**:

- No public `setRange`/`setDensity`/`setSkew` on `PulsarTrain` — only the center setters `setEmissionRate`/`setFormantFreq`.
- Emission range hardcoded `kMin/kMaxEmissionRate`, emission density hardcoded `0.0f` in ctor.
- `_emitPulsar()` consumes only `mPulsarData.resolve().formantFreq` — drawn `wavePosition`/`amp`/`pan` discarded.
- `setWavePosition` writes `mWavetable->setNormalizedWavePosition` directly, bypassing `mPulsarData.wavePosition` (dead).

## Mapping (UI param → RD_DSP target)

| UI param | center setter (exists) | spread target |
|---|---|---|
| emission | `setEmissionRate` → `mEmissionRateRandom` center | `mEmissionRateRandom` range/density |
| formant  | `setFormantFreq` → `mPulsarData.formantFreq` center | `mPulsarData.formantFreq` range/density |
| wavePos  | `setWavePosition` → wavetable **directly** | `mPulsarData.wavePosition` (dead until un-bypassed) |
| gain     | WEB_SYNTH shim `mGain` (post-multiply) | `mPulsarData.amp`? — see open question |

## Open question (gates amp setter)

Does UI "gain" become per-emission `amp` (stochastic, drawn in `_emitPulsar`) or stay a WEB_SYNTH master post-gain (no spread)? If master post-gain → skip `setAmpRange/Density` entirely.

---

## Increments

### [x] 1. Emission spread setters
- `setEmissionRange(min,max)` / `setEmissionDensity(d)` (+ optional `setEmissionSkew`) forwarding to `mEmissionRateRandom`. Machinery already wired (period drawn each emit). Center keeps flowing through `setEmissionRate`.
- RED: setter moves underlying `RandomizedParam` start/end/density.

### [x] 2. Formant spread setters
- `setFormantRange(min,max)` / `setFormantDensity(d)` → `mPulsarData.formantFreq`. Already consumed in `_emitPulsar`.
- RED: density 0 ⇒ every emission formant == center; density 1 + widened range ⇒ draws in `[min,max]` and vary.

### [ ] 3. Un-bypass wavePos + apply drawn wavePosition (decision pending)
- Decide: route wavePos center through `mPulsarData.wavePosition` vs. direct-center + separate spread path.
- Apply `resolve().wavePosition` per emission (table lerp) in `_emitPulsar`. Add `setWavePosRange/Density`.
- RED: density 0 ⇒ table pos unchanged; density 1 ⇒ varies within range.

### [ ] 4. Apply drawn amp (gated on gain decision)
- If amp is in scope: per-pulsar gain in `_emitPulsar` from `resolve().amp`. Add `setAmpRange/Density`. Needs a gain hook (Pulsar::emit has none today).
- RED: density 0 ⇒ amp unchanged; density 1 ⇒ varies.

### [x] 5. Bump `VERSION.txt`. (0.0.5 → 0.0.6)

---

## Hard rules

- Zero third-party deps in `SOURCE/`. No alloc / no exceptions on audio path.
- Atomic param state: `memory_order_relaxed` (control write, audio read; independent values).
- Allman braces. No `README.md` / `NOTES/` edits.
- Density-0 collapse must keep existing behavior byte-identical.
