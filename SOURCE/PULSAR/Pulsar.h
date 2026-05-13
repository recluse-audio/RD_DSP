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
/**
 * One emmission from the train
 */
class Pulsar
{
public:
    Pulsar(Wavetable& wavetable);
    ~Pulsar();

    Pulsar(Pulsar&&) noexcept;
    Pulsar(const Pulsar&) = delete;
    Pulsar& operator=(Pulsar&&) = delete;
    Pulsar& operator=(const Pulsar&) = delete;

    std::string getEngineName() const { return "rd_dsp::Pulsar"; }

    void prepare(double sampleRate, int maxBlockSize);
    void process(const float* const* readPointers, float* const* writePointers,
                 int numChannels, int numSamples);


private:
    friend class PulsarTester;

    Wavetable& mWavetable;
    std::unique_ptr<Oscillator> mOscillator;

    float mNormalizedWavePos = 0.f;
    float mCurrentFreqHz = 0.f;


};

} // namespace rd_dsp
