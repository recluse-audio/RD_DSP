/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#pragma once

#include "SynthVoice.h"

#include <cstdint>
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
    virtual ~Synth();

    virtual std::string getEngineName() const { return "rd_dsp::Synth"; }

    virtual void prepare(double sampleRate, int maxBlockSize);
    virtual void process(const float* const* readPointers, float* const* writePointers,
                         int numChannels, int numSamples);

    virtual void noteOn(int midiNote, float midiVelocity);
    virtual void noteOff(int midiNote, float midiVelocity);

    virtual void controlChange(int controlNumber, float normalizedValue);
    virtual void pitchBend(float signedNormalizedValue);

    virtual void setNumVoices(int numVoices);
    virtual int  getNumVoices() const noexcept;

    virtual void setWavePosition(float wavePos);
    virtual void loadWavetable(std::string tablePath);
protected:
    friend class SynthTester;
    std::unique_ptr<Wavetable> mWavetable;
    std::vector<SynthVoice> mSynthVoices;

    double mSampleRate = 48000.0;
    int mBlockSize = 512;
    std::uint64_t mNextVoiceAge = 0;

    SynthVoice* _findFreeVoice();
    SynthVoice* _findActiveVoice(int midiNoteNumber);
    SynthVoice* _findOldestVoice();
};

} // namespace rd_dsp
