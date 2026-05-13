#include "Synth.h"

#include "../WAVEFORM/Wavetable.h"

#include <cassert>

namespace rd_dsp
{

Synth::Synth()
    : mWavetable (std::make_unique<Wavetable>())
{
    this->setNumVoices(1);
}

Synth::~Synth() = default;

void Synth::prepare (double sampleRate, int maxBlockSize)
{
}

void Synth::process (const float* const* readPointers, float* const* writePointers,
                     int numChannels, int numSamples)
{
}

void Synth::noteOn (int midiNote, int midiVelocity)
{
}

void Synth::noteOff (int midiNote, int midiVelocity)
{
}

void Synth::controlChange (int controlNumber, int normalizedValue)
{
}

void Synth::pitchBend (float signedNormalizedValue)
{
}

int Synth::getNumVoices() const noexcept
{
    return static_cast<int>(mSynthVoices.size());
}

void Synth::setNumVoices(int numVoices)
{
    // too big or too small
    if(numVoices < 0 || numVoices > 10 )
        return;

    assert (mWavetable != nullptr && "Synth::setNumVoices called before mWavetable was initialized");
    if(mWavetable == nullptr)
        return;
        
    mSynthVoices.clear();
    mSynthVoices.reserve(static_cast<std::size_t>(numVoices));

    for(int i = 0; i < numVoices; i++)
    {
        mSynthVoices.emplace_back(*mWavetable);
    }
}
} // namespace rd_dsp
