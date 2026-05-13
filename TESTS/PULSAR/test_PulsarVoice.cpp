/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <cmath>
#include <vector>

#include "PULSAR/PulsarVoice.h"
#include "WAVEFORM/Wavetable.h"
#include "RD_BUFFER/RD_Buffer.h"

TEST_CASE ("PulsarVoice::getEngineName returns rd_dsp::PulsarVoice", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    REQUIRE (voice.getEngineName() == "rd_dsp::PulsarVoice");
}

TEST_CASE ("PulsarVoice::triggerGrain activates voice and arms countdown", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    voice.prepare (48000.0, 512);

    REQUIRE (voice.isActive() == false);

    voice.triggerGrain (480.f); // 100 samples/cycle at 48kSR
    CHECK (voice.isActive() == true);
}

TEST_CASE ("PulsarVoice::noteOff is a no-op (grain auto-stops)", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    voice.prepare (48000.0, 512);

    voice.triggerGrain (480.f);
    REQUIRE (voice.isActive() == true);

    voice.noteOff (0.f);
    CHECK (voice.isActive() == true); // still active; auto-stops after N cycles
}

TEST_CASE ("PulsarVoice plays N cycles then becomes inactive", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    voice.prepare (48000.0, 1024);

    voice.setGrainCycles (2);
    voice.triggerGrain (480.f); // 100 samples/cycle -> 200-sample grain

    rd_dsp::RD_Buffer buf (1, 199);
    voice.process (buf.getReadArray(), buf.getWriteArray(), 1, 199);
    CHECK (voice.isActive() == true);

    rd_dsp::RD_Buffer buf2 (1, 1);
    voice.process (buf2.getReadArray(), buf2.getWriteArray(), 1, 1);
    CHECK (voice.isActive() == false);
}

TEST_CASE ("PulsarVoice setGrainCycles changes grain duration", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    voice.prepare (48000.0, 1024);

    voice.setGrainCycles (4); // 4 * 100 = 400 samples
    voice.triggerGrain (480.f);

    rd_dsp::RD_Buffer buf (1, 200);
    voice.process (buf.getReadArray(), buf.getWriteArray(), 1, 200);
    CHECK (voice.isActive() == true); // halfway through

    voice.process (buf.getReadArray(), buf.getWriteArray(), 1, 200);
    CHECK (voice.isActive() == false);
}

TEST_CASE ("PulsarVoice retriggers cleanly after grain ends", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    voice.prepare (48000.0, 1024);

    voice.setGrainCycles (2);
    voice.triggerGrain (480.f);

    rd_dsp::RD_Buffer buf (1, 200);
    voice.process (buf.getReadArray(), buf.getWriteArray(), 1, 200);
    REQUIRE (voice.isActive() == false);

    voice.triggerGrain (480.f);
    CHECK (voice.isActive() == true);

    voice.process (buf.getReadArray(), buf.getWriteArray(), 1, 200);
    CHECK (voice.isActive() == false);
}

TEST_CASE ("PulsarVoice triggerGrain with zero frequency is a no-op", "[PulsarVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::PulsarVoice voice (wavetable);
    voice.prepare (48000.0, 512);

    voice.triggerGrain (0.f);
    CHECK (voice.isActive() == false);
}
