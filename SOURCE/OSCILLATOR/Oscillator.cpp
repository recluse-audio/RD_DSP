#include "Oscillator.h"

namespace rd_dsp
{

namespace
{
    constexpr int kDefaultWaveformSize = 8096;
}

Oscillator::Oscillator()
    : mWaveform (std::make_unique<Waveform>())
{
    mWaveform->setSize (kDefaultWaveformSize);
    mWaveform->setWaveType (Waveform::WaveType::wSine);
}

Oscillator::~Oscillator() = default;

void Oscillator::prepare (double sampleRate)
{
    mSampleRate = sampleRate;
    _updatePhaseIncrement();
}

void Oscillator::setFreq (float freq)
{
    mFrequency = freq;
    mPhaseIncrementUpdateNeeded = true;
}

void Oscillator::process(RD_Buffer& buffer)
{
    if(!mIsRunning)
        return;

    if (mPhaseIncrementUpdateNeeded)
    {
        _updatePhaseIncrement();
        mPhaseIncrementUpdateNeeded = false;
    }

    int numSamples  = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        float waveformSample = mWaveform->getSample(mCurrentIndex);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.setSample(ch, sampleIndex, waveformSample);
        }
        _incrementCurrentIndex();
    }
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
    mCurrentIndex = mCurrentIndex + mPhaseIncrement;
    if(mCurrentIndex >= mWaveform->getNumSamples())
        mCurrentIndex = mCurrentIndex - mWaveform->getNumSamples();
}

void Oscillator::_updatePhaseIncrement()
{
    if(mSampleRate == 0)
        return; // don't divide by zero

    mPhaseIncrement = _calculatePhaseIncrement (mFrequency, mSampleRate, mWaveform->getNumSamples());
}

float Oscillator::_calculatePhaseIncrement (float freq, double sampleRate, int waveformSize) noexcept
{
    if (sampleRate == 0.0)
        return 0.0f;

    return static_cast<float> (static_cast<double> (freq) * static_cast<double> (waveformSize) / sampleRate);
}

} // namespace rd_dsp
