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
class Pulsar;
class Wavetable;
class Window;
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

    void setEmissionRate(float emissionRate) noexcept;
    float getEmissionRate() const noexcept;

    void setFormantFreq(float formantFreq) noexcept;
    float getFormantFreq() const noexcept;

private:
    friend class PulsarTrainTester;

    std::unique_ptr<Wavetable> mWavetable;
    std::unique_ptr<Window> mWindow;
    std::unique_ptr<Pulsar> mPulsar;
    
    std::atomic<float> mEmissionRate { 0.f };
    std::atomic<float> mFormantFreq { 0.f };
    std::atomic<float> mEmissionPeriod { 0.f };

    double mSampleRate = 48000.0;
    int mBlockSize = 512;

    void _updateEmissionPeriod() noexcept;
};

} // namespace rd_dsp