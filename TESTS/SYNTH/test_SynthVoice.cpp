/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include "SYNTH/SynthVoice.h"
#include "WAVEFORM/Wavetable.h"

TEST_CASE ("SynthVoice::getEngineName returns rd_dsp::SynthVoice", "[SynthVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::SynthVoice voice (wavetable);
    REQUIRE (voice.getEngineName() == "rd_dsp::SynthVoice");
}
