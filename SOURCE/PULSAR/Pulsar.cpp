#include "PULSAR/Pulsar.h"

#include "../WAVEFORM/Wavetable.h"
#include "../OSCILLATOR/Oscillator.h"

namespace rd_dsp
{
Pulsar::Pulsar (Wavetable& wavetable)
    : mWavetable (wavetable)
    , mOscillator (std::make_unique<Oscillator>(wavetable))
{
}

Pulsar::~Pulsar() = default;
Pulsar::Pulsar (Pulsar&&) noexcept = default;

void Pulsar::prepare (double sampleRate, int maxBlockSize)
{
    mOscillator->prepare (sampleRate, maxBlockSize);
}

void Pulsar::process (const float* const* readPointers, float* const* writePointers,
                      int numChannels, int numSamples)
{

}

} // namespace rd_dsp
