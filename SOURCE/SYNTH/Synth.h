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

    void noteOn(int midiNote, float midiVelocity);
    void noteOff(int midiNote, float midiVelocity);

    void controlChange(int controlNumber, float normalizedValue);
    void pitchBend(float signedNormalizedValue);

    void setNumVoices(int numVoices);
    int  getNumVoices() const noexcept;

    void setWavePosition(float wavePos);
    void loadWavetable(std::string tablePath);
private:
friend class SynthTester;
    std::unique_ptr<Wavetable> mWavetable;
    std::vector<SynthVoice> mSynthVoices;

    double mSampleRate = 48000.0;
    int mBlockSize = 512;

    SynthVoice* _findFreeVoice();
    SynthVoice* _findActiveVoice(int midiNoteNumber);
};

} // namespace rd_dsp
