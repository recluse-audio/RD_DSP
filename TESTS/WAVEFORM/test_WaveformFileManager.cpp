/*
* This test coverage is largely a replacement for previous hand rolled attempt
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <memory>
#include <string>
#include <vector>

#include "../../SOURCE/WAVEFORM/WaveFactory.h"
#include "../../SOURCE/WAVEFORM/Waveform.h"
#include "../../SOURCE/WAVEFORM/WaveformFileManager.h"
#include "../../SOURCE/HELPERS/CsvLoader.h"

//========================================================
//============= READING / LOADING CSV ====================
// This chunk of tests focus on going from CSV to Waveform / Wavetable
TEST_CASE("WaveformFileManager loads golden csv" "[rapidcsv]")
{
    // known waveform size of golden waveform
    constexpr int numSamples = 8192;

    // path to single sine wave in 1 row of a csv, considered one source of truth
    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SineWave_HarmonicData_8192.csv";

    //
   rd_dsp::Wavetable waveTable;
   waveTable.clear();
   // this should ensure we are starting with clean slate
   REQUIRE(waveTable.getNumWaveforms() == 0);

   // fill 'waveTable' with golden CSV using waveFileManager
   rd_dsp::WaveformFileManager::fillFromCSV(waveTable, goldenPath);

   // retrieve vector of sample vals from golden
   std::vector<std::vector<float>> rows;
   const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/false);
   REQUIRE (loaded);
   REQUIRE (rows.size() == 1);
   REQUIRE (static_cast<int> (rows[0].size()) == numSamples);

    const auto& goldenRow = rows[0];
    for (int i = 0; i < numSamples; ++i)
    {
        const float loadedSample = waveTable.getSampleAtIndex (i);
        const float golden       = goldenRow[static_cast<std::size_t> (i)];

        INFO ("sample index " << i);
        REQUIRE (loadedSample == Catch::Approx (golden).margin (1.0e-6));
    }
}

TEST_CASE("WaveformFileManager loads golden sine CSV and samples match golden file", "[rapidcsv]")
{
    constexpr int numSamples = 8192;

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SineWave_HarmonicData_8192.csv";

    rd_dsp::Waveform wave;
    const bool waveLoaded = rd_dsp::WaveformFileManager::fillFromCSV (wave, goldenPath);

    INFO ("Golden path: " << goldenPath);
    REQUIRE (waveLoaded);
    REQUIRE (wave.getNumSamples() == numSamples);

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/false);
    REQUIRE (loaded);
    REQUIRE (rows.size() == 1);
    REQUIRE (static_cast<int> (rows[0].size()) == numSamples);

    const auto& goldenRow = rows[0];
    for (int i = 0; i < numSamples; ++i)
    {
        const float loadedSample = wave.getSample (i);
        const float golden       = goldenRow[static_cast<std::size_t> (i)];

        INFO ("sample index " << i);
        REQUIRE (loadedSample == Catch::Approx (golden).margin (1.0e-6));
    }
}

TEST_CASE("WaveformFileManager loads basic waveform table; wavePos=0 returns sine row", "[rapidcsv]")
{
    constexpr int numSamples = 8192;
    constexpr int numWaves = 4;

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8192.csv";

    const std::string goldenSineWaveformPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SineWave_HarmonicData_8192.csv";

    rd_dsp::Wavetable table;
    rd_dsp::WaveformFileManager::fillFromCSV (table, goldenPath);
    rd_dsp::Waveform sineWaveform;
    REQUIRE (rd_dsp::WaveformFileManager::fillFromCSV (sineWaveform, goldenSineWaveformPath));

    INFO ("Golden path: " << goldenPath);
    REQUIRE (table.getNumWaveforms() == numWaves);

    std::vector<std::vector<float>> rows;
    const bool loaded = rd_dsp::CsvLoader::load (goldenPath, rows, /*skipHeader=*/false);
    REQUIRE (loaded);
    REQUIRE (static_cast<int> (rows.size()) == numWaves);
    REQUIRE (static_cast<int> (rows[0].size()) == numSamples);

    //====================================
    // Basic Table at 0.0 is a sine wave
    table.setNormalizedWavePosition (0.0f);

    const auto& sineRow = rows[0];
    for (int i = 0; i < numSamples; ++i)
    {
        const float wavetableSample = table.getSampleAtIndex (static_cast<float> (i));
        const float waveformSample = sineWaveform.getInterpolatedSampleAtIndex (static_cast<float> (i));

        INFO ("sample index " << i);
        REQUIRE (wavetableSample == Catch::Approx (waveformSample).margin (1.0e-6));
    }

    //=====================================
    // Basic Table at 0.25 is a triangle (world wavePos = 1), should match golden triangle
    const std::string goldenTriWaveformPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_TriangleWave_HarmonicData_HarmonicCount_8_8192.csv";

    rd_dsp::Waveform triWaveform;
    REQUIRE (rd_dsp::WaveformFileManager::fillFromCSV (triWaveform, goldenTriWaveformPath));
    table.setNormalizedWavePosition (0.25f);

    for (int i = 0; i < numSamples; ++i)
    {
        const float wavetableSample = table.getSampleAtIndex (static_cast<float> (i));
        const float waveformSample = triWaveform.getInterpolatedSampleAtIndex (static_cast<float> (i));

        INFO ("sample index " << i);
        REQUIRE (wavetableSample == Catch::Approx (waveformSample).margin (1.0e-6));
    }

    //=====================================
    // Basic Table at 0.5 is a square (world wavePos = 2), should match golden square
    const std::string goldenSquareWaveformPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SquareWave_HarmonicData_HarmonicCount_8_8192.csv";

    rd_dsp::Waveform squareWaveform;
    REQUIRE (rd_dsp::WaveformFileManager::fillFromCSV (squareWaveform, goldenSquareWaveformPath));
    table.setNormalizedWavePosition (0.5f);

    for (int i = 0; i < numSamples; ++i)
    {
        const float wavetableSample = table.getSampleAtIndex (static_cast<float> (i));
        const float waveformSample = squareWaveform.getInterpolatedSampleAtIndex (static_cast<float> (i));

        INFO ("sample index " << i);
        REQUIRE (wavetableSample == Catch::Approx (waveformSample).margin (1.0e-6));
    }

    //=====================================
    // Basic Table at 0.75 is a saw (world wavePos = 3), should match golden saw
    const std::string goldenSawWaveformPath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVEFORM/GOLDEN_SawWave_HarmonicData_HarmonicCount_16_8192.csv";

    rd_dsp::Waveform sawWaveform;
    REQUIRE (rd_dsp::WaveformFileManager::fillFromCSV (sawWaveform, goldenSawWaveformPath));
    table.setNormalizedWavePosition (0.75f);

    for (int i = 0; i < numSamples; ++i)
    {
        const float wavetableSample = table.getSampleAtIndex (static_cast<float> (i));
        const float waveformSample = sawWaveform.getInterpolatedSampleAtIndex (static_cast<float> (i));

        INFO ("sample index " << i);
        REQUIRE (wavetableSample == Catch::Approx (waveformSample).margin (1.0e-6));
    }
}

