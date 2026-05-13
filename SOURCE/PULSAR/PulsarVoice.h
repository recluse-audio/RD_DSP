/**
 * Created on 2026-05-13 by Ryan Devens with Peace and Love
 */

#pragma once

#include "../SYNTH/SynthVoice.h"

namespace rd_dsp
{

/**
 * Grain-emitting voice. Plays N cycles of waveform at given frequency, then auto-stops.
 * Not sustained: noteOff is a no-op (immediate stop happens automatically after N cycles).
 */
class PulsarVoice : public SynthVoice
{
public:
    using SynthVoice::SynthVoice;
    PulsarVoice(PulsarVoice&&) noexcept = default;
    ~PulsarVoice() override;

    std::string getEngineName() const { return "rd_dsp::PulsarVoice"; }

    void prepare(double sampleRate, int maxBlockSize) override;

    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples) override;

    void noteOn(int midiNoteNumber, float velocity) override;
    void noteOff(float velocity) override;

    void triggerGrain(float frequencyHz);
    void setGrainCycles(int numCycles) noexcept;
    int  getGrainCycles() const noexcept { return mGrainCycles; }

protected:
    friend class PulsarVoiceTester;

    int mGrainCycles = 2;
    int mSamplesRemaining = 0;
    float mGrainFrequency = 0.f;

    void _armGrain(float freq);
};

} // namespace rd_dsp
