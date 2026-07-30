#include "Gain.h"

#include <algorithm>
#include <cmath>

namespace rd_dsp
{

Gain::Gain() = default;

Gain::~Gain() = default;

void Gain::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    mBlockSize  = maxBlockSize;
    reset();
}

void Gain::setGain (float linearGain) noexcept
{
    mTargetGain = std::max (0.f, linearGain);
    _updateIncrement();
}

float Gain::getGain() const noexcept
{
    return mTargetGain;
}

float Gain::getCurrentGain() const noexcept
{
    return mCurrentGain;
}

void Gain::setRampSeconds (float rampSeconds) noexcept
{
    mRampSeconds = std::max (0.f, rampSeconds);
    _updateIncrement();
}

void Gain::reset() noexcept
{
    mCurrentGain = mTargetGain;
    mIncrement   = 0.f;
    mRampSamplesRemaining = 0;
}

void Gain::process (RD_Buffer& buffer)
{
    _process (buffer.getReadArray(), buffer.getWriteArray(), buffer.getNumChannels(), buffer.getNumSamples());
}

void Gain::process (const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples)
{
    _process (readPointers, writePointers, numChannels, numSamples);
}

float Gain::processSingleSample (float inputSample) noexcept
{
    const float scaled = inputSample * mCurrentGain;
    _advance();
    return scaled;
}

void Gain::_process (const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples)
{
    if (readPointers == nullptr || writePointers == nullptr)
        return;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            writePointers[ch][sampleIndex] = readPointers[ch][sampleIndex] * mCurrentGain;
        }
        _advance();
    }
}

void Gain::_advance() noexcept
{
    if (mRampSamplesRemaining <= 0)
        return;

    mCurrentGain += mIncrement;
    --mRampSamplesRemaining;

    if (mRampSamplesRemaining == 0)
    {
        mCurrentGain = mTargetGain;
        mIncrement   = 0.f;
    }
}

void Gain::_updateIncrement() noexcept
{
    const int rampSamples = static_cast<int> (std::round (mRampSeconds * mSampleRate));

    if (rampSamples <= 0 || mCurrentGain == mTargetGain)
    {
        mCurrentGain = mTargetGain;
        mIncrement   = 0.f;
        mRampSamplesRemaining = 0;
        return;
    }

    mRampSamplesRemaining = rampSamples;
    mIncrement = (mTargetGain - mCurrentGain) / static_cast<float> (rampSamples);
}

} // namespace rd_dsp
