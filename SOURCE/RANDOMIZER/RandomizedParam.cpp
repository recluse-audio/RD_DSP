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

void RandomizedParam::setCentre (float centre)
{
    mCentre = centre;
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

float RandomizedParam::getCentre() const
{
    return mCentre;
}

float RandomizedParam::draw()
{
    return mRandomizer.getNextRandom (mRandomizer.getStart(), mCentre, mRandomizer.getEnd());
}

} // namespace rd_dsp
