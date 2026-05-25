# PulsarData plan

Goal: a `PulsarData` struct holding per-param randomization config. Each param
pairs a `Randomizer` (range bounds + skew + density) with a stored world-space
`centre`. On pulsar emission every param is *potentially* randomized within its
range; density decides the likelihood. Existing setters write the centre.

Params: formant freq, wave position, amp, pan. **Pan is a placeholder this
task — no operations wired; TODO afterwards.**

Building block: `Randomizer` (already done). `getNextRandom(start, centre, end)`
gates on density: returns centre, or a skewed random draw in [start, end].

Status legend: `[ ]` todo  `[~]` in progress  `[x]` done  `[!]` blocked

## Shapes (target, names provisional)

`RandomizedParam` is its own class/file with the `Randomizer` **private** behind
a clean interface (not a public aggregate). It lives in `SOURCE/RANDOMIZER/`
next to `Randomizer` since it is a generic randomization primitive.

```
// SOURCE/RANDOMIZER/RandomizedParam.{h,cpp}
class RandomizedParam
{
public:
    void setRange (float start, float end);
    void setSkew (float skew);
    void setDensity (float density);
    void setCentre (float centre);

    float getStart() const;
    float getEnd() const;
    float getSkew() const;
    float getDensity() const;
    float getCentre() const;

    float draw(); // density-gated random value around centre, within range

private:
    Randomizer mRandomizer; // encapsulated; not exposed directly
    float mCentre = 0.0f;
};

// SOURCE/PULSAR/PulsarData.{h,cpp}
struct PulsarData
{
    RandomizedParam formantFreq;
    RandomizedParam wavePosition;
    RandomizedParam amp;
    RandomizedParam pan;     // placeholder, not wired yet
};
```

Default bounds / density (density 0 = never randomize, safe default):
- formantFreq: 150 .. 2000 Hz
- wavePosition: 0 .. 1
- amp: 0 .. 1
- pan: -1 .. 1 (inert)
- every param density default 0; user dials up to randomize.

Resolved snapshot (one emission's values) is a *separate* small POD, name TBD
(e.g. `PulsarParamValues`), decided in increment 4.

## Increments

- [x] 1. RandomizedParam config interface. New
      `SOURCE/RANDOMIZER/RandomizedParam.{h,cpp}`: private mRandomizer, delegating
      setRange/setSkew/setDensity + getStart/getEnd/getSkew/getDensity. regenSource.
      Test (RED->GREEN): set then get round-trips through the hidden randomizer.
- [x] 2. Centre. setCentre/getCentre. DECIDED: no clamp, trust caller (draw
      clamps anyway).
- [x] 3. draw(). density-gated draw around centre via getNextRandom.
- [x] 4. PulsarData struct holding 4 RandomizedParam. regenSource.
- [x] 5. Default config per param (formant 150..2000, wavePos/amp 0..1,
      pan -1..1, density 0) via ctor.
- [x] 6. resolve() -> PulsarParamValues POD (formantFreq/wavePosition/amp/pan).
- [x] 7. Pan stays inert. TODO(pan) markers in PulsarData.h. (doc only)
- [x] 8. Wired formant freq through PulsarData in _emitPulsar (centre from the
      mFormantFreq atomic -> resolve -> drawn freq). Density 0 default keeps
      output identical; existing E2E/golden tests are the safety net.

## Follow-ups (not done)
- Control surface on PulsarTrain to set per-param random range/skew/density
  from the message thread. NEEDS thread-safety design: Randomizer/Range hold
  plain floats, not atomics -> racing the audio-thread resolve(). Options:
  double-buffer/snapshot the config, atomic param block, or set-before-start only.
- Consume wavePosition / amp draws (wavePosition -> wavetable per emit; amp ->
  gain). Currently resolved but unused.
- Pan operations (see TODO(pan)).

## Open decisions

- Inc 3: clamp centre into bounds, or trust caller?
- Inc 4: snapshot POD name + which fields Pulsar/PulsarTrain actually consume.
- Header-only vs .cpp for the trivial methods — decide inc 1.
- Density per-param vs one shared density knob? Plan assumes per-param.

## TODO after this task
- Pan operations (panning math + wiring).
