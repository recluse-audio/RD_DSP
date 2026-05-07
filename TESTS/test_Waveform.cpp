#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../SOURCE/WAVEFORM/Waveform.h"
#include "../SOURCE/INTERPOLATOR/Interpolator.h"

#include <cmath>

using rd_dsp::Waveform;
using Catch::Approx;

namespace
{
    constexpr float kTwoPi = 6.28318530717958647692f;
}

TEST_CASE ("Waveform::getInterpolatedSampleAtIndex returns exact sample at integer index", "[Waveform]")
{
    Waveform wf;
    wf.setSize (8);
    wf.setWaveType (Waveform::WaveType::wSine);

    for (int i = 0; i < wf.getNumSamples(); ++i)
    {
        const float expected = wf.getSample (i);
        CHECK (wf.getInterpolatedSampleAtIndex (static_cast<float>(i)) == Approx(expected));
    }
}

TEST_CASE ("Waveform::getInterpolatedSampleAtIndex linearly interpolates between samples", "[Waveform]")
{
    Waveform wf;
    const int n = 8;
    wf.setSize (n);
    wf.setWaveType (Waveform::WaveType::wSine);

    const float s0   = wf.getSample (2);
    const float s1   = wf.getSample (3);
    const float frac = 0.25f;
    const float expected = static_cast<float>(rd_dsp::Interpolator::linearInterp (s0, s1, frac));

    CHECK (wf.getInterpolatedSampleAtIndex (2.0f + frac) == Approx(expected));
}

TEST_CASE ("Waveform::getInterpolatedSampleAtIndex wraps cyclically at boundaries", "[Waveform]")
{
    Waveform wf;
    const int n = 8;
    wf.setSize (n);
    wf.setWaveType (Waveform::WaveType::wSine);

    // Index == numSamples wraps to index 0.
    CHECK (wf.getInterpolatedSampleAtIndex (static_cast<float>(n))
           == Approx(wf.getSample (0)));

    // Fractional index between last sample and wrap-around to sample 0.
    const float sLast = wf.getSample (n - 1);
    const float sZero = wf.getSample (0);
    const float expected = static_cast<float>(rd_dsp::Interpolator::linearInterp (sLast, sZero, 0.5));
    CHECK (wf.getInterpolatedSampleAtIndex (static_cast<float>(n - 1) + 0.5f)
           == Approx(expected));

    // Negative index wraps from the end.
    CHECK (wf.getInterpolatedSampleAtIndex (-1.0f)
           == Approx(wf.getSample (n - 1)));
}

TEST_CASE ("Waveform::getInterpolatedSampleAtIndex midpoint of full sine matches analytic value", "[Waveform]")
{
    Waveform wf;
    const int n = 16;
    wf.setSize (n);
    wf.setWaveType (Waveform::WaveType::wSine);

    // Integer index n/4 corresponds to phase pi/2 -> sin == 1.
    CHECK (wf.getInterpolatedSampleAtIndex (static_cast<float>(n) * 0.25f) == Approx(1.0f).margin(1e-6));
    // Integer index n/2 corresponds to phase pi -> sin == 0.
    CHECK (wf.getInterpolatedSampleAtIndex (static_cast<float>(n) * 0.5f) == Approx(0.0f).margin(1e-6));
}
