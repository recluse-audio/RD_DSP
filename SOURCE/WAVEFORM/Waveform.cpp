#include "Waveform.h"
#include "../INTERPOLATOR/Interpolator.h"

#include <cmath>

namespace rd_dsp
{

namespace
{
    constexpr float kTwoPi = 6.28318530717958647692f;
}

Waveform::Waveform()  = default;
Waveform::~Waveform() = default;

void Waveform::setSize (int numSamples)
{
    mBuffer.setSize (1, numSamples, true);
}

void Waveform::setWaveType (WaveType waveType)
{
    switch (waveType)
    {
        case WaveType::wSine:   _fillWithSine();   break;
        case WaveType::wTri:    _fillWithTri();    break;
        case WaveType::wSquare: _fillWithSquare(); break;
        case WaveType::wSaw:    _fillWithSaw();    break;
    }
}

float Waveform::getInterpolatedSampleAtIndex (float index)
{
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 0)
        return 0.0f;

    const float n = static_cast<float>(numSamples);
    float wrapped = std::fmod (index, n);
    if (wrapped < 0.0f)
        wrapped += n;

    const int   i0   = static_cast<int>(std::floor (wrapped));
    const int   i1   = (i0 + 1) % numSamples;
    const float frac = wrapped - static_cast<float>(i0);

    const float s0 = mBuffer.getSample (0, i0);
    const float s1 = mBuffer.getSample (0, i1);
    return static_cast<float>(Interpolator::linearInterp (s0, s1, frac));
}

void Waveform::_fillWithSine()
{
    mBuffer.clear();
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 0)
        return;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float phase  = (static_cast<float>(sampleIndex) * kTwoPi) / static_cast<float>(numSamples);
        const float sample = std::sin (phase);
        mBuffer.setSample (0, sampleIndex, sample);
    }
}

void Waveform::_fillWithTri()
{
}

void Waveform::_fillWithSquare()
{
}

void Waveform::_fillWithSaw()
{
}

} // namespace rd_dsp
