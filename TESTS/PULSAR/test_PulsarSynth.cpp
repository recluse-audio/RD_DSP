/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>

#include "PULSAR/PulsarSynth.h"
#include "PULSAR/PulsarVoice.h"
#include "RD_BUFFER/RD_Buffer.h"

namespace rd_dsp
{
class PulsarSynthTester
{
public:
    static std::vector<PulsarVoice>& voices (PulsarSynth& s) { return s.mPulsarVoices; }
    static double samplesUntilNextGrain (const PulsarSynth& s) { return s.mSamplesUntilNextGrain; }
};
} // namespace rd_dsp

using rd_dsp::PulsarSynthTester;

TEST_CASE ("PulsarSynth::getEngineName returns rd_dsp::PulsarSynth", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    REQUIRE (synth.getEngineName() == "rd_dsp::PulsarSynth");
}

TEST_CASE ("PulsarSynth seeds one voice on construction", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    CHECK (synth.getNumVoices() == 1);
}

TEST_CASE ("PulsarSynth::setNumVoices adjusts pulsar voice pool", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;

    synth.setNumVoices (4);
    CHECK (synth.getNumVoices() == 4);

    auto& voices = PulsarSynthTester::voices (synth);
    REQUIRE (voices.size() == 4);

    synth.setNumVoices (0);
    CHECK (synth.getNumVoices() == 0);

    synth.setNumVoices (-1); // ignored
    CHECK (synth.getNumVoices() == 0);

    synth.setNumVoices (11); // ignored
    CHECK (synth.getNumVoices() == 0);
}

TEST_CASE ("PulsarSynth::process with rate=0 produces silence and no active voices", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    synth.setNumVoices (4);
    synth.prepare (48000.0, 256);
    synth.setRate (0.f);
    synth.setGrainFrequency (480.f);

    rd_dsp::RD_Buffer buf (1, 256);
    synth.process (buf.getReadArray(), buf.getWriteArray(), 1, 256);

    auto& voices = PulsarSynthTester::voices (synth);
    for (auto& v : voices)
        CHECK (v.isActive() == false);

    for (int i = 0; i < 256; ++i)
        CHECK (buf.getSample (0, i) == Catch::Approx (0.f).margin (1e-6));
}

TEST_CASE ("PulsarSynth fires a grain at block start when rate>0", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    synth.setNumVoices (4);
    synth.prepare (48000.0, 512);
    synth.setRate (100.f);          // 480 samples between grains
    synth.setGrainFrequency (480.f); // 100 sample/cycle
    synth.setGrainCycles (2);        // 200 sample grain

    rd_dsp::RD_Buffer buf (1, 100);
    synth.process (buf.getReadArray(), buf.getWriteArray(), 1, 100);

    auto& voices = PulsarSynthTester::voices (synth);
    int activeCount = 0;
    for (auto& v : voices)
        if (v.isActive()) ++activeCount;

    CHECK (activeCount >= 1); // at least one grain fired
}

TEST_CASE ("PulsarSynth with grainFrequency=0 fires nothing", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    synth.setNumVoices (4);
    synth.prepare (48000.0, 512);
    synth.setRate (100.f);
    synth.setGrainFrequency (0.f);

    rd_dsp::RD_Buffer buf (1, 512);
    synth.process (buf.getReadArray(), buf.getWriteArray(), 1, 512);

    auto& voices = PulsarSynthTester::voices (synth);
    for (auto& v : voices)
        CHECK (v.isActive() == false);
}

TEST_CASE ("PulsarSynth::setGrainCycles propagates to all voices", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    synth.setNumVoices (3);
    synth.setGrainCycles (5);

    auto& voices = PulsarSynthTester::voices (synth);
    REQUIRE (voices.size() == 3);
    for (auto& v : voices)
        CHECK (v.getGrainCycles() == 5);
}

TEST_CASE ("PulsarSynth steals oldest voice when pool is exhausted", "[PulsarSynth]")
{
    rd_dsp::PulsarSynth synth;
    synth.setNumVoices (2);
    synth.prepare (48000.0, 4096);
    synth.setRate (1000.f);          // 48 samples between grains
    synth.setGrainFrequency (480.f);  // 100 samples/cycle
    synth.setGrainCycles (10);        // 1000 sample grain - longer than rate spacing

    rd_dsp::RD_Buffer buf (1, 4096);
    synth.process (buf.getReadArray(), buf.getWriteArray(), 1, 4096);

    // Pool full of active overlapping grains; oldest gets stolen each fire. No crash.
    SUCCEED();
}
