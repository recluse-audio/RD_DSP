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

TEST_CASE("WaveFactory constructs with default num harmonic data in member array""[WaveFactory][WaveformData]")
{
    const int expectedNumHarmonics = rd_dsp::kMaxAudioFriendlyHarmonics;
    REQUIRE(expectedNumHarmonics == 16);
    rd_dsp::WaveFactory waveFactory;

    auto fundamentalData = waveFactory.getHarmonicData(0);
    REQUIRE(fundamentalData->harmonic == 0);

    // without any args given, WaveFactory creates HarmonicData with only
    // fundamental with gain above 0.f
    REQUIRE(fundamentalData->gain > 0.f);

}
