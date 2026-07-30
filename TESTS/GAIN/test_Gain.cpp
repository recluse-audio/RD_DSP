#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "GAIN/Gain.h"
#include "RD_BUFFER/RD_Buffer.h"

#include <cmath>

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int    kBlockSize  = 512;

void fillWithConstant (rd_dsp::RD_Buffer& buffer, float value)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample (ch, i, value);
}
}

TEST_CASE ("Default gain is unity", "[Gain]")
{
    rd_dsp::Gain gain;

    REQUIRE (gain.getGain() == 1.f);
    REQUIRE (gain.getCurrentGain() == 1.f);
}

//-------------------------------------
TEST_CASE ("Unity gain passes the signal unchanged", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);

    rd_dsp::RD_Buffer buffer (2, kBlockSize);
    fillWithConstant (buffer, 0.5f);

    gain.process (buffer);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            REQUIRE (buffer.getSample (ch, i) == Catch::Approx (0.5f));
}

//-------------------------------------
TEST_CASE ("Zero gain gives silence once the ramp finishes", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (0.f);

    rd_dsp::RD_Buffer buffer (1, kBlockSize);

    for (int block = 0; block < 8; ++block)
    {
        fillWithConstant (buffer, 1.f);
        gain.process (buffer);
    }

    REQUIRE (gain.getCurrentGain() == 0.f);

    fillWithConstant (buffer, 1.f);
    gain.process (buffer);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == 0.f);
}

//-------------------------------------
TEST_CASE ("A gain change ramps instead of jumping", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (0.f);

    rd_dsp::RD_Buffer buffer (1, kBlockSize);
    fillWithConstant (buffer, 1.f);
    gain.process (buffer);

    const float first = buffer.getSample (0, 0);
    const float last  = buffer.getSample (0, kBlockSize - 1);

    REQUIRE (first == Catch::Approx (1.f));
    REQUIRE (last < first);
    REQUIRE (last > 0.f);

    for (int i = 1; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) <= buffer.getSample (0, i - 1));
}

//-------------------------------------
TEST_CASE ("The ramp lasts the requested number of samples", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.setRampSeconds (0.01f);
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (0.f);

    const int rampSamples = static_cast<int> (std::round (0.01f * kSampleRate));

    for (int i = 0; i < rampSamples - 1; ++i)
        gain.processSingleSample (1.f);

    REQUIRE (gain.getCurrentGain() > 0.f);

    gain.processSingleSample (1.f);

    REQUIRE (gain.getCurrentGain() == 0.f);
}

//-------------------------------------
TEST_CASE ("reset snaps the current gain to the target", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (0.25f);

    REQUIRE (gain.getCurrentGain() == 1.f);

    gain.reset();

    REQUIRE (gain.getCurrentGain() == Catch::Approx (0.25f));
}

//-------------------------------------
TEST_CASE ("A negative gain is clamped to zero", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (-2.f);

    REQUIRE (gain.getGain() == 0.f);
}

//-------------------------------------
TEST_CASE ("A gain above unity amplifies", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (2.f);
    gain.reset();

    rd_dsp::RD_Buffer buffer (1, kBlockSize);
    fillWithConstant (buffer, 0.25f);

    gain.process (buffer);

    REQUIRE (buffer.getSample (0, 0) == Catch::Approx (0.5f));
}

//-------------------------------------
TEST_CASE ("The pointer overload matches the buffer overload", "[Gain]")
{
    rd_dsp::Gain gain;
    gain.prepare (kSampleRate, kBlockSize);
    gain.setGain (0.5f);
    gain.reset();

    rd_dsp::RD_Buffer input (1, kBlockSize);
    rd_dsp::RD_Buffer output (1, kBlockSize);
    fillWithConstant (input, 1.f);
    output.clear();

    gain.process (input.getReadArray(), output.getWriteArray(), 1, kBlockSize);

    for (int i = 0; i < kBlockSize; ++i)
        REQUIRE (output.getSample (0, i) == Catch::Approx (0.5f));
}
