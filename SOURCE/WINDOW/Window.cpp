#include "Window.h"
#include "../INTERPOLATOR/Interpolator.h"

#include <cmath>

namespace rd_dsp
{

namespace
{
    constexpr double kPi          = 3.14159265358979323846;
    constexpr float  kTukeyRatio  = 0.8f; // matches RD default
}

Window::Window()
{
    mBuffer.setSize (1, 1024, true);
    _update();
}

Window::~Window() = default;

void Window::setSize (int numSamples)
{
    mBuffer.setSize (1, numSamples, true);
    _update();
}

void Window::setShape (Shape shape)
{
    if (mCurrentShape == shape)
        return;

    mCurrentShape = shape;
    _update();
}

float Window::getInterpolatedSampleAtIndex (float index)
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

void Window::_update()
{
    switch (mCurrentShape)
    {
        case Shape::kNone:    _fillWithOnes();    break;
        case Shape::kHanning: _fillWithHanning(); break;
        case Shape::kTukey:   _fillWithTukey();   break;
    }
}

void Window::_fillWithOnes()
{
    const int numSamples = mBuffer.getNumSamples();
    for (int i = 0; i < numSamples; ++i)
        mBuffer.setSample (0, i, 1.0f);
}

void Window::_fillWithHanning()
{
    mBuffer.clear();
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 1)
        return;

    const double denom = static_cast<double>(numSamples - 1);
    for (int i = 0; i < numSamples; ++i)
    {
        const double v = 0.5 * (1.0 - std::cos (2.0 * kPi * static_cast<double>(i) / denom));
        mBuffer.setSample (0, i, static_cast<float>(v));
    }
}

void Window::_fillWithTukey()
{
    mBuffer.clear();
    const int numSamples = mBuffer.getNumSamples();
    if (numSamples <= 1)
        return;

    const double alpha = static_cast<double>(kTukeyRatio);

    // Tukey is rectangular middle with cosine tapers of width alpha*(N-1)/2 on each end.
    if (alpha <= 0.0)
    {
        _fillWithOnes();
        return;
    }

    const double N        = static_cast<double>(numSamples - 1);
    const double taperEnd = alpha * N * 0.5;

    for (int i = 0; i < numSamples; ++i)
    {
        const double n = static_cast<double>(i);
        double v;
        if (n < taperEnd)
            v = 0.5 * (1.0 + std::cos (kPi * (n / taperEnd - 1.0)));
        else if (n <= N - taperEnd)
            v = 1.0;
        else
            v = 0.5 * (1.0 + std::cos (kPi * ((n - (N - taperEnd)) / taperEnd)));

        mBuffer.setSample (0, i, static_cast<float>(v));
    }
}

} // namespace rd_dsp
