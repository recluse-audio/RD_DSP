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

TEST_CASE ("PulsarTrain::consumePulsarFlash is false before any emission", "[PulsarTrain][PulsarFlash]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);

    CHECK (train.consumePulsarFlash() == false);
}

TEST_CASE ("PulsarTrain::consumePulsarFlash is true once after first emission", "[PulsarTrain][PulsarFlash]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    train.setEmissionRate (100.f);
    train.setFormantFreq  (200.f);
    train.start();

    runSamples (train, 1); // first tick emits at sample 0

    CHECK (train.consumePulsarFlash() == true);
}

TEST_CASE ("PulsarTrain::consumePulsarFlash returns false on immediate second call", "[PulsarTrain][PulsarFlash]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    train.setEmissionRate (100.f);
    train.setFormantFreq  (200.f);
    train.start();

    runSamples (train, 1);

    CHECK (train.consumePulsarFlash() == true);
    CHECK (train.consumePulsarFlash() == false); // already consumed, no new emission
}

TEST_CASE ("PulsarTrain::consumePulsarFlash latches again after next emission period", "[PulsarTrain][PulsarFlash]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    train.setEmissionRate (100.f); // period 480
    train.setFormantFreq  (200.f);
    train.start();

    runSamples (train, 1);
    CHECK (train.consumePulsarFlash() == true);

    runSamples (train, 480); // cross into the next emission

    CHECK (train.consumePulsarFlash() == true);
}
