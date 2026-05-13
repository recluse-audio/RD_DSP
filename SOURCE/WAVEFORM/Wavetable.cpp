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
    mNormalizedWavePos = normalizedWavePos;
}

float Wavetable::getSampleAtIndex(float index)
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

float Wavetable::_getPositionForWavetableSize()
{
    float worldWavePos = (float)(mWaveforms.size()) * mNormalizedWavePos;
    return worldWavePos;
}

} // namespace rd_dsp
