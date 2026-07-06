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

namespace
{
    void verifyWaveAgainstGolden (rd_dsp::Waveform::WaveType type,
                                  const std::string& relPath,
                                  int numSamples)
    {
        rd_dsp::Waveform wave;
        wave.setSize (numSamples);
        wave.setWaveType (type);

        const std::string goldenPath = std::string (RD_DSP_TESTS_DIR) + relPath;

        std::vector<std::vector<float>> rows;
        const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/false);
        INFO ("Golden path: " << goldenPath);
        REQUIRE (loaded);
        REQUIRE (rows.size() == 1);
        REQUIRE (static_cast<int> (rows[0].size()) == numSamples);

        const auto& goldenRow = rows[0];
        for (int i = 0; i < numSamples; ++i)
        {
            const float generated = wave.getSample (i);
            const float golden    = goldenRow[static_cast<std::size_t> (i)];

            INFO ("sample index " << i);
            REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
        }
    }
}

TEST_CASE("Waveform sine fill matches golden CSV exactly at N=8192", "[Waveform]")
{
    verifyWaveAgainstGolden (rd_dsp::Waveform::WaveType::wSine,
                             "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SineWave_HarmonicData_8192.csv",
                             8192);
}

TEST_CASE("Waveform triangle fill matches golden CSV exactly at N=8192", "[Waveform]")
{
    verifyWaveAgainstGolden (rd_dsp::Waveform::WaveType::wTri,
                             "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_TriangleWave_HarmonicData_HarmonicCount_8_8192.csv",
                             8192);
}

TEST_CASE("Waveform square fill matches golden CSV exactly at N=8192", "[Waveform]")
{
    verifyWaveAgainstGolden (rd_dsp::Waveform::WaveType::wSquare,
                             "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SquareWave_HarmonicData_HarmonicCount_8_8192.csv",
                             8192);
}

TEST_CASE("Waveform saw fill matches golden CSV exactly at N=8192", "[Waveform]")
{
    verifyWaveAgainstGolden (rd_dsp::Waveform::WaveType::wSaw,
                             "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SawWave_HarmonicData_HarmonicCount_16_8192.csv",
                             8192);
}
