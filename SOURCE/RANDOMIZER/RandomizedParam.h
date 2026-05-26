/**
 * Created by Ryan Devens 2026-05-24
 */

#pragma once

#include "Randomizer.h"

namespace rd_dsp
{
/**
 * A parameter wrapped in randomization: an encapsulated Randomizer (range
 * bounds, skew, density) plus a world-space center default.
 * getRandomizedValue() returns a density-gated value around the center, within
 * the range. The Randomizer is private -- callers go through this interface.
 */
class RandomizedParam
{
public:
    void setRange (float start, float end);
    void setSkew (float skew);
    void setDensity (float density);
    void setCenter (float center);

    float getStart() const;
    float getEnd() const;
    float getSkew() const;
    float getDensity() const;
    float getCenterValue() const;

    float getRandomizedValue(); // density-gated: the center, or a random draw in [start,end]

private:
    Randomizer mRandomizer;
    float mCenter = 0.0f;
};

} // namespace rd_dsp
