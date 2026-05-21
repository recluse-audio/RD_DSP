/**
 * Created on 2026-05-21 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "PULSAR/PulsarTrain.h"

namespace
{
    void runSamples (rd_dsp::PulsarTrain& train, int numSamples)
    {
        std::vector<float> out (static_cast<std::size_t> (numSamples), 0.f);
        float* writes[1] = { out.data() };
        train.process (nullptr, writes, 1, numSamples);
    }
}

TEST_CASE ("PulsarTrain::isActive is false before start", "[PulsarTrain][PulsarActivity]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);

    CHECK (train.isActive() == false);
}

TEST_CASE ("PulsarTrain::isActive is true mid duty cycle after start", "[PulsarTrain][PulsarActivity]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    train.setEmissionRate (100.f); // period 480 samples
    train.setFormantFreq  (200.f); // duty cycle 240 samples
    train.start();

    runSamples (train, 10); // well inside duty cycle

    CHECK (train.isActive() == true);
}

TEST_CASE ("PulsarTrain::isActive is false after duty cycle drains", "[PulsarTrain][PulsarActivity]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    train.setEmissionRate (100.f); // period 480 samples
    train.setFormantFreq  (200.f); // duty cycle 240 samples
    train.start();

    runSamples (train, 240); // drains the duty cycle, still before next emission

    CHECK (train.isActive() == false);
}
