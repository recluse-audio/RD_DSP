#include "SynthVoice.h"

#include "../WAVEFORM/Wavetable.h"
#include "../OSCILLATOR/Oscillator.h"

#include <cmath>

namespace rd_dsp
{

SynthVoice::SynthVoice (Wavetable& waveTable)
    : mWavetable (waveTable)
    , mOscillator (std::make_unique<Oscillator>(waveTable))
{
}

SynthVoice::~SynthVoice() = default;
SynthVoice::SynthVoice (SynthVoice&&) noexcept = default;

void SynthVoice::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate; mBlockSize = maxBlockSize;
    mOscillator->prepare(sampleRate, maxBlockSize);
}

void SynthVoice::process (const float* const* readPointers, float* const* writePointers,
                          int numChannels, int numSamples)
{
    if(mIsActive)
    {
        mOscillator->process(readPointers, writePointers, numChannels, numSamples);
    }
}

void SynthVoice::noteOn (int midiNoteNumber, float velocity)
{
    mCurrentMidiNote = midiNoteNumber;
    mIsActive = true;
    mOscillator->setFreq (midiToHertz (midiNoteNumber));
    mOscillator->start();
}

void SynthVoice::noteOff (float velocity)
{
    mCurrentMidiNote = -1;
    mIsActive = false;
    mOscillator->stop();
}

bool SynthVoice::isActive()
{
    return mIsActive;
}

int SynthVoice::getCurrentActiveNote()
{
    return mCurrentMidiNote;
}

float SynthVoice::midiToHertz (int midiNoteNumber) noexcept
{
    return 440.0f * std::pow (2.0f, (static_cast<float> (midiNoteNumber) - 69.0f) / 12.0f);
}

} // namespace rd_dsp
