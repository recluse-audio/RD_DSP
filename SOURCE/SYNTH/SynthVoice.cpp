#include "SynthVoice.h"

#include "../WAVEFORM/Wavetable.h"
#include "../OSCILLATOR/Oscillator.h"
#include "../RD_BUFFER/RD_Buffer.h"

#include <cmath>

namespace rd_dsp
{

static constexpr int kMaxVoiceChannels = 2;

SynthVoice::SynthVoice (Wavetable& waveTable)
    : mWavetable (waveTable)
    , mOscillator (std::make_unique<Oscillator>(waveTable))
    , mVoiceBuffer (std::make_unique<RD_Buffer>(kMaxVoiceChannels, 512))
{
}

SynthVoice::~SynthVoice() = default;
SynthVoice::SynthVoice (SynthVoice&&) noexcept = default;

void SynthVoice::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate; mBlockSize = maxBlockSize;
    mOscillator->prepare(sampleRate, maxBlockSize);
    mVoiceBuffer->setSize(kMaxVoiceChannels, maxBlockSize);
}

void SynthVoice::process (const float* const* readPointers, float* const* writePointers,
                          int numChannels, int numSamples)
{
    if(!mIsActive)
        return;

    if(numChannels > mVoiceBuffer->getNumChannels())
        numChannels = mVoiceBuffer->getNumChannels();

    if(numSamples > mVoiceBuffer->getNumSamples())
        numSamples = mVoiceBuffer->getNumSamples();

    float* const* voiceChannels = mVoiceBuffer->getWriteArray();
    mOscillator->process(readPointers, voiceChannels, numChannels, numSamples);

    for(int ch = 0; ch < numChannels; ++ch)
    {
        for(int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
        {
            writePointers[ch][sampleIndex] += voiceChannels[ch][sampleIndex];
        }
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
