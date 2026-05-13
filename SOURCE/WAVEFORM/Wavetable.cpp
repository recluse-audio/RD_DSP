#include "Wavetable.h"

namespace rd_dsp
{

Wavetable::Wavetable()
{
    mWaveforms.clear();
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
    float wavePos = _getPositionForWavetableSize();
    int lowerWave = (int)wavePos;
    int upperWave = lowerWave + 1;
    float spillover = wavePos - (float) lowerWave;

    float lowerSample = mWaveforms[lowerWave]->getInterpolatedSampleAtIndex(index) * (1.f - spillover);
    float upperSample = mWaveforms[upperWave]->getInterpolatedSampleAtIndex(index) * spillover;

    return lowerSample + upperSample; // don't divide by 2 here, accounted for in spillover
}

int Wavetable::getNumWaveforms() const noexcept
{
    return static_cast<int>(mWaveforms.size());
}

void Wavetable::addWaveform(std::unique_ptr<Waveform> waveForm)
{
    mWaveforms.push_back(std::move(waveForm));
}

float Wavetable::_getPositionForWavetableSize()
{
    float worldWavePos = (float)(mWaveforms.size()) * mNormalizedWavePos;
    return worldWavePos;
}

} // namespace rd_dsp
