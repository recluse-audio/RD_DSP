#include "Oscillator.h"

namespace rd_dsp
{

Oscillator::Oscillator (Wavetable& wavetable)
    : mWavetable (wavetable)
{
}

Oscillator::~Oscillator() = default;

void Oscillator::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate;
    mBlockSize  = maxBlockSize;
    _updatePhaseIncrement();
}

void Oscillator::setFreq (float freq)
{
    mFrequency = freq;
    mPhaseIncrementUpdateNeeded = true;
}

void Oscillator::process(RD_Buffer& buffer)
{
    _process (buffer.getReadArray(), buffer.getWriteArray(), buffer.getNumChannels(), buffer.getNumSamples());
}

void Oscillator::process(const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples)
{
    _process (readPointers, writePointers, numChannels, numSamples);
}

void Oscillator::_process(const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples)
{
    if(!mIsRunning)
        return;

    if (mPhaseIncrementUpdateNeeded)
    {
        _updatePhaseIncrement();
        mPhaseIncrementUpdateNeeded = false;
    }

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        float waveformSample = mWavetable.getSampleAtIndex (mCurrentIndex);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            writePointers[ch][sampleIndex] = waveformSample;
        }
        _incrementCurrentIndex();
    }
}

float Oscillator::processSingleSample()
{
    if (!mIsRunning)
        return 0.f;

    if (mPhaseIncrementUpdateNeeded)
    {
        _updatePhaseIncrement();
        mPhaseIncrementUpdateNeeded = false;
    }

    float sample = mWavetable.getSampleAtIndex (mCurrentIndex);
    _incrementCurrentIndex();
    return sample;
}

void Oscillator::start()
{
    mIsRunning = true;
}

void Oscillator::stop()
{
    mIsRunning = false;
}

void Oscillator::_incrementCurrentIndex()
{
    const int size = mWavetable.getWaveformSize();
    mCurrentIndex = mCurrentIndex + mPhaseIncrement;
    if (size > 0 && mCurrentIndex >= (float) size)
        mCurrentIndex = mCurrentIndex - (float) size;
}

void Oscillator::_updatePhaseIncrement()
{
    if(mSampleRate == 0)
        return; // don't divide by zero

    mPhaseIncrement = _calculatePhaseIncrement (mFrequency, mSampleRate, mWavetable.getWaveformSize());
}

float Oscillator::_calculatePhaseIncrement (float freq, double sampleRate, int waveformSize) noexcept
{
    if (sampleRate == 0.0)
        return 0.0f;

    return static_cast<float> (static_cast<double> (freq) * static_cast<double> (waveformSize) / sampleRate);
}

} // namespace rd_dsp
