/**
 * Per-variant golden comparison.
 *
 * Loads a HARMONIC_DATA JSON (nlohmann), builds the RAW waveform with WaveFactory, applies the
 * actual WaveFactory scaling functions, and compares against each WAVEFORMS/<VARIANT>/ golden
 * (rapidcsv). One SECTION per scaling variant.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <nlohmann/json.hpp>
#include <rapidcsv.h>

#include <fstream>
#include <string>
#include <vector>

#include "WAVEFORM/WaveFactory.h"
#include "WAVEFORM/Waveform.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

namespace
{
    const std::string kGoldenDir = std::string(RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN";

    // Read the HARMONIC_DATA JSON and push its harmonics into the factory; all others muted.
    void setHarmonicsFromJson(rd_dsp::WaveFactory& factory, const std::string& jsonPath)
    {
        std::ifstream jsonFile(jsonPath);
        REQUIRE(jsonFile.is_open());

        nlohmann::json root;
        jsonFile >> root;

        // Start from silence: every harmonic muted, ratio kept sane.
        for(int harmonicIndex = 0; harmonicIndex < rd_dsp::kMaxAudioFriendlyHarmonics; harmonicIndex++)
            factory.setHarmonicDataValues(harmonicIndex, 0.f, 0.f, static_cast<float>(harmonicIndex + 1));

        for(const auto& harmonic : root["HarmonicData"])
        {
            const int   harmonicIndex = harmonic["harmonicIndex"].get<int>();
            const float ratio         = harmonic["ratio"].get<float>();
            const float gain          = harmonic["gain"].get<float>();
            const float phaseOffset   = harmonic.value("phaseOffset", 0.0f);
            factory.setHarmonicDataValues(harmonicIndex, gain, phaseOffset, ratio);
        }
    }

    // Single-row golden CSV -> vector<float>.
    std::vector<float> loadGoldenRow(const std::string& csvPath)
    {
        rapidcsv::Document doc(csvPath, rapidcsv::LabelParams(-1, -1));
        return doc.GetRow<float>(0);
    }

    void compareToGolden(const rd_dsp::Waveform& waveform, const std::string& csvPath)
    {
        const std::vector<float> golden = loadGoldenRow(csvPath);
        INFO("Golden path: " << csvPath);
        REQUIRE(static_cast<int>(golden.size()) == rd_dsp::kDefaultWaveformSize);

        for(int i = 0; i < rd_dsp::kDefaultWaveformSize; i++)
        {
            INFO("sample index " << i);
            REQUIRE(waveform.getSample(i)
                    == Catch::Approx(golden[static_cast<std::size_t>(i)]).margin(1.0e-6));
        }
    }
}

TEST_CASE("Sine waveform matches golden across all scaling variants", "[WaveformVariants]")
{
    const std::string jsonPath = kGoldenDir + "/HARMONIC_DATA/GOLDEN_SineWave_HarmonicData.json";
    const std::string csvName  = "GOLDEN_SineWave_HarmonicData_8192.csv";

    rd_dsp::WaveFactory factory;
    rd_dsp::Waveform    waveform;
    waveform.setSize(rd_dsp::kDefaultWaveformSize);
    setHarmonicsFromJson(factory, jsonPath);

    SECTION("RAW")
    {
        factory.fillWaveformWithHarmonics(waveform);
        compareToGolden(waveform, kGoldenDir + "/WAVEFORMS/RAW/" + csvName);
    }
    SECTION("RMS_SCALED")
    {
        factory.fillWaveformWithHarmonics(waveform);
        factory.rmsScale(waveform);
        compareToGolden(waveform, kGoldenDir + "/WAVEFORMS/RMS_SCALED/" + csvName);
    }
    SECTION("PEAK_SCALED")
    {
        factory.fillWaveformWithHarmonics(waveform);
        factory.peakScale(waveform);
        compareToGolden(waveform, kGoldenDir + "/WAVEFORMS/PEAK_SCALED/" + csvName);
    }
    SECTION("RMS_PEAK_SCALED")
    {
        factory.fillWaveformWithHarmonics(waveform);
        factory.rmsScale(waveform);
        factory.peakScale(waveform);
        compareToGolden(waveform, kGoldenDir + "/WAVEFORMS/RMS_PEAK_SCALED/" + csvName);
    }
}
