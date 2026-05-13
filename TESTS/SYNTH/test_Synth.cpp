/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include "SYNTH/Synth.h"

TEST_CASE ("Synth::getEngineName returns rd_dsp::Synth", "[Synth]")
{
    rd_dsp::Synth synth;
    REQUIRE (synth.getEngineName() == "rd_dsp::Synth");
}

TEST_CASE ("Synth::setNumVoices adjusts voice count", "[Synth]")
{
    rd_dsp::Synth synth;

    // ctor seeds with 1 voice
    REQUIRE (synth.getNumVoices() == 1);

    synth.setNumVoices (5);
    CHECK (synth.getNumVoices() == 5);

    synth.setNumVoices (10); // upper bound
    CHECK (synth.getNumVoices() == 10);

    synth.setNumVoices (0); // lower bound
    CHECK (synth.getNumVoices() == 0);

    // out-of-range values are ignored; count must not change
    synth.setNumVoices (3);
    REQUIRE (synth.getNumVoices() == 3);

    synth.setNumVoices (-1);
    CHECK (synth.getNumVoices() == 3);

    synth.setNumVoices (11);
    CHECK (synth.getNumVoices() == 3);
}
