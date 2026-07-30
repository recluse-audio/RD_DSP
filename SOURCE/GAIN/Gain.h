#pragma once

#include "../RD_BUFFER/RD_Buffer.h"

namespace rd_dsp
{

class Gain
{
public:
    Gain();
    ~Gain();

    static constexpr float kDefaultRampSeconds = 0.02f;

    void prepare (double sampleRate, int maxBlockSize);

    void  setGain (float linearGain) noexcept;
    float getGain() const noexcept;
    float getCurrentGain() const noexcept;

    void setRampSeconds (float rampSeconds) noexcept;

    void reset() noexcept;

    void process (RD_Buffer& buffer);
    void process (const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples);
    float processSingleSample (float inputSample) noexcept;

private:
    friend class GainTester;

    void _process (const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples);
    void _advance() noexcept;
    void _updateIncrement() noexcept;

    double mSampleRate  = 44100.0;
    int    mBlockSize   = 0;
    float  mRampSeconds = kDefaultRampSeconds;
    float  mTargetGain  = 1.f;
    float  mCurrentGain = 1.f;
    float  mIncrement   = 0.f;
    int    mRampSamplesRemaining = 0;
};

} // namespace rd_dsp
