#include "PulsarSynth.h"

#include "../WAVEFORM/Wavetable.h"

#include <algorithm>
#include <cassert>

namespace rd_dsp
{

PulsarSynth::PulsarSynth()
{
    // Base Synth ctor seeded its own SynthVoice pool; we don't use it.
    mSynthVoices.clear();
    this->setNumVoices(1);
}

PulsarSynth::~PulsarSynth() = default;

void PulsarSynth::prepare(double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate;
    mBlockSize  = maxBlockSize;

    for (auto& voice : mPulsarVoices)
        voice.prepare(sampleRate, maxBlockSize);

    mSamplesUntilNextGrain = 0.0;
}

void PulsarSynth::process(const float* const* readPointers, float* const* writePointers,
                          int numChannels, int numSamples)
{
    if (mRate > 0.f)
    {
        const double samplesPerGrain = mSampleRate / static_cast<double>(mRate);

        // fire all grains scheduled within this block at block start
        // (block-quantized scheduling; sufficient for moderate block sizes)
        while (mSamplesUntilNextGrain < static_cast<double>(numSamples))
        {
            _fireGrain();
            mSamplesUntilNextGrain += samplesPerGrain;
        }

        mSamplesUntilNextGrain -= static_cast<double>(numSamples);
        if (mSamplesUntilNextGrain < 0.0)
            mSamplesUntilNextGrain = 0.0;
    }

    for (auto& voice : mPulsarVoices)
    {
        if (voice.isActive())
            voice.process(readPointers, writePointers, numChannels, numSamples);
    }
}

void PulsarSynth::setNumVoices(int numVoices)
{
    if (numVoices < 0 || numVoices > 10)
        return;

    assert(mWavetable != nullptr && "PulsarSynth::setNumVoices called before mWavetable was initialized");
    if (mWavetable == nullptr)
        return;

    mPulsarVoices.clear();
    mPulsarVoices.reserve(static_cast<std::size_t>(numVoices));

    for (int i = 0; i < numVoices; ++i)
        mPulsarVoices.emplace_back(*mWavetable);

    for (auto& voice : mPulsarVoices)
    {
        voice.prepare(mSampleRate, mBlockSize);
        voice.setGrainCycles(mGrainCycles);
    }
}

int PulsarSynth::getNumVoices() const noexcept
{
    return static_cast<int>(mPulsarVoices.size());
}

void PulsarSynth::setRate(float hz) noexcept
{
    mRate = std::max(0.f, hz);
    mSamplesUntilNextGrain = 0.0;
}

void PulsarSynth::setGrainFrequency(float hz) noexcept
{
    mGrainFrequency = std::max(0.f, hz);
}

void PulsarSynth::setGrainCycles(int numCycles) noexcept
{
    mGrainCycles = std::max(0, numCycles);
    for (auto& voice : mPulsarVoices)
        voice.setGrainCycles(mGrainCycles);
}

PulsarVoice* PulsarSynth::_findFreePulsarVoice()
{
    for (auto& voice : mPulsarVoices)
    {
        if (!voice.isActive())
            return &voice;
    }
    return nullptr;
}

PulsarVoice* PulsarSynth::_findOldestPulsarVoice()
{
    PulsarVoice* oldest = nullptr;
    for (auto& voice : mPulsarVoices)
    {
        if (!voice.isActive())
            continue;

        if (oldest == nullptr || voice.getAge() < oldest->getAge())
            oldest = &voice;
    }
    return oldest;
}

void PulsarSynth::_fireGrain()
{
    if (mGrainFrequency <= 0.f || mGrainCycles <= 0)
        return;

    PulsarVoice* voice = _findFreePulsarVoice();
    if (voice == nullptr)
        voice = _findOldestPulsarVoice();

    if (voice == nullptr)
        return;

    voice->triggerGrain(mGrainFrequency);
    voice->setAge(mNextGrainAge++);
}

} // namespace rd_dsp
