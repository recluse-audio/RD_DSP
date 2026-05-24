/**
 * Created by Ryan Devens 2026-05-23
 */

#pragma once

#include "../RANGE/NormalizedRange.h"

#include <random>

namespace rd_dsp
{
/**
 * Used to get a random number from a range,
 * with the added twist of mDensity which affects the likelihood
 * that it would produce a random value.
 *
 * So you give it a center value within a range and it *might* come back different
 * 
 *  mDensity refers to the percentage of times a random value should be generated. 
 */
class NormalizedRandomizer
{
public:
    void setNormalizedRange (float min, float center, float max);

    float getMin() const;
    float getCenter() const;
    float getMax() const;

    float getNextRandom (float min, float center, float max);

    void setDensity (float density);
    float getDensity() const;

private:
    NormalizedRange mRange { 0.0f, 0.0f, 1.0f };
    std::minstd_rand mRandomNumberGenerator;
    float mDensity = 1.0f;

    float _getNormalizedRandom();    // raw draw mapped to [0,1]
    float _getRandomValueInRange();  // [0,1] draw scaled to [mMin, mMax]

};

} // namespace rd_dsp
