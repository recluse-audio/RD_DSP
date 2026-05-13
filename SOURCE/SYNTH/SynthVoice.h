/**
 * Created on 2026-05-13 by Ryan Devens with Peace and Love
 */

#pragma once

#include <cstdint>
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
    virtual ~SynthVoice();

    SynthVoice(SynthVoice&&) noexcept;
    SynthVoice(const SynthVoice&) = delete;
    SynthVoice& operator=(SynthVoice&&) = delete;       // cannot reseat reference member
    SynthVoice& operator=(const SynthVoice&) = delete;

    std::string getEngineName() const { return "rd_dsp::SynthVoice"; }

    virtual void prepare(double sampleRate, int maxBlockSize);
    virtual void process(const float* const* readPointers, float* const* writePointers,
                         int numChannels, int numSamples);

    virtual void noteOn(int midiNoteNumber, float velocity);
    // this sends the message, but doesn't always automatically free the voice
    // that may be prolonged if there is an envelope
    virtual void noteOff(float velocity);

    static float midiToHertz (int midiNoteNumber) noexcept;

    virtual bool isActive();
    virtual int getCurrentActiveNote();

    void setAge(std::uint64_t age) noexcept { mAge = age; }
    std::uint64_t getAge() const noexcept { return mAge; }
protected:
    friend class SynthVoiceTester;

    Wavetable& mWavetable;
    std::unique_ptr<Oscillator> mOscillator;
    int mCurrentMidiNote = -1;
    bool mIsActive = false;
    std::uint64_t mAge = 0;

    double mSampleRate = 48000;
    int mBlockSize = 512;
};

} // namespace rd_dsp
