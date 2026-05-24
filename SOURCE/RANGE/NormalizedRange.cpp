/**
 * Created by Ryan Devens 2026-05-23
 */

#include "NormalizedRange.h"

#include <algorithm>

namespace rd_dsp
{
NormalizedRange::NormalizedRange (float min, float center, float max)
{
    mMin = clamp01 (min);
    mCenter = clamp01 (center);
    mMax = clamp01 (max);

    // Enforce ordering: min lowest, max highest.
    if (mMin > mMax)
        std::swap (mMin, mMax);

    mCenter = std::clamp (mCenter, mMin, mMax);
}

float NormalizedRange::clamp01 (float value)
{
    return std::clamp (value, 0.0f, 1.0f);
}

} // namespace rd_dsp
