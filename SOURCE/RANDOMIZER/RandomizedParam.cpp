/**
 * Created by Ryan Devens 2026-05-24
 */

#include "RandomizedParam.h"

namespace rd_dsp
{
void RandomizedParam::setRange (float start, float end)
{
    mRandomizer.setRange (start, end);
}

void RandomizedParam::setSkew (float skew)
{
    mRandomizer.setSkew (skew);
}

void RandomizedParam::setDensity (float density)
{
    mRandomizer.setDensity (density);
}

void RandomizedParam::setCenter (float center)
{
    mCenter.store (center, std::memory_order_relaxed);
}

float RandomizedParam::getStart() const
{
    return mRandomizer.getStart();
}

float RandomizedParam::getEnd() const
{
    return mRandomizer.getEnd();
}

float RandomizedParam::getSkew() const
{
    return mRandomizer.getSkew();
}

float RandomizedParam::getDensity() const
{
    return mRandomizer.getDensity();
}

float RandomizedParam::getCenterValue() const
{
    return mCenter.load (std::memory_order_relaxed);
}

float RandomizedParam::getRandomizedValue()
{
    return mRandomizer.getNextRandom (mRandomizer.getStart(),
                                      mCenter.load (std::memory_order_relaxed),
                                      mRandomizer.getEnd());
}

} // namespace rd_dsp
