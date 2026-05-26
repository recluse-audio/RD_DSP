/**
 * Created by Ryan Devens 2026-05-23
 */

#include "Randomizer.h"

#include <algorithm>

namespace rd_dsp
{
void Randomizer::setRange (float start, float end)
{
    // keep current skew; only the bounds change
    mStart.store (start, std::memory_order_relaxed);
    mEnd.store (end, std::memory_order_relaxed);
}

void Randomizer::setSkew (float skew)
{
    mSkew.store (skew, std::memory_order_relaxed);
}

float Randomizer::getStart() const
{
    return mStart.load (std::memory_order_relaxed);
}

float Randomizer::getEnd() const
{
    return mEnd.load (std::memory_order_relaxed);
}

float Randomizer::getSkew() const
{
    return mSkew.load (std::memory_order_relaxed);
}

void Randomizer::setDensity (float density)
{
    mDensity.store (std::clamp (density, 0.0f, 1.0f), std::memory_order_relaxed);
}

float Randomizer::getDensity() const
{
    return mDensity.load (std::memory_order_relaxed);
}

float Randomizer::getNextRandom (float start, float centre, float end)
{
    setRange (start, end);

    float nextRandomValue = centre; // default unless the density check passes

    // mDensity is the probability a fresh random value replaces the centre.
    if (_getNormalizedRandom() < mDensity.load (std::memory_order_relaxed))
        nextRandomValue = _getRandomValueInRange();

    return nextRandomValue;
}

float Randomizer::getNextNormalizedRandom (float normalizedCentre)
{
    float nextNormalizedValue = normalizedCentre; // default unless the gate passes

    if (_getNormalizedRandom() < mDensity.load (std::memory_order_relaxed))
        nextNormalizedValue = _getNormalizedRandom();

    return nextNormalizedValue;
}

float Randomizer::convertTo0to1 (float worldValue) const
{
    return _makeRange().convertTo0to1 (worldValue);
}

float Randomizer::convertFrom0to1 (float normalizedValue) const
{
    return _makeRange().convertFrom0to1 (normalizedValue);
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
    return _makeRange().convertFrom0to1 (_getNormalizedRandom());
}

Range Randomizer::_makeRange() const
{
    return Range (mStart.load (std::memory_order_relaxed),
                  mEnd.load   (std::memory_order_relaxed),
                  mSkew.load  (std::memory_order_relaxed));
}

} // namespace rd_dsp
