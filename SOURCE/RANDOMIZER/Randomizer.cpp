/**
 * Created by Ryan Devens 2026-05-23
 */

#include "Randomizer.h"

#include <algorithm>

namespace rd_dsp
{
void Randomizer::setRange (float start, float end)
{
    mRange = Range (start, end, mRange.getSkew());
}

void Randomizer::setSkew (float skew)
{
    mRange.setSkew (skew);
}

float Randomizer::getStart() const
{
    return mRange.getStart();
}

float Randomizer::getEnd() const
{
    return mRange.getEnd();
}

float Randomizer::getSkew() const
{
    return mRange.getSkew();
}

void Randomizer::setDensity (float density)
{
    mDensity = std::clamp (density, 0.0f, 1.0f);
}

float Randomizer::getDensity() const
{
    return mDensity;
}

float Randomizer::getNextRandom (float start, float centre, float end)
{
    setRange (start, end);

    float nextRandomValue = centre; // default unless the density check passes

    // mDensity is the probability a fresh random value replaces the centre.
    if (_getNormalizedRandom() < mDensity)
        nextRandomValue = _getRandomValueInRange();

    return nextRandomValue;
}

float Randomizer::getNextNormalizedRandom (float normalizedCentre)
{
    float nextNormalizedValue = normalizedCentre; // default unless the gate passes

    if (_getNormalizedRandom() < mDensity)
        nextNormalizedValue = _getNormalizedRandom();

    return nextNormalizedValue;
}

float Randomizer::convertTo0to1 (float worldValue) const
{
    return mRange.convertTo0to1 (worldValue);
}

float Randomizer::convertFrom0to1 (float normalizedValue) const
{
    return mRange.convertFrom0to1 (normalizedValue);
}

float Randomizer::_getNormalizedRandom()
{
    // minstd_rand output is fixed by the standard: [1, 2147483646].
    // Divisor is a compile-time constant, so fold it into a reciprocal we multiply by.
    static_assert (std::minstd_rand::min() == 1u, "unexpected generator min");
    static_assert (std::minstd_rand::max() == 2147483646u, "unexpected generator max");

    constexpr float kGeneratorMin = 1.0f;
    constexpr float kInvGeneratorRange = 1.0f / (2147483646.0f - 1.0f);

    return (static_cast<float> (mRandomNumberGenerator()) - kGeneratorMin) * kInvGeneratorRange;
}

float Randomizer::_getRandomValueInRange()
{
    return mRange.convertFrom0to1 (_getNormalizedRandom());
}

} // namespace rd_dsp
