#include "Waveform.h"

#include <cmath>

namespace rd_dsp
{

namespace
{
    constexpr float kTwoPi = 6.28318530717958647692f;
}

Waveform::Waveform()  = default;
Waveform::~Waveform() = default;

void Waveform::setSize (int numChannels, int numSamples)
{
    mBuffer.setSize (numChannels, numSamples, true);
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

void Waveform::_fillWithSine()
{
    mBuffer.clear();
    const int numChannels = mBuffer.getNumChannels();
    const int numSamples  = mBuffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float phase  = (static_cast<float>(sampleIndex) * kTwoPi) / static_cast<float>(numSamples);
        const float sample = std::sin (phase);
        for (int channel = 0; channel < numChannels; ++channel)
        {
            mBuffer.setSample (channel, sampleIndex, sample);
        }
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
