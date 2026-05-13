/**
 * Created on 2026-05-13 by Ryan Devens with Peace and Love
 */

#pragma once

#include "../SYNTH/Synth.h"
#include "PulsarVoice.h"

#include <cstdint>
#include <string>
#include <vector>

namespace rd_dsp
{

/**
 * Pulsar-style synth. Reuses Synth via private inheritance for wavetable / sample-rate
 * plumbing, but does not expose noteOn/noteOff. Owns its own PulsarVoice pool; grains
 * are scheduled internally at mRate and play for mGrainCycles waveform cycles.
 */
class PulsarSynth : private Synth
{
public:
    PulsarSynth();
    ~PulsarSynth() override;

    std::string getEngineName() const { return "rd_dsp::PulsarSynth"; }

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples) override;

    void setNumVoices(int numVoices) override;
    int  getNumVoices() const noexcept override;

    void setRate(float hz) noexcept;
    void setGrainFrequency(float hz) noexcept;
    void setGrainCycles(int numCycles) noexcept;

    float getRate() const noexcept { return mRate; }
    float getGrainFrequency() const noexcept { return mGrainFrequency; }
    int   getGrainCycles() const noexcept { return mGrainCycles; }

    using Synth::setWavePosition;
    using Synth::loadWavetable;

protected:
    friend class PulsarSynthTester;

    std::vector<PulsarVoice> mPulsarVoices;

    float mRate = 0.f;
    float mGrainFrequency = 0.f;
    int   mGrainCycles = 2;

    double mSamplesUntilNextGrain = 0.0;
    std::uint64_t mNextGrainAge = 0;

    PulsarVoice* _findFreePulsarVoice();
    PulsarVoice* _findOldestPulsarVoice();

    void _fireGrain();
};

} // namespace rd_dsp
