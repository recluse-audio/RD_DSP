/**
 * Created by Ryan Devens on 2026-05-07 with peace & love
 *
 * JUCE-free port of RD/SOURCE/Interpolator.h.
 */

#pragma once

namespace rd_dsp
{

class Interpolator
{
public:
    /**
     * Linear interpolation between val1 and val2.
     * deltaPercent is in [0, 1]: 0 -> val1, 1 -> val2.
     */
    static double linearInterp (double val1, double val2, double deltaPercent)
    {
        const double delta = (val2 - val1) * deltaPercent;
        return val1 + delta;
    }
};

} // namespace rd_dsp
