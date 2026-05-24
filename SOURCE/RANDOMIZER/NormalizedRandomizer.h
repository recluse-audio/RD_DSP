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
 */
class NormalizedRandomizer
{
public:
    void setNormalizedRange (float min, float center, float max);

    float getMin() const;
    float getCenter() const;
    float getMax() const;

    float getNextRandom();
    float getNextRandom (float min, float center, float max);

private:
    NormalizedRange mRange { 0.0f, 0.0f, 1.0f };
    std::minstd_rand mRandomNumberGenerator;
};

} // namespace rd_dsp
