/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#pragma once

#include "SynthVoice.h"

#include <memory>
#include <string>
#include <vector>

namespace rd_dsp
{
class Wavetable;

/**
 *
 */
class Synth
{
public:
    Synth();
    ~Synth();

    std::string getEngineName() const { return "rd_dsp::Synth"; }

    void prepare(double sampleRate, int maxBlockSize);
    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples);

    void noteOn(int midiNote, int midiVelocity);
    void noteOff(int midiNote, int midiVelocity);

    void controlChange(int controlNumber, int normalizedValue);
    void pitchBend(float signedNormalizedValue);

    void setNumVoices(int numVoices);
    int  getNumVoices() const noexcept;
private:
    std::unique_ptr<Wavetable> mWavetable;
    std::vector<SynthVoice> mSynthVoices;
};

} // namespace rd_dsp
