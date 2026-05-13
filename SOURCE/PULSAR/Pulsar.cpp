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
Pulsar::Pulsar (Pulsar&&) noexcept = default;

void Pulsar::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate;
    mOscillator->prepare (sampleRate, maxBlockSize);
}

void Pulsar::process (const float* const* readPointers, float* const* writePointers,
                      int numChannels, int numSamples)
{


    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
    {
        if(mDutyCycleSamples <= 0)
            break;

        float windowSample = mWindow.getInterpolatedSampleAtIndex(mWindowPos);
        float oscSample = mOscillator->processSingleSample();

        for(int ch = 0; ch < numChannels; ch++)
        {
            writePointers[ch][sampleIndex] = oscSample * windowSample;    
        }

        mWindowPos = mWindowPos + mWindowIncrement;
        mDutyCycleSamples--;
    }
}

void Pulsar::emit (float formantFreq, int dutyCycleSamples)
{
    mWindowPos = 0.f;
    if(dutyCycleSamples <= 0)
        return; // something messed up, shouldn't be here

    mWindowIncrement = static_cast<float> (mWindow.getNumSamples()) / static_cast<float> (dutyCycleSamples);
    
    mIsActive = true;
    mOscillator->setFreq(formantFreq);
    mOscillator->start();

    mDutyCycleSamples = dutyCycleSamples;
}

} // namespace rd_dsp
