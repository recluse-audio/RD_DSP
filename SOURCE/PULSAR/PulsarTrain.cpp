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
    mEmissionCountdownUpdateNeeded.store (true, std::memory_order_relaxed);
}

void PulsarTrain::process (const float* const* readPointers, float* const* writePointers,
                           int numChannels, int numSamples)
{
    if (mEmissionCountdownUpdateNeeded.load (std::memory_order_relaxed))
    {
        _updateEmissionCountdown();
        mEmissionCountdownUpdateNeeded.store (false, std::memory_order_relaxed);
    }

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        if (mEmissionCountdown.load (std::memory_order_relaxed) <= 0.f)
            _emitPulsar();

        const float sample = mPulsar->processSingleSample();
        for (int ch = 0; ch < numChannels; ++ch)
            writePointers[ch][sampleIndex] = sample;

        const float c = mEmissionCountdown.load (std::memory_order_relaxed);
        mEmissionCountdown.store (c - 1.f, std::memory_order_relaxed);
    }
}

void PulsarTrain::_emitPulsar()
{
    const float rate = mEmissionRate.load (std::memory_order_relaxed);
    const float period = (rate <= 0.f) ? 0.f : static_cast<float> (mSampleRate) / rate;
    mEmissionCountdown.store (period, std::memory_order_relaxed);

    const float freq = mFormantFreq.load (std::memory_order_relaxed);
    const int dutyCycle = (freq <= 0.f) ? 0: static_cast<int> (static_cast<float> (mSampleRate) / freq);
    mPulsar->emit (freq, dutyCycle);
}

void PulsarTrain::setEmissionRate (float emissionRate) noexcept
{
    mEmissionRate.store (emissionRate, std::memory_order_relaxed);
    mEmissionCountdownUpdateNeeded.store (true, std::memory_order_relaxed);
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

void PulsarTrain::_updateEmissionCountdown() noexcept
{
    const float rate = mEmissionRate.load (std::memory_order_relaxed);
    const float period = (rate <= 0.f) ? 0.f : static_cast<float> (mSampleRate) / rate;
    mEmissionCountdown.store (period, std::memory_order_relaxed);
}

void PulsarTrain::setWindowType (WindowType windowType)
{
    mWindow->setShape (windowType);
}


} // namespace rd_dsp
