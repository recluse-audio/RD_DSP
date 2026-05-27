/**
 * Created by Ryan Devens w/ peace and love
 *
 *
 */

#pragma once

#include <atomic>
#include <memory>
#include <string>

namespace rd_dsp
{
class Oscillator;
class Wavetable;
class Window;
/**
 * One emmission from the train
 */
class Pulsar
{
public:
    Pulsar(Wavetable& wavetable, Window& window);
    ~Pulsar();

    Pulsar(Pulsar&&) noexcept;
    Pulsar(const Pulsar&) = delete;
    Pulsar& operator=(Pulsar&&) = delete;
    Pulsar& operator=(const Pulsar&) = delete;

    std::string getEngineName() const { return "rd_dsp::Pulsar"; }

    void prepare(double sampleRate, int maxBlockSize);
    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples);
    float processSingleSample();

    // with no width the dutyCycleSamples = 2x the freq converted to period in samples
    // amp scales this emission's output; unity by default (separate from any train-level gain)
    // wavePos is this emission's normalized wavetable position, latched and pushed
    // to the wavetable so the duty-cycle render reads from it
    void emit(float formantFreq, int dutyCycleSamples, float amp = 1.0f, float wavePos = 0.0f);

    bool isActive() const noexcept;

private:
    friend class PulsarTester;

    Wavetable& mWavetable;
    Window& mWindow;
    std::unique_ptr<Oscillator> mOscillator;

    float mWindowPos = 0.f;
    float mWindowIncrement = 0.f;
    float mAmp = 1.f; // this emission's amplitude scalar, set on emit
    float mWavePos = 0.f; // this emission's normalized wavetable position, latched on emit

    std::atomic<bool> mIsActive { false };
    double mSampleRate = 44100;

    int mDutyCycleSamples = 0;

};

} // namespace rd_dsp
