#include "SynthVoice.h"

#include "../WAVEFORM/Wavetable.h"
#include "../OSCILLATOR/Oscillator.h"

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

}

void SynthVoice::noteOn (int midiNoteNumber, float velocity)
{
    mCurrentMidiNote = midiNoteNumber;
    mIsActive = true;
}

void SynthVoice::noteOff (float velocity)
{
    mCurrentMidiNote = -1;
    mIsActive = false;
}

bool SynthVoice::isActive()
{
    return mIsActive;
}

int SynthVoice::getCurrentActiveNote()
{
    return mCurrentMidiNote;
}

} // namespace rd_dsp