//================ MULTI-ROW CSV to MULTI-WAVE TABLE ================
TEST_CASE("Can load Wavetable from multi-row csv" "[Wavetable][CSV]")
{
    rd_dsp::Wavetable wavetable;
    wavetable.clear();
    REQUIRE(wavetable.getNumWaveforms() == 0);

    const std::string goldenTablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8192.csv";

    rd_dsp::WaveformFileManager::fillFromCSV(wavetable, goldenTablePath);

    // basic tables are sine/tri/square/saw
    REQUIRE(wavetable.getNumWaveforms() == 4);

}

//====================================================================
//============= WRITING WAVEFORM/WAVETABLE TO CSV ====================
//
TEST_CASE("Can write single waveform to CSV" )
{
    // Create a waveform to write to csv using Waveform's builtin default waves
    // Write that waveform to csv
    // load the newly written csv into new waveform
    // compare the two waveforms
    int numSamples = 8192;
    rd_dsp::Waveform waveform;
    waveform.setSize(8192);
    waveform.setWaveType(rd_dsp::Waveform::WaveType::wSine);

    const std::string outputPath = std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/OUTPUT/WaveformFileManager_WriteWaveformToCSV_Output.csv";

    rd_dsp::WaveformFileManager::writeWaveformToCSV(waveform, outputPath);

    rd_dsp::Waveform csvWaveform;
    REQUIRE(rd_dsp::WaveformFileManager::fillFromCSV(csvWaveform, outputPath));

    REQUIRE(csvWaveform.getNumSamples() == waveform.getNumSamples());

}

TEST_CASE("Can create csv row aka vector of doubles from waveform amp values" "[write_waveforms]" )
{
    int numSamples = 8192;
    rd_dsp::Waveform waveform;
    waveform.setSize(8192);
    waveform.setWaveType(rd_dsp::Waveform::WaveType::wSine);

    // declare vector of doubles same num samples as waveform
    std::vector<double> row(8192);

    // fill row with sample values (hopefully)
    rd_dsp::WaveformFileManager::fillRowFromWaveform(waveform, row);

    // check them, expecting the same values
    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
    {
        auto waveformSample = waveform.getSample(sampleIndex);
        auto rowSample = row[sampleIndex];
        REQUIRE(waveformSample == rowSample);
    }
}

TEST_CASE("Can write Wavetable to CSV file with 1 row / waveform" "[Wavetable][CSV]")
{
    const int numSamples = 8192;
    rd_dsp::Wavetable wavetable;
    // clear/fill with basic shapes to ensure test conditions
    wavetable.clear();
    wavetable.fillWithBasicShapes(numSamples);

    const std::string csvPath = std::string(RD_DSP_TESTS_DIR) +"/WAVEFORM/OUTPUT/WaveformFileManager_WriteWavetableToCSV_Output.csv";
    rd_dsp::WaveformFileManager::writeWavetableToCSV(wavetable, csvPath);

    // make a wave table, clear, then fill from csv
    // expect this to match wavetable above that wrote to csv
    rd_dsp::Wavetable wavetableFromCSV;
    wavetableFromCSV.clear();
    REQUIRE(wavetableFromCSV.getNumWaveforms() == 0); // just cleared it

    // fill with csv we just wrote
    rd_dsp::WaveformFileManager::fillFromCSV(wavetableFromCSV, csvPath);

    // should have more waveforms now
    REQUIRE(wavetableFromCSV.getNumWaveforms() == 4);
}
