/**
 * Created by Ryan Devens 2026-05-24
 */

#pragma once

namespace rd_dsp
{
/**
 * Copied basics of this from JUCE.
 * 
 * World-value range mapped to/from normalized [0,1], with a skew factor.
 * Set a world span (e.g. frequency 1..1500) and convert between a [0,1]
 * knob position and the world value. skew == 1 is linear.
 */
class Range
{
public:
    Range (float start, float end, float skew = 1.0f);

    float getStart() const { return mStart; }
    float getEnd() const { return mEnd; }
    float getSkew() const { return mSkew; }

    void setSkew (float skew);
    void setSkewForCentre (float centreWorldValue); // solve skew so 0.5 -> centre

    float convertTo0to1 (float worldValue) const;
    float convertFrom0to1 (float proportion) const;

private:
    float mStart = 0.0f;
    float mEnd = 1.0f;
    float mSkew = 1.0f;
};

} // namespace rd_dsp
