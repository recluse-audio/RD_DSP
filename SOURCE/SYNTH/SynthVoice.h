/**
 * Created on 2026-05-13 by Ryan Devens with Peace and Love
 */

#pragma once

#include <memory>
#include <string>

namespace rd_dsp
{
class Wavetable;
class Oscillator;

/**
 * Owns an oscillator and is aware of envelopes / note on events.
 * Turns the oscillator on/off as needed.
 */
class SynthVoice
{
public:
    SynthVoice(Wavetable& waveTable);
    ~SynthVoice();

    SynthVoice(SynthVoice&&) noexcept;
    SynthVoice(const SynthVoice&) = delete;
    SynthVoice& operator=(SynthVoice&&) = delete;       // cannot reseat reference member
    SynthVoice& operator=(const SynthVoice&) = delete;

    std::string getEngineName() const { return "rd_dsp::SynthVoice"; }

    void prepare(double sampleRate, int maxBlockSize);
    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples);

    void noteOn(int midiNoteNumber, float velocity);
    void noteOff(float velocity);

private:
    Wavetable& mWavetable;
    std::unique_ptr<Oscillator> mOscillator;
    int mCurrentMidiNote = -1;
};

} // namespace rd_dsp
