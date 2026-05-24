/**
 * Created by Ryan Devens 2026-05-23
 */

#pragma once

namespace rd_dsp
{
/**
 * A normalized range defined by three points: min, center, max.
 * All three are clamped to [0.0, 1.0]. After construction the
 * invariant mMin <= mCenter <= mMax always holds.
 */
class NormalizedRange
{
public:
    NormalizedRange (float min, float center, float max);

    float getMin() const { return mMin; }
    float getCenter() const { return mCenter; }
    float getMax() const { return mMax; }

private:
    static float clamp01 (float value);

    float mMin = 0.0f;
    float mCenter = 0.0f;
    float mMax = 0.0f;
};

} // namespace rd_dsp
