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
    mCenter = center;
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
    return mCenter;
}

float RandomizedParam::getRandomizedValue()
{
    return mRandomizer.getNextRandom (mRandomizer.getStart(), mCenter, mRandomizer.getEnd());
}

} // namespace rd_dsp
