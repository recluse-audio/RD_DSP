/**
 * Per-variant golden comparison.
 *
 * Loads a HARMONIC_DATA JSON (nlohmann), builds the RAW waveform with WaveFactory, applies the
 * actual WaveFactory scaling functions, and compares against each WAVEFORMS/<VARIANT>/ golden
 * (rapidcsv). One TEST_CASE per shape, checking all four scaling variants.
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

    enum class Variant { Raw, RmsScaled, PeakScaled, RmsPeakScaled };

    const char* variantDir(Variant v)
    {
        switch(v)
        {
            case Variant::Raw:           return "RAW";
            case Variant::RmsScaled:     return "RMS_SCALED";
            case Variant::PeakScaled:    return "PEAK_SCALED";
            case Variant::RmsPeakScaled: return "RMS_PEAK_SCALED";
        }
        return "RAW";
    }

    // Read the HARMONIC_DATA JSON and push its harmonics into the factory; all others muted.
    void setHarmonicsFromJson(rd_dsp::WaveFactory& factory, const std::string& jsonPath)
    {
        std::ifstream jsonFile(jsonPath);
        INFO("JSON path: " << jsonPath);
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

    // Build the RAW waveform for jsonStem, apply the variant's scaling, compare to its golden.
    void checkVariant(const std::string& jsonStem, Variant variant)
    {
        const std::string jsonPath = kGoldenDir + "/HARMONIC_DATA/" + jsonStem + ".json";
        const std::string csvPath  =
            kGoldenDir + "/WAVEFORMS/" + variantDir(variant) + "/" + jsonStem + "_8192.csv";

        rd_dsp::WaveFactory factory;
        rd_dsp::Waveform    waveform;
        waveform.setSize(rd_dsp::kDefaultWaveformSize);

        setHarmonicsFromJson(factory, jsonPath);
        factory.fillWaveformWithHarmonics(waveform);

        switch(variant)
        {
            case Variant::Raw:                                                          break;
            case Variant::RmsScaled:     factory.applyScaleRMS(waveform);               break;
            case Variant::PeakScaled:    factory.applyPeakNormalization(waveform);      break;
            case Variant::RmsPeakScaled: factory.applyScaleRMS(waveform);
                                         factory.applyPeakNormalization(waveform);      break;
        }

        const std::vector<float> golden = loadGoldenRow(csvPath);
        INFO("Golden path: " << csvPath);
        REQUIRE(static_cast<int>(golden.size()) == rd_dsp::kDefaultWaveformSize);

        for(int i = 0; i < rd_dsp::kDefaultWaveformSize; i++)
        {
            INFO("variant " << variantDir(variant) << " sample index " << i);
            REQUIRE(waveform.getSample(i)
                    == Catch::Approx(golden[static_cast<std::size_t>(i)]).margin(1.0e-6));
        }
    }

    void checkAllVariants(const std::string& jsonStem)
    {
        checkVariant(jsonStem, Variant::Raw);
        checkVariant(jsonStem, Variant::RmsScaled);
        checkVariant(jsonStem, Variant::PeakScaled);
        checkVariant(jsonStem, Variant::RmsPeakScaled);
    }
}

TEST_CASE("Sine waveform matches golden across all scaling variants", "[WaveformVariants]")
{
    checkAllVariants("GOLDEN_SineWave_HarmonicData");
}

TEST_CASE("Sawtooth waveform matches golden across all scaling variants", "[WaveformVariants]")
{
    checkAllVariants("GOLDEN_SawWave_HarmonicData_HarmonicCount_16");
}

TEST_CASE("Square waveform matches golden across all scaling variants", "[WaveformVariants]")
{
    checkAllVariants("GOLDEN_SquareWave_HarmonicData_HarmonicCount_8");
}

TEST_CASE("Triangle waveform matches golden across all scaling variants", "[WaveformVariants]")
{
    checkAllVariants("GOLDEN_TriangleWave_HarmonicData_HarmonicCount_8");
}
