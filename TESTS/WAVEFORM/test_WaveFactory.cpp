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

TEST_CASE("WaveFactory constructs with default num harmonic data in member array""[WaveFactory][HarmonicData]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;

    auto fundamentalData = waveFactory.getHarmonicData(0);
    REQUIRE(fundamentalData->ratio == 1.f);

    // without any args given, WaveFactory creates HarmonicData with only
    // fundamental with gain above 0.f
    REQUIRE(fundamentalData->gain > 0.f);

    for(int harmonicIndex = 1; harmonicIndex < expectedNumHarmonics; harmonicIndex++)
    {
        auto harmonicData = waveFactory.getHarmonicData(harmonicIndex);
        REQUIRE(harmonicData->gain == 0.f);
        REQUIRE(harmonicData->ratio == (float)(harmonicIndex+1));
    }

}


TEST_CASE("WaveFactory allows setting of specific HarmonicData""[WaveFactory][HarmonicData]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;

    // return false if attempting something out of range
    bool canSetOutOfRangeHarmonicDataValues = waveFactory.setHarmonicDataValues(expectedNumHarmonics+1, 0.f, 0.f, 1.f);
    REQUIRE(!canSetOutOfRangeHarmonicDataValues);

    const int harmonicToTest = 1;
    const float gainTestValue = 0.75f; // value we will set 1st harmonic to
    bool success = waveFactory.setHarmonicDataValues(harmonicToTest, gainTestValue, 0.f, (float)(harmonicToTest + 1));
    REQUIRE(success);

    auto harmonicData = waveFactory.getHarmonicData(harmonicToTest);
    REQUIRE(harmonicData->gain == gainTestValue);

}


TEST_CASE("WaveFactory can fill a waveform with harmonics as expected""[WaveFactory]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;
    rd_dsp::Waveform waveForm;
    waveForm.setSize(rd_dsp::kDefaultWaveformSize);

    // set fundamental to 1.f gain, no other harmonics active
    waveFactory.setHarmonicDataValues(0, 1.f, 0.f, 1.f);

    // fundamental-only harmonic fill is a plain sine, matches golden sine file
    waveFactory.fillWaveformWithHarmonics(waveForm);

    const std::string goldenPath =
        std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/SINE/GOLDEN_SINE_8192.csv";

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp::CsvLoader::load(goldenPath, rows, /*skipHeader=*/false);
    INFO("Golden path: " << goldenPath);
    REQUIRE(loaded);
    REQUIRE(rows.size() == 1);
    REQUIRE(static_cast<int>(rows[0].size()) == rd_dsp::kDefaultWaveformSize);

    const auto& goldenRow = rows[0];
    for(int i = 0; i < rd_dsp::kDefaultWaveformSize; i++)
    {
        const float generated = waveForm.getSample(i);
        const float golden = goldenRow[static_cast<std::size_t>(i)];

        INFO("sample index " << i);
        REQUIRE(generated == Catch::Approx(golden).margin(1.0e-6));
    }
}
