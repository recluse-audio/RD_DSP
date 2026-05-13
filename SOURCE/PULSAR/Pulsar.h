/**
 * Created by Ryan Devens w/ peace and love
 *
 *
 */

#pragma once

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
    void emit(float formantFreq, int dutyCycleSamples);

private:
    friend class PulsarTester;

    Wavetable& mWavetable;
    Window& mWindow;
    std::unique_ptr<Oscillator> mOscillator;

    float mWindowPos = 0.f;
    float mWindowIncrement = 0.f;

    bool mIsActive = false;
    double mSampleRate = 44100;

    int mDutyCycleSamples = 0;

};

} // namespace rd_dsp
