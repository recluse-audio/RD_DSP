# Emission-rate randomization + RandomizedParam rename plan

Two changes:
1. RandomizedParam API: spelling centre -> center, and replace `getCentre()`/
   `draw()` with `getCenterValue()` and `getRandomizedValue()`. Update callers
   (PulsarData, PulsarTrain) and tests.
2. Emission period: recompute it on *every* emission, running the emission rate
   through a RandomizedParam like the other params. Density 0 / no range = same
   result as today; with randomization the period varies per emission.

Scope note: only RandomizedParam's own "centre" is respelled. `Range::
setSkewForCentre` and `Randomizer`'s `centre` parameter names are left as-is
(not requested).

Status legend: `[ ]` todo  `[~]` in progress  `[x]` done  `[!]` blocked

## Increments

- [x] 1. RandomizedParam rename. setCentre->setCenter, getCentre->getCenterValue,
      draw->getRandomizedValue, mCentre->mCenter, comments. Update PulsarData.cpp,
      PulsarTrain.cpp, test_RandomizedParam, test_PulsarData. Refactor: tests stay
      green (rename only).
- [x] 2. Emission-rate param on PulsarTrain (NOT PulsarData -- emission rate is
      train-level timing, not per-pulsar). Add `RandomizedParam mEmissionRateRandom`
      member, configured in ctor: range kMinEmissionRate..kMaxEmissionRate,
      density 0. Test via PulsarTrainTester accessor.
- [x] 3. Per-emission period. `_emitPulsar` sets mEmissionRateRandom center from
      mEmissionRate atomic, getRandomizedValue(), recomputes mEmissionPeriod every
      emission. Density 0 keeps timing identical (existing E2E/period tests green).
      Add randomization test via Tester (density 1 -> period varies / within bounds).

## Open decisions
- Inc 3: keep flag-based `_updateEmissionPeriod` for start()/prepare() seeding,
  add per-emission recompute in _emitPulsar (don't break flushUpdate tests).
- Control surface (PulsarTrain setters for emission-rate random range/density)
  deferred -- same thread-safety caveat as formant. Tests poke via friend for now.

## Follow-ups (carried from PulsarData task)
- Thread-safe control surface for live random config (Randomizer holds plain
  floats, not atomics).
- Consume wavePosition / amp draws. Pan operations (TODO(pan)).
