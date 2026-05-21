/**
 * Created on 2026-05-21 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include "PULSAR/Pulsar.h"
#include "WAVEFORM/Wavetable.h"
#include "WINDOW/Window.h"

TEST_CASE ("Pulsar::isActive is false on construction", "[Pulsar][PulsarActivity]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    rd_dsp::Pulsar pulsar (wavetable, window);

    CHECK (pulsar.isActive() == false);
}

TEST_CASE ("Pulsar::isActive is true after emit", "[Pulsar][PulsarActivity]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    window.setSize (48000);
    rd_dsp::Pulsar pulsar (wavetable, window);
    pulsar.prepare (48000.0, 512);

    pulsar.emit (440.f, 10);

    CHECK (pulsar.isActive() == true);
}

TEST_CASE ("Pulsar::isActive returns false after duty cycle drains", "[Pulsar][PulsarActivity]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    window.setSize (48000);
    rd_dsp::Pulsar pulsar (wavetable, window);
    pulsar.prepare (48000.0, 512);

    constexpr int kDutyCycle = 10;
    pulsar.emit (440.f, kDutyCycle);

    for (int i = 0; i < kDutyCycle; ++i)
        pulsar.processSingleSample();

    CHECK (pulsar.isActive() == false);
}
