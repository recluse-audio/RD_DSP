/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include "PULSAR/Pulsar.h"
#include "OSCILLATOR/Oscillator.h"
#include "WAVEFORM/Wavetable.h"

namespace rd_dsp
{
class PulsarTester
{
public:
    static Oscillator* oscillator (Pulsar& p) { return p.mOscillator.get(); }
    static Wavetable& wavetable (Pulsar& p) { return p.mWavetable; }

    static const Wavetable* oscillatorWavetable (Pulsar& p) { return &p.mOscillator->mWavetable; }
    static double oscillatorSampleRate (Pulsar& p) { return p.mOscillator->mSampleRate; }
    static int    oscillatorBlockSize  (Pulsar& p) { return p.mOscillator->mBlockSize; }
};
} // namespace rd_dsp

using rd_dsp::PulsarTester;

TEST_CASE ("Pulsar::getEngineName returns rd_dsp::Pulsar", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Pulsar pulsar (wavetable);
    REQUIRE (pulsar.getEngineName() == "rd_dsp::Pulsar");
}

TEST_CASE ("Pulsar holds reference to provided wavetable and constructs oscillator on it", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Pulsar pulsar (wavetable);

    CHECK (&PulsarTester::wavetable (pulsar) == &wavetable);
    REQUIRE (PulsarTester::oscillator (pulsar) != nullptr);
    CHECK (PulsarTester::oscillatorWavetable (pulsar) == &wavetable);
}

TEST_CASE ("Pulsar::prepare propagates sampleRate/blockSize to oscillator", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Pulsar pulsar (wavetable);

    constexpr double kSampleRate = 48000.0;
    constexpr int    kBlockSize  = 256;
    pulsar.prepare (kSampleRate, kBlockSize);

    CHECK (PulsarTester::oscillatorSampleRate (pulsar) == kSampleRate);
    CHECK (PulsarTester::oscillatorBlockSize  (pulsar) == kBlockSize);
}
