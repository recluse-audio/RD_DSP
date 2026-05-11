/**
 * Created on 2026-05-07 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>
#include <string>
#include <vector>

#include "WAVEFORM/Waveform.h"
#include "HELPERS/CsvLoader.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

TEST_CASE("Waveform sine fill matches golden CSV exactly at N=8096", "[Waveform]")
{
    constexpr int numSamples = 8096;

    rd_dsp::Waveform wave;
    wave.setSize (numSamples);
    wave.setWaveType (rd_dsp::Waveform::WaveType::wSine);

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/SINE/GOLDEN_SINE_8096.csv";

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp_tests::loadCsv (goldenPath, rows, /*skipHeader=*/true);
    INFO ("Golden path: " << goldenPath);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numSamples);

    // Columns: index, normalized_phase, pi_radians, amplitude
    constexpr int kAmplitudeCol = 3;

    for (int i = 0; i < numSamples; ++i)
    {
        REQUIRE (static_cast<int> (rows[i].size()) > kAmplitudeCol);
        const float generated = wave.getSample (i);
        const float golden    = rows[i][kAmplitudeCol];

        INFO ("sample index " << i);
        REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
    }
}

TEST_CASE("Waveform triangle fill matches golden CSV exactly at N=8096", "[Waveform]")
{
    constexpr int numSamples = 8096;

    rd_dsp::Waveform wave;
    wave.setSize (numSamples);
    wave.setWaveType (rd_dsp::Waveform::WaveType::wTri);

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/TRIANGLE/GOLDEN_TRIANGLE_8096.csv";

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp_tests::loadCsv (goldenPath, rows, /*skipHeader=*/true);
    INFO ("Golden path: " << goldenPath);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numSamples);

    // Columns: index, normalized_phase, pi_radians, amplitude
    constexpr int kAmplitudeCol = 3;

    for (int i = 0; i < numSamples; ++i)
    {
        REQUIRE (static_cast<int> (rows[i].size()) > kAmplitudeCol);
        const float generated = wave.getSample (i);
        const float golden    = rows[i][kAmplitudeCol];

        INFO ("sample index " << i);
        REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
    }
}

TEST_CASE("Waveform square fill matches golden CSV exactly at N=8096", "[Waveform]")
{
    constexpr int numSamples = 8096;

    rd_dsp::Waveform wave;
    wave.setSize (numSamples);
    wave.setWaveType (rd_dsp::Waveform::WaveType::wSquare);

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/SQUARE/GOLDEN_SQUARE_8096.csv";

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp_tests::loadCsv (goldenPath, rows, /*skipHeader=*/true);
    INFO ("Golden path: " << goldenPath);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numSamples);

    constexpr int kAmplitudeCol = 3;

    for (int i = 0; i < numSamples; ++i)
    {
        REQUIRE (static_cast<int> (rows[i].size()) > kAmplitudeCol);
        const float generated = wave.getSample (i);
        const float golden    = rows[i][kAmplitudeCol];

        INFO ("sample index " << i);
        REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
    }
}

TEST_CASE("Waveform saw fill matches golden CSV exactly at N=8096", "[Waveform]")
{
    constexpr int numSamples = 8096;

    rd_dsp::Waveform wave;
    wave.setSize (numSamples);
    wave.setWaveType (rd_dsp::Waveform::WaveType::wSaw);

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/SAW/GOLDEN_SAW_8096.csv";

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp_tests::loadCsv (goldenPath, rows, /*skipHeader=*/true);
    INFO ("Golden path: " << goldenPath);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numSamples);

    constexpr int kAmplitudeCol = 3;

    for (int i = 0; i < numSamples; ++i)
    {
        REQUIRE (static_cast<int> (rows[i].size()) > kAmplitudeCol);
        const float generated = wave.getSample (i);
        const float golden    = rows[i][kAmplitudeCol];

        INFO ("sample index " << i);
        REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
    }
}
