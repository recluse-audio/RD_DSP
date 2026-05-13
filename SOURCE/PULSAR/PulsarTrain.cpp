#include "PULSAR/PulsarTrain.h"
#include "PULSAR/Pulsar.h"

#include "../WAVEFORM/Wavetable.h"
#include "../WAVEFORM/WaveFactory.h"
#include "../HELPERS/CsvLoader.h"

#include <cassert>

namespace rd_dsp
{
PulsarTrain::PulsarTrain()
    : mWavetable (std::make_unique<Wavetable>())
    , mPulsar (std::make_unique<Pulsar>(*mWavetable))
{
}

PulsarTrain::~PulsarTrain() = default;

void PulsarTrain::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate; mBlockSize = maxBlockSize;
    mPulsar->prepare (sampleRate, maxBlockSize);
    this->_updateEmissionPeriod();
}

void PulsarTrain::process (const float* const* readPointers, float* const* writePointers,
                           int numChannels, int numSamples)
{
    
}

void PulsarTrain::setEmissionRate (float emissionRate) noexcept
{
    mEmissionRate.store (emissionRate, std::memory_order_relaxed);
    this->_updateEmissionPeriod();
}

float PulsarTrain::getEmissionRate() const noexcept
{
    return mEmissionRate.load (std::memory_order_relaxed);
}

void PulsarTrain::setFormantFreq (float formantFreq) noexcept
{
    mFormantFreq.store (formantFreq, std::memory_order_relaxed);
}

float PulsarTrain::getFormantFreq() const noexcept
{
    return mFormantFreq.load (std::memory_order_relaxed);
}

void PulsarTrain::setWavePosition (float wavePos)
{
    if (mWavetable == nullptr)
        return;

    mWavetable->setNormalizedWavePosition (wavePos);
}

void PulsarTrain::loadWavetable (std::string tablePath)
{
    assert (mWavetable != nullptr && "PulsarTrain::loadWavetable called before mWavetable was initialized");
    if (mWavetable == nullptr)
        return;

    std::vector<std::vector<float>> rows;
    if (! CsvLoader::load (tablePath, rows, false))
        return;

    mWavetable->clear();

    for (const auto& row : rows)
    {
        auto wave = WaveFactory::waveformFromRow (row);
        if (wave == nullptr)
        {
            mWavetable->clear();
            return;
        }
        mWavetable->addWaveform (std::move (wave));
    }
}

void PulsarTrain::_updateEmissionPeriod() noexcept
{
    const float rate = mEmissionRate.load (std::memory_order_relaxed);
    const float period = (rate <= 0.f) ? 0.f : static_cast<float> (mSampleRate) / rate;
    mEmissionPeriod.store (period, std::memory_order_relaxed);
}

} // namespace rd_dsp
