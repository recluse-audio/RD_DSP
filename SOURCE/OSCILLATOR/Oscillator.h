/**
 * Created on 2026-05-07 by Ryan Devens with peace and love
 */

#pragma once

#include "../WAVEFORM/Waveform.h"
#include <memory>

namespace rd_dsp
{
/**
 * This class reads from a waveform and writes to a buffer
 * It owns a Waveform, and reads values from it at varied rates.
 * Does not need to handle interpolation between samples in Waveform, can pass
 * float sample indices to it and get the sample values it needs.
 */
class Oscillator
{
public:
    Oscillator();
    ~Oscillator();

    void prepare(double sampleRate);
    void setFreq(float freq);

    void process(RD_Buffer& buffer);
    void start();
    void stop();
private:
    friend class OscillatorTester;
    std::unique_ptr<Waveform> mWaveform;
    float  mCurrentIndex   = 0.f;
    float  mPhaseIncrement = 0.f;
    float  mFrequency      = 0.f;
    double mSampleRate     = 44100.0;
    bool mIsRunning = false;
    bool mPhaseIncrementUpdateNeeded = false;

    // this uses mCurrentIndex 
    void _incrementCurrentIndex();

    // depends on mSampleRate and mFrequency being set.
    void _updatePhaseIncrement();

    // Pure formula. Exposed (privately) so tests can drive it with
    // explicit waveform sizes that match a golden CSV.
    static float _calculatePhaseIncrement (float freq, double sampleRate, int waveformSize) noexcept;
};



} // namespace rd_dsp
