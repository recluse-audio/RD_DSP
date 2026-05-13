/**
 * Created on 2026-05-11 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <memory>
#include <string>
#include <vector>

#include "WAVEFORM/WaveFactory.h"
#include "WAVEFORM/Waveform.h"
#include "HELPERS/CsvLoader.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

TEST_CASE("WaveFactory loads golden sine CSV and samples match golden file", "[WaveFactory]")
{
    constexpr int numSamples = 8096;

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/SINE/GOLDEN_SINE_8096.csv";

    rd_dsp::WaveFactory factory;
    auto wave = factory.loadWaveformFromCSV (goldenPath);

    INFO ("Golden path: " << goldenPath);
    REQUIRE (wave != nullptr);
    REQUIRE (wave->getNumSamples() == numSamples);

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/true);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numSamples);

    constexpr int kAmplitudeCol = 3;

    for (int i = 0; i < numSamples; ++i)
    {
        REQUIRE (static_cast<int> (rows[i].size()) > kAmplitudeCol);
        const float loadedSample = wave->getSample (i);
        const float golden       = rows[i][kAmplitudeCol];

        INFO ("sample index " << i);
        REQUIRE (loadedSample == Catch::Approx (golden).margin (1.0e-6));
    }
}

TEST_CASE("WaveFactory loads basic waveform table; wavePos=0 returns sine row", "[WaveFactory]")
{
    constexpr int numSamples = 8096;
    constexpr int numWaves = 4;

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";

    rd_dsp::WaveFactory factory;
    auto table = factory.loadWavetableFromCSV (goldenPath);

    INFO ("Golden path: " << goldenPath);
    REQUIRE (table != nullptr);
    REQUIRE (table->getNumWaveforms() == numWaves);

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/false);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numWaves);
    REQUIRE (static_cast<int> (rows[0].size()) == numSamples);

    table->setNormalizedWavePosition (0.0f);

    const auto& sineRow = rows[0];
    for (int i = 0; i < numSamples; ++i)
    {
        const float got = table->getSampleAtIndex (static_cast<float> (i));
        const float golden = sineRow[static_cast<std::size_t> (i)];

        INFO ("sample index " << i);
        REQUIRE (got == Catch::Approx (golden).margin (1.0e-6));
    }
}
