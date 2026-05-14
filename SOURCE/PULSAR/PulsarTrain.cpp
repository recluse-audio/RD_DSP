#include "PULSAR/PulsarTrain.h"
#include "PULSAR/Pulsar.h"

#include "../WAVEFORM/Wavetable.h"
#include "../WAVEFORM/WaveFactory.h"
#include "../WINDOW/Window.h"
#include "../HELPERS/CsvLoader.h"

#include <cassert>

namespace rd_dsp
{
PulsarTrain::PulsarTrain()
    : mWavetable (std::make_unique<Wavetable>())
    , mWindow (std::make_unique<Window>())
    , mPulsar (std::make_unique<Pulsar>(*mWavetable, *mWindow))
{
}

PulsarTrain::~PulsarTrain() = default;

void PulsarTrain::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate; mBlockSize = maxBlockSize;
    mWindow->setSize (static_cast<int> (sampleRate));
    mPulsar->prepare (sampleRate, maxBlockSize);
    mEmissionPeriodUpdateNeeded.store (true, std::memory_order_relaxed);
}

void PulsarTrain::process (const float* const* readPointers, float* const* writePointers,
                           int numChannels, int numSamples)
{
    if (! mIsRunning.load (std::memory_order_relaxed))
    {
        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = 0; i < numSamples; ++i)
                writePointers[ch][i] = 0.f;
        return;
    }

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float count  = mEmissionCount.load  (std::memory_order_relaxed);
        const float period = mEmissionPeriod.load (std::memory_order_relaxed);
        if (count >= period)
            _emitPulsar();

        const float sample = mPulsar->processSingleSample();
        for (int ch = 0; ch < numChannels; ++ch)
            writePointers[ch][sampleIndex] = sample;

        mEmissionCount.store (mEmissionCount.load (std::memory_order_relaxed) + 1.f,
                              std::memory_order_relaxed);
    }
}

void PulsarTrain::_emitPulsar()
{
    if (mEmissionPeriodUpdateNeeded.load (std::memory_order_relaxed))
    {
        _updateEmissionPeriod();
        mEmissionPeriodUpdateNeeded.store (false, std::memory_order_relaxed);
    }

    mEmissionCount.store (0.f, std::memory_order_relaxed);

    const float freq = mFormantFreq.load (std::memory_order_relaxed);
    const int dutyCycle = (freq <= 0.f) ? 0
                                        : static_cast<int> (static_cast<float> (mSampleRate) / freq);
    mPulsar->emit (freq, dutyCycle);
}

void PulsarTrain::setEmissionRate (float emissionRate) noexcept
{
    mEmissionRate.store (emissionRate, std::memory_order_relaxed);
    mEmissionPeriodUpdateNeeded.store (true, std::memory_order_relaxed);
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

void PulsarTrain::setWindowType (WindowType windowType)
{
    mWindow->setShape (windowType);
}

void PulsarTrain::start()
{
    if (mEmissionPeriodUpdateNeeded.load (std::memory_order_relaxed))
    {
        _updateEmissionPeriod();
        mEmissionPeriodUpdateNeeded.store (false, std::memory_order_relaxed);
    }
    // Seed count >= period so first process tick triggers an emit.
    mEmissionCount.store (mEmissionPeriod.load (std::memory_order_relaxed),
                          std::memory_order_relaxed);
    mIsRunning.store (true, std::memory_order_relaxed);
}

void PulsarTrain::stop()
{
    mIsRunning.store (false, std::memory_order_relaxed);
}


} // namespace rd_dsp
