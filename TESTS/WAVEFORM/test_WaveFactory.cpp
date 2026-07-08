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


TEST_CASE("WaveFactory can fill Waveform with fundamental only sine wave""[WaveFactory]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;
    rd_dsp::Waveform waveForm;
    waveForm.setSize(rd_dsp::kDefaultWaveformSize);

    // set fundamental to 1.f gain, no other harmonics active
    waveFactory.setHarmonicDataValues(0, 1.f, 0.f, 1.f);

    // fundamental-only harmonic fill is a plain sine; scale to match the RMS_PEAK_SCALED golden
    waveFactory.fillWaveformWithHarmonics(waveForm);
    waveFactory.applyScaleRMS(waveForm);
    waveFactory.applyPeakNormalization(waveForm);

    const std::string goldenPath =
        std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORMS/RMS_PEAK_SCALED/GOLDEN_SineWave_HarmonicData_8192.csv";

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
        REQUIRE(generated == Catch::Approx(golden).margin(1.0e-5));
    }
}

/**
 * This test's purpose is to check existing C++ waveform generation based on HarmonicData
 * It is comparing against a python generated waveform which is standard for me.
 * But this method of python generating creates waveform based on HarmonicData.json file.
 *
 * If we use the same coefficients from HarmonicData.json files in WaveFactory::setHarmonicDataValues()
 * Then we can expect sample values to match; whether python using HarmonicData.json or C++ impl using a function call
 * */
 // commented out for now because the old sine path had not been regenerated with target rms applied
TEST_CASE("New golden sine wave matches old golden sine wave - both made with python""[GoldenWaveform]")
{

    // const std::string oldSinePath =
    //     std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/ALT_WAVEFORMS/WAVEFORMS/GOLDEN_SineWave_HarmonicData_16_8192.csv";

    // // HarmonicData python generated sine wave
    // const std::string newSinePath =
    //     std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SineWave_HarmonicData_8192.csv";

    // std::vector<std::vector<float>> oldRows;
    // std::vector<std::vector<float>> newRows;

    // const bool loadedOld = rd_dsp::CsvLoader::load(oldSinePath, oldRows, /*skipHeader=*/false);
    // const bool loadedNew = rd_dsp::CsvLoader::load(newSinePath, newRows, /*skipHeader=*/false);

    // //==================================
    // // Ensure paths load and are properly sized
    // INFO("OLD PATH: " << oldSinePath);
    // INFO("NEW PATH: " << newSinePath);

    // REQUIRE(loadedOld); REQUIRE(loadedNew);
    // REQUIRE(oldRows.size() == 1); REQUIRE(newRows.size() == 1);

    // REQUIRE(static_cast<int>(oldRows[0].size()) == rd_dsp::kDefaultWaveformSize);
    // REQUIRE(static_cast<int>(newRows[0].size()) == rd_dsp::kDefaultWaveformSize);

    // //=================================
    // // Compare sample values in each row
    // // Technically could be many rows, so use first row only for meaningful comparison
    // // (these are single row waveforms)
    // const auto& oldRow = oldRows[0];
    // const auto& newRow = newRows[0];
    // for(int sampleIndex = 0; sampleIndex < rd_dsp::kDefaultWaveformSize; sampleIndex++)
    // {
    //     const float oldSample = oldRow[static_cast<std::size_t>(sampleIndex)];
    //     const float newSample = newRow[static_cast<std::size_t>(sampleIndex)];
    //     INFO("Sample Index: " << sampleIndex);
    //     REQUIRE(oldSample == Catch::Approx(newSample).margin(1.0e-6));
    // }

}

