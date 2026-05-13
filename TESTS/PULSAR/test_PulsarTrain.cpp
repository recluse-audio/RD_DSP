/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <atomic>
#include <string>

#include <vector>

#include "PULSAR/PulsarTrain.h"
#include "PULSAR/Pulsar.h"
#include "WAVEFORM/Wavetable.h"
#include "WINDOW/Window.h"
#include "OSCILLATOR/Oscillator.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

namespace rd_dsp
{
class PulsarTrainTester
{
public:
    static Pulsar* pulsar (PulsarTrain& t) { return t.mPulsar.get(); }
    static Wavetable& wavetable (PulsarTrain& t) { return *t.mWavetable; }
    static Window& window (PulsarTrain& t) { return *t.mWindow; }
    static double sampleRate (const PulsarTrain& t) { return t.mSampleRate; }
    static int    blockSize  (const PulsarTrain& t) { return t.mBlockSize; }

    static float emissionCountdown (const PulsarTrain& t)
    {
        return t.mEmissionCountdown.load (std::memory_order_relaxed);
    }

    static bool updateNeeded (const PulsarTrain& t)
    {
        return t.mEmissionCountdownUpdateNeeded.load (std::memory_order_relaxed);
    }

    static void flushUpdate (PulsarTrain& t)
    {
        t._updateEmissionCountdown();
        t.mEmissionCountdownUpdateNeeded.store (false, std::memory_order_relaxed);
    }
};
} // namespace rd_dsp

using rd_dsp::PulsarTrainTester;

TEST_CASE ("PulsarTrain::getEngineName returns rd_dsp::PulsarTrain", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    REQUIRE (train.getEngineName() == "rd_dsp::PulsarTrain");
}

TEST_CASE ("PulsarTrain constructs wavetable, window, and pulsar in ctor", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    REQUIRE (PulsarTrainTester::pulsar (train) != nullptr);

    rd_dsp::Wavetable& wt = PulsarTrainTester::wavetable (train);
    rd_dsp::Window& win = PulsarTrainTester::window (train);
    (void) wt; (void) win;
    SUCCEED();
}

TEST_CASE ("PulsarTrain::prepare stores sampleRate/blockSize", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    constexpr double kSampleRate = 48000.0;
    constexpr int    kBlockSize  = 256;
    train.prepare (kSampleRate, kBlockSize);

    CHECK (PulsarTrainTester::sampleRate (train) == kSampleRate);
    CHECK (PulsarTrainTester::blockSize  (train) == kBlockSize);
}

TEST_CASE ("PulsarTrain::prepare sizes window to 1 second of samples", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    train.prepare (48000.0, 512);
    CHECK (PulsarTrainTester::window (train).getNumSamples() == 48000);

    train.prepare (44100.0, 512);
    CHECK (PulsarTrainTester::window (train).getNumSamples() == 44100);
}

TEST_CASE ("PulsarTrain::loadWavetable replaces wavetable contents in place", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    auto& wt = PulsarTrainTester::wavetable (train);

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";

    train.loadWavetable (tablePath);

    CHECK (wt.getNumWaveforms() == 4);
    CHECK (wt.getWaveformSize() == 8096);
}

TEST_CASE ("PulsarTrain::setEmissionRate / getEmissionRate round-trip", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    // default is 0
    CHECK (train.getEmissionRate() == 0.f);

    train.setEmissionRate (25.f);
    CHECK (train.getEmissionRate() == 25.f);

    train.setEmissionRate (1000.f);
    CHECK (train.getEmissionRate() == 1000.f);

    train.setEmissionRate (0.f);
    CHECK (train.getEmissionRate() == 0.f);

    // atomic<float> must be lock-free for real-time safety
    CHECK (std::atomic<float> {}.is_lock_free());
}

TEST_CASE ("PulsarTrain::setFormantFreq / getFormantFreq round-trip", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    CHECK (train.getFormantFreq() == 0.f);

    train.setFormantFreq (440.f);
    CHECK (train.getFormantFreq() == 440.f);

    train.setFormantFreq (rd_dsp::PulsarTrain::kMinFormantFreq);
    CHECK (train.getFormantFreq() == rd_dsp::PulsarTrain::kMinFormantFreq);

    train.setFormantFreq (rd_dsp::PulsarTrain::kMaxFormantFreq);
    CHECK (train.getFormantFreq() == rd_dsp::PulsarTrain::kMaxFormantFreq);
}

