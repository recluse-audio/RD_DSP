/**
 * Created by Ryan Devens 2026-05-23
 */

#include "NormalizedRandomizer.h"

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

float NormalizedRandomizer::getNextRandom (float min, float center, float max)
{
    setNormalizedRange (min, center, max);
    return getNextRandom();
}

float NormalizedRandomizer::getNextRandom()
{
    // minstd_rand output is fixed by the standard: [1, 2147483646].
    // Divisor is a compile-time constant, so fold it into a reciprocal we multiply by.
    static_assert (std::minstd_rand::min() == 1u, "unexpected generator min");
    static_assert (std::minstd_rand::max() == 2147483646u, "unexpected generator max");

    constexpr float kGeneratorMin = 1.0f;
    constexpr float kInvGeneratorRange = 1.0f / (2147483646.0f - 1.0f);

    const float normalized = (static_cast<float> (mRandomNumberGenerator()) - kGeneratorMin) * kInvGeneratorRange;

    return mRange.getMin() + normalized * (mRange.getMax() - mRange.getMin());
}

} // namespace rd_dsp
