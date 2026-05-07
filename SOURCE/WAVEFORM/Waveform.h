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
 * Owns an RD_Buffer of amplitude values and knows how to fill itself with
 * canonical wave shapes (sine, tri, square, saw).
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

    void setSize (int numChannels, int numSamples);

    int   getNumChannels() const noexcept { return mBuffer.getNumChannels(); }
    int   getNumSamples()  const noexcept { return mBuffer.getNumSamples(); }
    float getSample (int channel, int sampleIndex) const noexcept
    {
        return mBuffer.getSample (channel, sampleIndex);
    }

    void setWaveType (WaveType waveType);

private:
    friend class WaveformTester;
    void _fillWithSine();
    void _fillWithTri();
    void _fillWithSquare();
    void _fillWithSaw();

    RD_Buffer mBuffer;
};

} // namespace rd_dsp
