/**
 * Created by Ryan Devens 2026-05-23
 */

#include "NormalizedRandomizer.h"

#include <algorithm>

namespace rd_dsp
{
void NormalizedRandomizer::setNormalizedRange (float min, float center, float max)
{
    mRange = NormalizedRange (min, center, max);
}

float NormalizedRandomizer::getMin() const
{
    return mRange.getMin();
}

float NormalizedRandomizer::getCenter() const
{
    return mRange.getCenter();
}

float NormalizedRandomizer::getMax() const
{
    return mRange.getMax();
}

void NormalizedRandomizer::setDensity (float density)
{
    mDensity = std::clamp (density, 0.0f, 1.0f);
}

float NormalizedRandomizer::getDensity() const
{
    return mDensity;
}

float NormalizedRandomizer::getNextRandom (float min, float center, float max)
{
    setNormalizedRange (min, center, max);

    float nextRandomValue = center; // default unless the density check passes

    // mDensity is the probability a fresh random value replaces the center.
    if (_getNormalizedRandom() < mDensity)
        nextRandomValue = _getRandomValueInRange();

    return nextRandomValue;
}

float NormalizedRandomizer::_getNormalizedRandom()
{
    // minstd_rand output is fixed by the standard: [1, 2147483646].
    // Divisor is a compile-time constant, so fold it into a reciprocal we multiply by.
    static_assert (std::minstd_rand::min() == 1u, "unexpected generator min");
    static_assert (std::minstd_rand::max() == 2147483646u, "unexpected generator max");

    constexpr float kGeneratorMin = 1.0f;
    constexpr float kInvGeneratorRange = 1.0f / (2147483646.0f - 1.0f);

    return (static_cast<float> (mRandomNumberGenerator()) - kGeneratorMin) * kInvGeneratorRange;
}

float NormalizedRandomizer::_getRandomValueInRange()
{
    return mRange.getMin() + _getNormalizedRandom() * (mRange.getMax() - mRange.getMin());
}

} // namespace rd_dsp
