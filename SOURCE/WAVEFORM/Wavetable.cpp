#include "Wavetable.h"

namespace rd_dsp
{

namespace
{
    constexpr int kDefaultWaveformSize = 8096;
}

Wavetable::Wavetable()
{
    fillWithBasicShapes (kDefaultWaveformSize);
}

Wavetable::~Wavetable()
{
    mWaveforms.clear();
}

void Wavetable::setNormalizedWavePosition(float normalizedWavePos)
{
    mNormalizedWavePos.store (normalizedWavePos, std::memory_order_relaxed);
}

float Wavetable::getSampleAtIndex (float index) const noexcept
{
    const int lastWave = static_cast<int>(mWaveforms.size()) - 1;
    if (lastWave < 0)
        return 0.f;

    float wavePos = _getPositionForWavetableSize();
    if (wavePos < 0.f)             wavePos = 0.f;
    if (wavePos > (float)lastWave) wavePos = (float)lastWave;

    int lowerWave = (int)wavePos;
    if (lowerWave > lastWave) lowerWave = lastWave;
    int upperWave = (lowerWave < lastWave) ? lowerWave + 1 : lowerWave;
    float spillover = wavePos - (float) lowerWave;

    float lowerSample = mWaveforms[lowerWave]->getInterpolatedSampleAtIndex(index) * (1.f - spillover);
    float upperSample = mWaveforms[upperWave]->getInterpolatedSampleAtIndex(index) * spillover;

    return lowerSample + upperSample; // don't divide by 2 here, accounted for in spillover
}

void Wavetable::fillDisplayBuffer (float* out, int outSize) const noexcept
{
    if (out == nullptr || outSize <= 0 || mWaveforms.empty())
        return;

    const float waveSize = static_cast<float>(getWaveformSize());
    if (waveSize <= 0.f)
        return;

    const float step = waveSize / static_cast<float>(outSize);
    for (int i = 0; i < outSize; ++i)
        out[i] = getSampleAtIndex (static_cast<float>(i) * step);
}

void Wavetable::fillDisplayBufferAveraged (float* out, int outSize) const noexcept
{
    if (out == nullptr || outSize <= 0 || mWaveforms.empty())
        return;

    const int waveSize = getWaveformSize();
    if (waveSize <= 0)
        return;

    const float step = static_cast<float>(waveSize) / static_cast<float>(outSize);

    // No decimation -> point-sample (avoids degenerate 1-sample bins).
    if (step <= 1.f)
    {
        for (int i = 0; i < outSize; ++i)
            out[i] = getSampleAtIndex (static_cast<float>(i) * step);
        return;
    }

    for (int i = 0; i < outSize; ++i)
    {
        int start = static_cast<int>(static_cast<float>(i) * step);
        int end   = static_cast<int>(static_cast<float>(i + 1) * step);
        if (end <= start) end = start + 1;
        if (end > waveSize) end = waveSize;

        float sum = 0.f;
        for (int s = start; s < end; ++s)
            sum += getSampleAtIndex (static_cast<float>(s));

        out[i] = sum / static_cast<float>(end - start);
    }
}

int Wavetable::getNumWaveforms() const noexcept
{
    return static_cast<int>(mWaveforms.size());
}

int Wavetable::getWaveformSize() const noexcept
{
    if (mWaveforms.empty())
        return 0;
    return mWaveforms[0]->getNumSamples();
}

void Wavetable::addWaveform(std::unique_ptr<Waveform> waveForm)
{
    mWaveforms.push_back(std::move(waveForm));
}

void Wavetable::clear() noexcept
{
    mWaveforms.clear();
}

void Wavetable::fillWithBasicShapes (int numSamples)
{
    mWaveforms.clear();

    constexpr Waveform::WaveType types[] = {
        Waveform::WaveType::wSine,
        Waveform::WaveType::wTri,
        Waveform::WaveType::wSquare,
        Waveform::WaveType::wSaw,
    };

    for (auto type : types)
    {
        auto wave = std::make_unique<Waveform>();
        wave->setSize (numSamples);
        wave->setWaveType (type);
        mWaveforms.push_back (std::move (wave));
    }
}

float Wavetable::_getPositionForWavetableSize() const noexcept
{
    float worldWavePos = (float)(mWaveforms.size()) * mNormalizedWavePos.load (std::memory_order_relaxed);
    return worldWavePos;
}

} // namespace rd_dsp
