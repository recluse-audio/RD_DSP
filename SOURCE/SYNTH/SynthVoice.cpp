#include "SynthVoice.h"

#include "../WAVEFORM/Wavetable.h"
#include "../OSCILLATOR/Oscillator.h"

namespace rd_dsp
{

SynthVoice::SynthVoice (Wavetable& waveTable)
    : mWavetable (waveTable)
{
}

SynthVoice::~SynthVoice() = default;
SynthVoice::SynthVoice (SynthVoice&&) noexcept = default;

void SynthVoice::prepare (double sampleRate, int maxBlockSize)
{
}

void SynthVoice::process (const float* const* readPointers, float* const* writePointers,
                          int numChannels, int numSamples)
{
}

void SynthVoice::noteOn (int midiNoteNumber, float velocity)
{
}

void SynthVoice::noteOff (float velocity)
{
}

} // namespace rd_dsp
