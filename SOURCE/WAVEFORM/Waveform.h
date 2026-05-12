/**
 * Created on 2026-05-07 by Ryan Devens w/ peace and love
 */

#pragma once
#include "../RD_BUFFER/RD_Buffer.h"

namespace rd_dsp
{

/**
 * Cyclical, normalized waveform with two zero-crossings.
 *
 * Owns a single-channel RD_Buffer of amplitude values and knows how to fill
 * itself with canonical wave shapes (sine, tri, square, saw).
 *
 * Rules:
 *   1. Cyclical — end amplitude near start amplitude.
 *   2. Normalized — anything above abs(1.0) is treated as clipped.
 *   3. Two zero-crossings.
 */
class Waveform
{
public:
    enum class WaveType
    {
        wSine   = 0,
        wTri    = 1,
        wSquare = 2,
        wSaw    = 3
    };

    Waveform();
    ~Waveform();

    void setSize (int numSamples);

    int   getNumSamples() const noexcept { return mBuffer.getNumSamples(); }
    float getSample (int sampleIndex) const noexcept
    {
        return mBuffer.getSample (0, sampleIndex);
    }

    void setWaveType (WaveType waveType);

    void setSample (int sampleIndex, float value) noexcept
    {
        mBuffer.setSample (0, sampleIndex, value);
    }

    // notice this takes a FLOAT index? That means don't round before here, it will
    // handle interp.
    float getInterpolatedSampleAtIndex(float index);
private:
    friend class WaveformTester;
    void _fillWithSine();
    void _fillWithTri();
    void _fillWithSquare();
    void _fillWithSaw();

    RD_Buffer mBuffer;
};

} // namespace rd_dsp
