/**
 * Created on 2026-05-07 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>
#include <string>
#include <vector>

#include "WAVEFORM/WaveformBufferFiller.h"
#include "RD_BUFFER/RD_Buffer.h"
#include "HELPERS/CsvLoader.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

// Bridges test code to private fill helpers via friendship.
// Must live in rd_dsp namespace to match friend decl in WaveformBufferFiller.h.
namespace rd_dsp
{
class WaveformBufferTester
{
public:
    static void fillWithSine (WaveformBufferFiller& filler, RD_Buffer& buffer)
    {
        filler._fillWithSine (buffer);
    }
};
} // namespace rd_dsp

using namespace rd_dsp;

TEST_CASE("Sine fill matches golden CSV exactly at N=8096", "[WaveformBufferFiller]")
{
    constexpr int numSamples = 8096;

    RD_Buffer buffer (1, numSamples);
    WaveformBufferFiller filler;
    WaveformBufferTester::fillWithSine (filler, buffer);

    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/GOLDEN/SINE/GOLDEN_SINE_8096.csv";

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
        const float generated = buffer.getSample (0, i);
        const float golden    = rows[i][kAmplitudeCol];

        INFO ("sample index " << i);
        REQUIRE (generated == Catch::Approx (golden).margin (1.0e-6));
    }
}
