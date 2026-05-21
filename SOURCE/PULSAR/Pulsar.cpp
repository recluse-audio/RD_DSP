#include "PULSAR/Pulsar.h"

#include "../WAVEFORM/Wavetable.h"
#include "../WINDOW/Window.h"
#include "../OSCILLATOR/Oscillator.h"

namespace rd_dsp
{
Pulsar::Pulsar (Wavetable& wavetable, Window& window)
: mWavetable (wavetable)
, mWindow (window)
, mOscillator (std::make_unique<Oscillator>(wavetable))
{

}

Pulsar::~Pulsar() = default;

Pulsar::Pulsar (Pulsar&& other) noexcept
: mWavetable (other.mWavetable)
, mWindow (other.mWindow)
, mOscillator (std::move (other.mOscillator))
, mWindowPos (other.mWindowPos)
, mWindowIncrement (other.mWindowIncrement)
, mIsActive (other.mIsActive.load (std::memory_order_relaxed))
, mSampleRate (other.mSampleRate)
, mDutyCycleSamples (other.mDutyCycleSamples)
{
}

void Pulsar::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate;
    mOscillator->prepare (sampleRate, maxBlockSize);
}

void Pulsar::process (const float* const* readPointers, float* const* writePointers,
                      int numChannels, int numSamples)
{
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float sample = processSingleSample();
        for (int ch = 0; ch < numChannels; ++ch)
            writePointers[ch][sampleIndex] = sample;
    }
}

float Pulsar::processSingleSample()
{
    if (mDutyCycleSamples <= 0)
        return 0.f;

    const float windowSample = mWindow.getInterpolatedSampleAtIndex (mWindowPos);
    const float oscSample    = mOscillator->processSingleSample();
    const float out          = oscSample * windowSample;

    mWindowPos += mWindowIncrement;
    --mDutyCycleSamples;

    if (mDutyCycleSamples <= 0)
    {
        mIsActive.store (false, std::memory_order_relaxed);
        mOscillator->stop();
    }

    return out;
}

void Pulsar::emit (float formantFreq, int dutyCycleSamples)
{
    mWindowPos = 0.f;
    if(dutyCycleSamples <= 0)
        return; // something messed up, shouldn't be here

    mWindowIncrement = static_cast<float> (mWindow.getNumSamples()) / static_cast<float> (dutyCycleSamples);
    
    mIsActive.store (true, std::memory_order_relaxed);
    mOscillator->setFreq(formantFreq);
    mOscillator->start();

    mDutyCycleSamples = dutyCycleSamples;
}

bool Pulsar::isActive() const noexcept
{
    return mIsActive.load (std::memory_order_relaxed);
}


} // namespace rd_dsp
