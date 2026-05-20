/**
 * Created on 2026-05-11 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>
#include <memory>
#include <vector>

#include "WAVEFORM/Wavetable.h"
#include "WAVEFORM/Waveform.h"

TEST_CASE("Wavetable constructs", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    (void) wt;
}

TEST_CASE("Wavetable can be assigned waveforms", "[Wavetable]")
{
    rd_dsp::Wavetable wt;

    auto wave = std::make_unique<rd_dsp::Waveform>();
    wave->setSize (256);
    wave->setWaveType (rd_dsp::Waveform::WaveType::wSine);

    REQUIRE (wt.getNumWaveforms() == 4); // default size

    wt.addWaveform (std::move (wave));

    REQUIRE (wt.getNumWaveforms() == 5);
}

namespace
{
    constexpr float kTwoPi = 6.28318530717958647692f;

    std::unique_ptr<rd_dsp::Waveform> makeSine (int n)
    {
        auto w = std::make_unique<rd_dsp::Waveform>();
        w->setSize (n);
        w->setWaveType (rd_dsp::Waveform::WaveType::wSine);
        return w;
    }

    std::unique_ptr<rd_dsp::Waveform> makeSaw (int n)
    {
        auto w = std::make_unique<rd_dsp::Waveform>();
        w->setSize (n);
        w->setWaveType (rd_dsp::Waveform::WaveType::wSaw);
        return w;
    }
}

TEST_CASE("fillDisplayBuffer no-op when wavetable empty", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();

    constexpr int kN = 64;
    constexpr float kSentinel = -999.0f;
    std::vector<float> out (kN, kSentinel);

    wt.fillDisplayBuffer (out.data(), kN);

    for (int i = 0; i < kN; ++i)
        REQUIRE (out[i] == kSentinel);
}

TEST_CASE("fillDisplayBuffer no-op when outSize <= 0", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    constexpr float kSentinel = -999.0f;
    float out = kSentinel;

    wt.fillDisplayBuffer (&out, 0);
    REQUIRE (out == kSentinel);

    wt.fillDisplayBuffer (&out, -10);
    REQUIRE (out == kSentinel);
}

TEST_CASE("fillDisplayBuffer single sine, outSize == waveformSize", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();
    constexpr int kN = 2048;
    wt.addWaveform (makeSine (kN));

    std::vector<float> out (kN, 0.0f);
    wt.fillDisplayBuffer (out.data(), kN);

    for (int i = 0; i < kN; ++i)
    {
        const float expected = std::sin ((static_cast<float>(i) * kTwoPi) / static_cast<float>(kN));
        REQUIRE (out[i] == Catch::Approx (expected).margin (1.0e-5));
    }
}

TEST_CASE("fillDisplayBuffer single sine, decimated by 16", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();
    constexpr int kN       = 2048;
    constexpr int kOutSize = kN / 16; // 128
    wt.addWaveform (makeSine (kN));

    std::vector<float> out (kOutSize, 0.0f);
    wt.fillDisplayBuffer (out.data(), kOutSize);

    // step = N/outSize = 16. out[i] should equal sine at integer index i*16.
    for (int i = 0; i < kOutSize; ++i)
    {
        const int srcIndex = i * 16;
        const float expected = std::sin ((static_cast<float>(srcIndex) * kTwoPi) / static_cast<float>(kN));
        REQUIRE (out[i] == Catch::Approx (expected).margin (1.0e-5));
    }
}

TEST_CASE("fillDisplayBuffer interpolates between two waveforms", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();
    constexpr int kN = 512;
    wt.addWaveform (makeSine (kN));
    wt.addWaveform (makeSaw  (kN));

    // Reference: read each underlying waveform directly via a one-shot
    // Wavetable holding just that waveform, so the test stays decoupled from
    // Waveform internals.
    rd_dsp::Wavetable sineOnly; sineOnly.clear(); sineOnly.addWaveform (makeSine (kN));
    rd_dsp::Wavetable sawOnly;  sawOnly.clear();  sawOnly.addWaveform  (makeSaw  (kN));

    std::vector<float> sineRef (kN, 0.0f);
    std::vector<float> sawRef  (kN, 0.0f);
    sineOnly.fillDisplayBuffer (sineRef.data(), kN);
    sawOnly .fillDisplayBuffer (sawRef .data(), kN);

    std::vector<float> out (kN, 0.0f);

    SECTION ("pos = 0.0 -> sine only")
    {
        wt.setNormalizedWavePosition (0.0f);
        wt.fillDisplayBuffer (out.data(), kN);
        for (int i = 0; i < kN; ++i)
            REQUIRE (out[i] == Catch::Approx (sineRef[i]).margin (1.0e-5));
    }

    SECTION ("pos = 1.0 -> saw only (clamped)")
    {
        wt.setNormalizedWavePosition (1.0f);
        wt.fillDisplayBuffer (out.data(), kN);
        for (int i = 0; i < kN; ++i)
            REQUIRE (out[i] == Catch::Approx (sawRef[i]).margin (1.0e-5));
    }

    SECTION ("pos = 0.25 -> 50/50 mix (worldWavePos = 0.5)")
    {
        wt.setNormalizedWavePosition (0.25f);
        wt.fillDisplayBuffer (out.data(), kN);
        for (int i = 0; i < kN; ++i)
        {
            const float expected = 0.5f * sineRef[i] + 0.5f * sawRef[i];
            REQUIRE (out[i] == Catch::Approx (expected).margin (1.0e-5));
        }
    }
}

TEST_CASE("fillDisplayBufferAveraged no-op when wavetable empty", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();

    constexpr int kN = 64;
    constexpr float kSentinel = -999.0f;
    std::vector<float> out (kN, kSentinel);

    wt.fillDisplayBufferAveraged (out.data(), kN);

    for (int i = 0; i < kN; ++i)
        REQUIRE (out[i] == kSentinel);
}

TEST_CASE("fillDisplayBufferAveraged point-samples when step <= 1", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();
    constexpr int kN = 512;
    wt.addWaveform (makeSine (kN));

    std::vector<float> out (kN, 0.0f);
    wt.fillDisplayBufferAveraged (out.data(), kN);

    for (int i = 0; i < kN; ++i)
    {
        const float expected = std::sin ((static_cast<float>(i) * kTwoPi) / static_cast<float>(kN));
        REQUIRE (out[i] == Catch::Approx (expected).margin (1.0e-5));
    }
}

TEST_CASE("fillDisplayBufferAveraged box-averages decimated bins", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    wt.clear();
    constexpr int kN       = 2048;
    constexpr int kOutSize = kN / 16; // 128, step = 16
    wt.addWaveform (makeSine (kN));

    std::vector<float> out (kOutSize, 0.0f);
    wt.fillDisplayBufferAveraged (out.data(), kOutSize);

    // Each output bin = mean of 16 consecutive source samples.
    for (int i = 0; i < kOutSize; ++i)
    {
        const int start = i * 16;
        float expected = 0.f;
        for (int s = start; s < start + 16; ++s)
            expected += std::sin ((static_cast<float>(s) * kTwoPi) / static_cast<float>(kN));
        expected /= 16.f;

        REQUIRE (out[i] == Catch::Approx (expected).margin (1.0e-5));
    }
}