TEST_CASE ("PulsarTrain::setEmissionRate queues update; _updateEmissionCountdown computes period", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    PulsarTrainTester::flushUpdate (train);

    constexpr float margin = 1e-3f;

    train.setEmissionRate (100.f);
    CHECK (PulsarTrainTester::updateNeeded (train) == true);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == Catch::Approx (480.f).margin (margin));

    train.setEmissionRate (rd_dsp::PulsarTrain::kMaxEmissionRate);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == Catch::Approx (320.f).margin (margin));

    train.setEmissionRate (rd_dsp::PulsarTrain::kMinEmissionRate);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == Catch::Approx (384000.f).margin (margin));

    train.setEmissionRate (0.f);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == 0.f);

    train.setEmissionRate (-1.f);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == 0.f);
}

TEST_CASE ("PulsarTrain::prepare queues update; _updateEmissionCountdown recomputes for new sampleRate", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.setEmissionRate (100.f);

    constexpr float margin = 1e-3f;

    train.prepare (48000.0, 512);
    CHECK (PulsarTrainTester::updateNeeded (train) == true);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == Catch::Approx (480.f).margin (margin));

    train.prepare (44100.0, 512);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == Catch::Approx (441.f).margin (margin));

    train.prepare (96000.0, 512);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionCountdown (train) == Catch::Approx (960.f).margin (margin));
}

TEST_CASE ("PulsarTrain::setWindowType propagates to window shape", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);

    auto& win = PulsarTrainTester::window (train);

    train.setWindowType (rd_dsp::WindowType::kHanning);
    CHECK (win.getShape() == rd_dsp::Window::Shape::kHanning);

    train.setWindowType (rd_dsp::WindowType::kTukey);
    CHECK (win.getShape() == rd_dsp::Window::Shape::kTukey);

    train.setWindowType (rd_dsp::WindowType::kNone);
    CHECK (win.getShape() == rd_dsp::Window::Shape::kNone);
}

TEST_CASE ("PulsarTrain::setWavePosition does not crash", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    train.setWavePosition (0.0f);
    train.setWavePosition (0.5f);
    train.setWavePosition (1.0f);
    SUCCEED();
}

TEST_CASE ("PulsarTrain end-to-end: one 200Hz sine cycle then zeros within emission period", "[PulsarTrain]")
{
    constexpr double kSampleRate     = 48000.0;
    constexpr int    kBlockSize      = 512;
    constexpr float  kEmissionRate   = 100.f;   // period = 480 samples
    constexpr float  kFormantFreq    = 200.f;   // one cycle = 240 samples
    constexpr int    kEmissionPeriod = 480;
    constexpr int    kCycleSamples   = 240;
    constexpr float  kMargin         = 1e-4f;

    rd_dsp::PulsarTrain train;
    train.prepare (kSampleRate, kBlockSize);

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";
    train.loadWavetable (tablePath);
    train.setWavePosition (0.f);                          // sine slot
    train.setWindowType  (rd_dsp::WindowType::kNone);     // all-ones window
    train.setEmissionRate (kEmissionRate);
    train.setFormantFreq  (kFormantFreq);

    // run two full emission periods so first emit (at sample kEmissionPeriod) is included
    constexpr int kTotal = 2 * kEmissionPeriod;
    std::vector<float> outL (kTotal, 0.f);
    float* writes[1] = { outL.data() };
    train.process (nullptr, writes, 1, kTotal);

    // expected first cycle: separate Oscillator on the same wavetable at formant freq
    rd_dsp::Oscillator refOsc (PulsarTrainTester::wavetable (train));
    refOsc.prepare (kSampleRate, kBlockSize);
    refOsc.setFreq (kFormantFreq);
    refOsc.start();

    std::vector<float> expected (kCycleSamples, 0.f);
    float* expWrites[1] = { expected.data() };
    refOsc.process (nullptr, expWrites, 1, kCycleSamples);

    // samples 0..kEmissionPeriod-1: silent (countdown elapsing before first emit)
    for (int i = 0; i < kEmissionPeriod; ++i)
        CHECK (outL[i] == Catch::Approx (0.f).margin (1e-6f));

    // samples kEmissionPeriod..kEmissionPeriod+kCycleSamples-1: one sine cycle
    for (int i = 0; i < kCycleSamples; ++i)
        CHECK (outL[kEmissionPeriod + i] == Catch::Approx (expected[i]).margin (kMargin));

    // remainder of second period: zeros
    for (int i = kEmissionPeriod + kCycleSamples; i < kTotal; ++i)
        CHECK (outL[i] == Catch::Approx (0.f).margin (1e-6f));
}
