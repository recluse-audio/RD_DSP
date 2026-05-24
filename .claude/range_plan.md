# Range plan

Goal: one world-space `Range` class (start, end, skew) that maps world values
to/from normalized [0,1], emulating the *core* of JUCE `NormalisableRange`.
It subsumes the old `NormalizedRange` (min/center/max [0,1] triple). Skip JUCE
extras (interval snapping, symmetric skew, conversion lambdas) until needed.

Use case: set a world range for a parameter (e.g. frequency 1..1500) plus a
skew factor, then convert between a knob position [0,1] and the world value.

Status legend: `[ ]` todo  `[~]` in progress  `[x]` done  `[!]` blocked

## Class shape (target)

```
class Range
{
public:
    Range (float start, float end, float skew = 1.0f);

    float getStart() const;
    float getEnd() const;
    float getSkew() const;
    void  setSkew (float skew);
    void  setSkewForCentre (float centreWorldValue); // derive skew so 0.5 -> centre

    float convertTo0to1 (float worldValue) const;   // clamp -> normalize -> skew
    float convertFrom0to1 (float proportion) const;  // unskew -> denormalize

private:
    float mStart, mEnd, mSkew;
};
```

Skew math (JUCE core):
- to0to1:   p = (v - start)/(end - start);  if skew!=1 && p>0: p = pow(p, 1/skew)
- from0to1: if skew!=1: p = pow(p, skew);   v = start + (end - start)*p

## Increments

- [ ] 1. RED+GREEN linear core. New `SOURCE/RANGE/Range.{h,cpp}`. ctor(start,end,
      skew=1), getters, convert*0to1 with skew==1 (linear). Test: endpoints map
      0 and 1, round-trip, midpoint.
- [ ] 2. Skew math. Add pow-based skew to both convert fns. Test: skew!=1
      midpoint maps off-0.5; round-trip still exact.
- [ ] 3. `setSkew` + `setSkewForCentre`. Derive skew = log(0.5)/log((c-start)/
      (end-start)). Test: after setSkewForCentre(c), convertFrom0to1(0.5)==c.
- [ ] 4. Clamp out-of-range world values into [start,end] before normalizing.
      Test: v<start -> 0, v>end -> 1.
- [ ] 5. Randomizer integration. Swap `NormalizedRange mRange` for `Range`.
      DECIDE here: how center/default value is carried (per-call arg vs stored).
      Update `NormalizedRandomizer` + its tests.
- [ ] 6. Retire `NormalizedRange`. Remove old files, rename test_NormalizedRange
      -> test_Range, fix includes. User runs regenSource + build.

## Open decisions

- Increment 5: does `Range` keep an explicit centre/default, or does the
  randomizer pass it per call (current `getNextRandom(min,center,max)` shape)?
  Resolve when we reach it.
- Rename `NormalizedRandomizer` -> `Randomizer`? Defer; not required for skew.
