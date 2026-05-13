#include "PulsarVoice.h"

#include "../OSCILLATOR/Oscillator.h"

#include <algorithm>
#include <cmath>

namespace rd_dsp
{

PulsarVoice::~PulsarVoice() = default;

void PulsarVoice::prepare(double sampleRate, int maxBlockSize)
{
    SynthVoice::prepare(sampleRate, maxBlockSize);
}

void PulsarVoice::noteOn(int midiNoteNumber, float velocity)
{
    const float freq = midiToHertz(midiNoteNumber);
    mCurrentMidiNote = midiNoteNumber;
    _armGrain(freq);
}

void PulsarVoice::noteOff(float /*velocity*/)
{
    // Pulsar grains auto-stop after N cycles; noteOff is a no-op.
}

void PulsarVoice::triggerGrain(float frequencyHz)
{
    mCurrentMidiNote = -1;
    _armGrain(frequencyHz);
}

void PulsarVoice::setGrainCycles(int numCycles) noexcept
{
    mGrainCycles = std::max(0, numCycles);
}

void PulsarVoice::_armGrain(float freq)
{
    mGrainFrequency = freq;
    mIsActive = true;
    mOscillator->setFreq(freq);
    mOscillator->start();

    if (freq > 0.f && mGrainCycles > 0)
    {
        const float samplesPerCycle = static_cast<float>(mSampleRate) / freq;
        mSamplesRemaining = static_cast<int>(std::ceil(samplesPerCycle * static_cast<float>(mGrainCycles)));
    }
    else
    {
        mSamplesRemaining = 0;
        mIsActive = false;
        mOscillator->stop();
    }
}

void PulsarVoice::process(const float* const* readPointers, float* const* writePointers,
                          int numChannels, int numSamples)
{
    if (!mIsActive || mSamplesRemaining <= 0)
        return;

    const int samplesThisBlock = std::min(numSamples, mSamplesRemaining);

    SynthVoice::process(readPointers, writePointers, numChannels, samplesThisBlock);

    mSamplesRemaining -= samplesThisBlock;

    if (mSamplesRemaining <= 0)
    {
        mSamplesRemaining = 0;
        mIsActive = false;
        mOscillator->stop();
    }
}

} // namespace rd_dsp
