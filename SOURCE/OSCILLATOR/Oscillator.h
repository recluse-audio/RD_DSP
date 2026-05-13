/**
 * Created on 2026-05-07 by Ryan Devens with peace and love
 */

#pragma once

#include "../WAVEFORM/Wavetable.h"
#include "../RD_BUFFER/RD_Buffer.h"

namespace rd_dsp
{
/**
 * Reads samples from a Wavetable at varied rates and writes to a buffer.
 * Does not own the wavetable; caller supplies a reference at construction.
 */
class Oscillator
{
public:
    explicit Oscillator (Wavetable& wavetable);
    ~Oscillator();

    void prepare(double sampleRate, int maxBlockSize);
    void setFreq(float freq);

    void process(RD_Buffer& buffer);
    void process(const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples);
    void start();
    void stop();
private:
    friend class OscillatorTester;
    friend class SynthVoiceTester;

    void _process(const float* const* readPointers, float* const* writePointers, int numChannels, int numSamples);

    Wavetable& mWavetable;
    float  mCurrentIndex   = 0.f;
    float  mPhaseIncrement = 0.f;
    float  mFrequency      = 0.f;
    double mSampleRate     = 44100.0;
    int    mBlockSize      = 0;
    bool mIsRunning = false;
    bool mPhaseIncrementUpdateNeeded = false;

    void _incrementCurrentIndex();
    void _updatePhaseIncrement();

    static float _calculatePhaseIncrement (float freq, double sampleRate, int waveformSize) noexcept;
};

} // namespace rd_dsp
