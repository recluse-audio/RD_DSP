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
    _updatePhaseIncrement();
}

void Oscillator::process(RD_Buffer& buffer)
{
    if(!mIsRunning)
        return;

    int numSamples  = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            
        }
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

void Oscillator::_updatePhaseIncrement()
{
    if(mSampleRate == 0)
        return; // don't divide by zero

    mPhaseIncrement = static_cast<float> (static_cast<double> (mFrequency) * mWaveform->getNumSamples() / mSampleRate);
}

} // namespace rd_dsp
