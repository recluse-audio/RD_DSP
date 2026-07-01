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
    mEmissionRateRandom.setRange (kMinEmissionRate, kMaxEmissionRate);
    mEmissionRateRandom.setDensity (0.0f); // no randomization until dialed up
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
    mEmissionCount.store (0.f, std::memory_order_relaxed);

    // period is updated each emission - changed by randomization and param change
    const float rate = mEmissionRateRandom.getRandomizedValue();
    mEmissionPeriod.store ((rate <= 0.f) ? 0.f : static_cast<float> (mSampleRate) / rate, std::memory_order_relaxed);
    mEmissionPeriodUpdateNeeded.store (false, std::memory_order_relaxed);

    // Per-emission params run through PulsarData: density 0 returns each center
    // (the set value) unchanged; higher density randomizes within the range.
    // Drawn once here so all params resolve for this single emission.
    const PulsarParamValues values = mPulsarData.resolve();

    const float freq = values.formantFreq;
    const int dutyCycle = (freq <= 0.f) ? 0 : static_cast<int> (static_cast<float> (mSampleRate) / freq);

    // Pulsar latches the wave position and pushes it to the wavetable for its render.
    mPulsar->emit (freq, dutyCycle, values.amp, values.wavePosition);

    // Flag a display redraw only when the wave position actually changed since the
    // last one the GUI accounted for. Density 0 keeps drawing the same center, so
    // this stays unset and the GUI never regenerates faster than it polls.
    if (values.wavePosition != mGuiWavePos.load (std::memory_order_relaxed))
    {
        mGuiWavePos.store (values.wavePosition, std::memory_order_relaxed);
        mWavePosReportedToGUI.store (false, std::memory_order_relaxed);
    }

    mPulsarReportedToGUI.store (false, std::memory_order_relaxed);
}

void PulsarTrain::setEmissionRate (float emissionRate) noexcept
{
    mEmissionRateRandom.setCenter (emissionRate);
    mEmissionPeriodUpdateNeeded.store (true, std::memory_order_relaxed);
}

float PulsarTrain::getEmissionRate() const noexcept
{
    return mEmissionRateRandom.getCenterValue();
}

void PulsarTrain::setEmissionRange (float minRate, float maxRate) noexcept
{
    mEmissionRateRandom.setRange (minRate, maxRate);
}

void PulsarTrain::setEmissionDensity (float density) noexcept
{
    mEmissionRateRandom.setDensity (density);
}

void PulsarTrain::setFormantFreq (float formantFreq) noexcept
{
    mPulsarData.formantFreq.setCenter (formantFreq);
}

float PulsarTrain::getFormantFreq() const noexcept
{
    return mPulsarData.formantFreq.getCenterValue();
}

void PulsarTrain::setFormantRange (float minFreq, float maxFreq) noexcept
{
    mPulsarData.formantFreq.setRange (minFreq, maxFreq);
}

void PulsarTrain::setFormantDensity (float density) noexcept
{
    mPulsarData.formantFreq.setDensity (density);
}

void PulsarTrain::setAmp (float amp) noexcept
{
    mPulsarData.amp.setCenter (amp);
}

float PulsarTrain::getAmp() const noexcept
{
    return mPulsarData.amp.getCenterValue();
}

void PulsarTrain::setAmpRange (float minAmp, float maxAmp) noexcept
{
    mPulsarData.amp.setRange (minAmp, maxAmp);
}

void PulsarTrain::setAmpDensity (float density) noexcept
{
    mPulsarData.amp.setDensity (density);
}

void PulsarTrain::setWavePosition (float wavePos)
{
    // The set position is the randomizer center; also pushed straight to the
    // wavetable so the display reflects it even while stopped (no emission yet).
    mPulsarData.wavePosition.setCenter (wavePos);

    // Control-side change: flag a redraw so the GUI picks up the new shape.
    mGuiWavePos.store (wavePos, std::memory_order_relaxed);
    mWavePosReportedToGUI.store (false, std::memory_order_relaxed);

    if (mWavetable == nullptr)
        return;

    mWavetable->setNormalizedWavePosition (wavePos);
}

float PulsarTrain::getWavePosition() const noexcept
{
    return mPulsarData.wavePosition.getCenterValue();
}

void PulsarTrain::setWavePositionRange (float minPos, float maxPos) noexcept
{
    mPulsarData.wavePosition.setRange (minPos, maxPos);
}

void PulsarTrain::setWavePositionDensity (float density) noexcept
{
    mPulsarData.wavePosition.setDensity (density);
}

const Wavetable& PulsarTrain::getWavetable() const noexcept
{
    assert (mWavetable != nullptr && "PulsarTrain::getWavetable called before mWavetable was initialized");
    return *mWavetable;
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
    const float rate = mEmissionRateRandom.getCenterValue();
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

bool PulsarTrain::isActive() const noexcept
{
    return mPulsar ? mPulsar->isActive() : false;
}

bool PulsarTrain::consumePulsarFlash() noexcept
{
    return ! mPulsarReportedToGUI.exchange (true, std::memory_order_relaxed);
}

bool PulsarTrain::consumeWavePositionChanged() noexcept
{
    return ! mWavePosReportedToGUI.exchange (true, std::memory_order_relaxed);
}


} // namespace rd_dsp