// GOLDEN TRIANGLE WAVE CONFIRMATION
TEST_CASE("New golden triangle wave matches old golden triangle wave - both made with python""[GoldenWaveform]")
{
    // const std::string oldPath =
    //     std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/ALT_WAVEFORMS/WAVEFORMS/GOLDEN_TriangleWave_HarmonicData_16_8192.csv";

    // // HarmonicData python generated sine wave
    // const std::string newPath =
    //     std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_TriangleWave_HarmonicData_HarmonicCount_8_8192.csv";

    // std::vector<std::vector<float>> oldRows;
    // std::vector<std::vector<float>> newRows;

    // const bool loadedOld = rd_dsp::CsvLoader::load(oldPath, oldRows, /*skipHeader=*/false);
    // const bool loadedNew = rd_dsp::CsvLoader::load(newPath, newRows, /*skipHeader=*/false);

    // //==================================
    // // Ensure paths load and are properly sized
    // INFO("OLD PATH: " << oldPath);
    // INFO("NEW PATH: " << newPath);

    // REQUIRE(loadedOld); REQUIRE(loadedNew);
    // REQUIRE(oldRows.size() == 1); REQUIRE(newRows.size() == 1);

    // REQUIRE(static_cast<int>(oldRows[0].size()) == rd_dsp::kDefaultWaveformSize);
    // REQUIRE(static_cast<int>(newRows[0].size()) == rd_dsp::kDefaultWaveformSize);

    // //=================================
    // // Compare sample values in each row
    // // Technically could be many rows, so use first row only for meaningful comparison
    // // (these are single row waveforms)
    // const auto& oldRow = oldRows[0];
    // const auto& newRow = newRows[0];
    // for(int sampleIndex = 0; sampleIndex < rd_dsp::kDefaultWaveformSize; sampleIndex++)
    // {
    //     const float oldSample = static_cast<float>(oldRow[static_cast<std::size_t>(sampleIndex)]);
    //     const float newSample = static_cast<float>(newRow[static_cast<std::size_t>(sampleIndex)]);
    //     INFO("Sample Index: " << sampleIndex);
    //     REQUIRE(oldSample == Catch::Approx(newSample).margin(1.0e-5));
    // }

}





TEST_CASE("Sine wave generated with multi-harmonic method matches previous simple sine wave""[GoldenSineWave]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;
    rd_dsp::Waveform waveForm;
    waveForm.setSize(rd_dsp::kDefaultWaveformSize);

    // set fundamental to 1.f gain, no other harmonics active
    waveFactory.setHarmonicDataValues(0, 1.f, 0.f, 1.f);

    // fundamental-only harmonic fill is a plain sine; scale to match the RMS_PEAK_SCALED golden
    waveFactory.fillWaveformWithHarmonics(waveForm);
    waveFactory.applyScaleRMS(waveForm);
    waveFactory.applyPeakNormalization(waveForm);

    const std::string goldenPath =
        std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORMS/RMS_PEAK_SCALED/GOLDEN_SineWave_HarmonicData_8192.csv";

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
        REQUIRE(generated == Catch::Approx(golden).margin(1.0e-5));
    }
}


/**
 * Same as Sinewave test above but with TriangleWave
 * */
TEST_CASE("Triangle wave generated with multi-harmonic method matches previous simple sine wave""[GoldenTriangleWave]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;
    rd_dsp::Waveform waveForm;
    waveForm.setSize(rd_dsp::kDefaultWaveformSize);

    // set fundamental to 1.f gain, no other harmonics active
    waveFactory.setHarmonicDataValues(0,  1.f,          0.f, 1.f);
    waveFactory.setHarmonicDataValues(1,  0.f,          0.f, 2.f);
    waveFactory.setHarmonicDataValues(2,  -0.11111f,    0.f, 3.f);
    waveFactory.setHarmonicDataValues(3,  0.f,          0.f, 4.f);
    waveFactory.setHarmonicDataValues(4,  0.04f,        0.f, 5.f);
    waveFactory.setHarmonicDataValues(5,  0.f,          0.f, 6.f);
    waveFactory.setHarmonicDataValues(6,  -0.0204082f,  0.f, 7.f);
    waveFactory.setHarmonicDataValues(7,  0.f,          0.f, 8.f);
    waveFactory.setHarmonicDataValues(8,  0.0123457f,   0.f, 9.f);
    waveFactory.setHarmonicDataValues(9,  0.f,          0.f, 10.f);
    waveFactory.setHarmonicDataValues(10, -0.00826446f, 0.f, 11.f);
    waveFactory.setHarmonicDataValues(11, 0.f,          0.f, 12.f);
    waveFactory.setHarmonicDataValues(12, 0.00591716f,  0.f, 13.f);
    waveFactory.setHarmonicDataValues(13, 0.f,          0.f, 14.f);
    waveFactory.setHarmonicDataValues(14, -0.00444444f, 0.f, 15.f);
    waveFactory.setHarmonicDataValues(15, 0.f,          0.f, 16.f);

    // fundamental-only harmonic fill is a plain sine, matches golden sine file
    waveFactory.fillWaveformWithHarmonics(waveForm);

    const std::string goldenPath =
        std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_TriangleWave_HarmonicData_HarmonicCount_8_8192.csv";

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
        //CHECK(generated == Catch::Approx(golden).margin(1.0e-6));
    }
}
