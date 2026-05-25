/**
 * Created by Ryan Devens 2026-05-24
 */

#pragma once

#include "Randomizer.h"

namespace rd_dsp
{
/**
 * A parameter wrapped in randomization: an encapsulated Randomizer (range
 * bounds, skew, density) plus a world-space centre default (added later).
 * draw() will return a density-gated random value around the centre, within
 * the range. The Randomizer is private -- callers go through this interface.
 */
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

    float draw(); // density-gated value: the centre, or a random draw in [start,end]

private:
    Randomizer mRandomizer;
    float mCentre = 0.0f;
};

} // namespace rd_dsp
