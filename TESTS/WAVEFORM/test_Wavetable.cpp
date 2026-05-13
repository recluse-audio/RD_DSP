/**
 * Created on 2026-05-11 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "WAVEFORM/Wavetable.h"
#include "WAVEFORM/Waveform.h"

TEST_CASE("Wavetable constructs", "[Wavetable]")
{
    rd_dsp::Wavetable wt;
    (void) wt;
}

TEST_CASE("Wavetable can be assigned waveforms", "[Wavetable]")
{
    rd_dsp::Wavetable wt;

    auto wave = std::make_unique<rd_dsp::Waveform>();
    wave->setSize (256);
    wave->setWaveType (rd_dsp::Waveform::WaveType::wSine);

    REQUIRE (wt.getNumWaveforms() == 4); // default size

    wt.addWaveform (std::move (wave));

    REQUIRE (wt.getNumWaveforms() == 5);
}