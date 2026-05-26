/**
 * Created by Ryan Devens w/ peace and love
 * 
 * 
 */

#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "../WINDOW/Window.h"
#include "../RANDOMIZER/RandomizedParam.h"
#include "PulsarData.h"

namespace rd_dsp
{
class Pulsar;
class Wavetable;

using WindowType = Window::Shape;
/**
 * This emits pulsarets at a given emission rate and formant freq
 * 
 * Decidely not a "synth" with midi stuff for it
 * 
 * PulsarTrain - Responsible for emitting pulsars at correct time / with correct formant/fund periods
 * Pulsar - Responsible for stopping at the right time after its duty cycle
 */
class PulsarTrain
{
public:
    static constexpr float kMinEmissionRate = 0.125f;
    static constexpr float kMaxEmissionRate = 150.f;
    static constexpr float kMinFormantFreq  = 150.f;
    static constexpr float kMaxFormantFreq  = 2000.f;

    PulsarTrain();
    ~PulsarTrain();

    std::string getEngineName() const { return "rd_dsp::PulsarTrain"; }

    void prepare(double sampleRate, int maxBlockSize);
    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples);

    void loadWavetable(std::string tablePath);
    void setWavePosition(float wavePos);

    const Wavetable& getWavetable() const noexcept;

    void setEmissionRate(float emissionRate) noexcept;
    float getEmissionRate() const noexcept;

    void setEmissionRange(float minRate, float maxRate) noexcept;
    void setEmissionDensity(float density) noexcept;

    void setFormantFreq(float formantFreq) noexcept;
    float getFormantFreq() const noexcept;

    void setFormantRange(float minFreq, float maxFreq) noexcept;
    void setFormantDensity(float density) noexcept;

    void setWindowType(WindowType windowType);

    void start();
    void stop();

    bool isActive() const noexcept;

    bool consumePulsarFlash() noexcept;

private:
    friend class PulsarTrainTester;

    std::unique_ptr<Wavetable> mWavetable;
    std::unique_ptr<Window> mWindow;
    std::unique_ptr<Pulsar> mPulsar;

    PulsarData mPulsarData; // per-emission, per-pulsar randomization config + draw (formant freq center = the set freq)
    RandomizedParam mEmissionRateRandom; // train-level: emission rate center = the set rate, randomizes the period

    std::atomic<float> mEmissionCount { 0.f }; // count towards next emission, incremented per sample
    std::atomic<float> mEmissionPeriod { 0.f }; // number to count to, only updated once per emission (not changing countdown goal mid emission)
    std::atomic<bool>  mEmissionPeriodUpdateNeeded { false };
    std::atomic<bool>  mIsRunning { false };
    std::atomic<bool>  mPulsarReportedToGUI { true }; // true = current pulsar already drawn


    double mSampleRate = 48000.0;
    int mBlockSize = 512;

    void _updateEmissionPeriod() noexcept;
    void _emitPulsar();

};

} // namespace rd_dsp