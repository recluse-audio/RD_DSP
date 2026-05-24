/**
 * Created by Ryan Devens 2026-05-24
 */

#include "Range.h"

#include <algorithm>
#include <cmath>

namespace rd_dsp
{
Range::Range (float start, float end, float skew)
{
    mStart = start;
    mEnd = end;
    mSkew = skew;
}

void Range::setSkew (float skew)
{
    mSkew = skew;
}

void Range::setSkewForCentre (float centreWorldValue)
{
    float centreNormalized = (centreWorldValue - mStart) / (mEnd - mStart);
    mSkew = std::log (centreNormalized) / std::log (0.5f);
}

float Range::convertTo0to1 (float worldValue) const
{
    float clampedWorld = std::clamp (worldValue, mStart, mEnd);
    float proportion = (clampedWorld - mStart) / (mEnd - mStart);

    if (mSkew != 1.0f && proportion > 0.0f)
        proportion = std::pow (proportion, 1.0f / mSkew);

    return proportion;
}

float Range::convertFrom0to1 (float proportion) const
{
    proportion = std::clamp (proportion, 0.0f, 1.0f);

    if (mSkew != 1.0f && proportion > 0.0f)
        proportion = std::pow (proportion, mSkew);

    return mStart + (mEnd - mStart) * proportion;
}

} // namespace rd_dsp
