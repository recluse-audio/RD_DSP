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
    mBuffer.clear();
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 0)
        return;

    // Band-limited triangle via additive synthesis. Must match
    // TESTS/WAVEFORM/GOLDEN/gen_golden_triangle.py sample-for-sample.
    constexpr int    kNumHarmonics = 31;
    constexpr double kPi           = 3.14159265358979323846;
    constexpr double kCoeff        = 8.0 / (kPi * kPi);

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        double acc = 0.0;
        for (int k = 0; k <= kNumHarmonics; ++k)
        {
            const int    n    = 2 * k + 1;
            const double sign = (k & 1) ? -1.0 : 1.0;
            const double arg  = 2.0 * kPi * n * static_cast<double>(sampleIndex)
                              / static_cast<double>(numSamples);
            acc += sign * std::sin (arg) / static_cast<double>(n * n);
        }
        mBuffer.setSample (0, sampleIndex, static_cast<float>(kCoeff * acc));
    }
}

void Waveform::_fillWithSquare()
{
    mBuffer.clear();
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 0)
        return;

    // Band-limited square via additive synthesis. Must match
    // TESTS/WAVEFORM/GOLDEN/gen_golden_square.py sample-for-sample.
    constexpr int    kNumHarmonics = 31;
    constexpr double kPi           = 3.14159265358979323846;
    constexpr double kCoeff        = 4.0 / kPi;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        double acc = 0.0;
        for (int k = 0; k <= kNumHarmonics; ++k)
        {
            const int    n   = 2 * k + 1;
            const double arg = 2.0 * kPi * n * static_cast<double>(sampleIndex)
                             / static_cast<double>(numSamples);
            acc += std::sin (arg) / static_cast<double>(n);
        }
        mBuffer.setSample (0, sampleIndex, static_cast<float>(kCoeff * acc));
    }
}

void Waveform::_fillWithSaw()
{
    mBuffer.clear();
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 0)
        return;

    // Band-limited ascending saw via additive synthesis. Must match
    // TESTS/WAVEFORM/GOLDEN/gen_golden_saw.py sample-for-sample.
    constexpr int    kNumHarmonics = 32;
    constexpr double kPi           = 3.14159265358979323846;
    constexpr double kCoeff        = 2.0 / kPi;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        double acc = 0.0;
        for (int n = 1; n <= kNumHarmonics; ++n)
        {
            const double sign = (n % 2 == 1) ? 1.0 : -1.0;
            const double arg  = 2.0 * kPi * n * static_cast<double>(sampleIndex)
                              / static_cast<double>(numSamples);
            acc += sign * std::sin (arg) / static_cast<double>(n);
        }
        mBuffer.setSample (0, sampleIndex, static_cast<float>(kCoeff * acc));
    }
}

} // namespace rd_dsp
