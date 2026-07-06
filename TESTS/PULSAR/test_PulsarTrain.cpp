/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <atomic>
#include <cmath>
#include <string>

#include <vector>

#include "PULSAR/PulsarTrain.h"
#include "PULSAR/Pulsar.h"
#include "WAVEFORM/Wavetable.h"
#include "WINDOW/Window.h"
#include "OSCILLATOR/Oscillator.h"
#include "RD_BUFFER/RD_Buffer.h"
#include "RD_BUFFER/BufferFiller.h"

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

    static RandomizedParam& emissionRateRandom (PulsarTrain& t) { return t.mEmissionRateRandom; }
    static RandomizedParam& formantRandom (PulsarTrain& t) { return t.mPulsarData.formantFreq; }
    static RandomizedParam& wavePositionRandom (PulsarTrain& t) { return t.mPulsarData.wavePosition; }
    static RandomizedParam& ampRandom (PulsarTrain& t) { return t.mPulsarData.amp; }

    static void emit (PulsarTrain& t) { t._emitPulsar(); }

    static float emissionCount (const PulsarTrain& t)
    {
        return t.mEmissionCount.load (std::memory_order_relaxed);
    }

    static float emissionPeriod (const PulsarTrain& t)
    {
        return t.mEmissionPeriod.load (std::memory_order_relaxed);
    }

    static bool updateNeeded (const PulsarTrain& t)
    {
        return t.mEmissionPeriodUpdateNeeded.load (std::memory_order_relaxed);
    }

    static void flushUpdate (PulsarTrain& t)
    {
        t._updateEmissionPeriod();
        t.mEmissionPeriodUpdateNeeded.store (false, std::memory_order_relaxed);
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
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";

    train.loadWavetable (tablePath);

    CHECK (wt.getNumWaveforms() == 4);
    CHECK (wt.getWaveformSize() == 8192);
}

TEST_CASE ("PulsarTrain::getWavetable returns live ref usable for fillDisplayBuffer", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";

    train.loadWavetable (tablePath);
    train.setWavePosition (0.5f);

    const rd_dsp::Wavetable& wt = train.getWavetable();

    CHECK (wt.getNumWaveforms() == 4);
    CHECK (wt.getWaveformSize() == 8192);

    constexpr int kDisplaySize = 128;
    std::vector<float> buf (kDisplaySize, 0.f);

    wt.fillDisplayBuffer (buf.data(), kDisplaySize);

    float absSum = 0.f;
    for (float v : buf)
        absSum += std::abs (v);

    CHECK (absSum > 0.f);

    // accessor must reflect live wave-pos state: changing wave position changes the buffer
    std::vector<float> bufA (kDisplaySize, 0.f);
    train.setWavePosition (0.f);
    wt.fillDisplayBuffer (bufA.data(), kDisplaySize);

    std::vector<float> bufB (kDisplaySize, 0.f);
    train.setWavePosition (1.f);
    wt.fillDisplayBuffer (bufB.data(), kDisplaySize);

    bool anyDiff = false;
    for (int i = 0; i < kDisplaySize; ++i)
        if (bufA[i] != bufB[i]) { anyDiff = true; break; }

    CHECK (anyDiff);
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

TEST_CASE ("PulsarTrain emission-rate random param defaults to full range, density 0", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& emissionRand = PulsarTrainTester::emissionRateRandom (train);

    CHECK (emissionRand.getStart() == rd_dsp::PulsarTrain::kMinEmissionRate);
    CHECK (emissionRand.getEnd() == rd_dsp::PulsarTrain::kMaxEmissionRate);
    CHECK (emissionRand.getDensity() == 0.0f);
}

TEST_CASE ("PulsarTrain::setEmissionRange / setEmissionDensity move the underlying RandomizedParam", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& emissionRand = PulsarTrainTester::emissionRateRandom (train);

    train.setEmissionRange (10.f, 40.f);
    CHECK (emissionRand.getStart() == 10.f);
    CHECK (emissionRand.getEnd() == 40.f);

    train.setEmissionDensity (0.7f);
    CHECK (emissionRand.getDensity() == 0.7f);
}

TEST_CASE ("PulsarTrain randomized emission rate varies the period within bounds", "[PulsarTrain]")
{
    constexpr double kSampleRate = 48000.0;

    rd_dsp::PulsarTrain train;
    train.prepare (kSampleRate, 512);
    train.setEmissionRate (50.f); // center; ignored when density 1 (always random)

    rd_dsp::RandomizedParam& emissionRand = PulsarTrainTester::emissionRateRandom (train);
    emissionRand.setDensity (1.0f); // always randomize the rate

    // rate in [kMin,kMax] -> period in [sr/kMax, sr/kMin]
    const float minPeriod = static_cast<float> (kSampleRate) / rd_dsp::PulsarTrain::kMaxEmissionRate;
    const float maxPeriod = static_cast<float> (kSampleRate) / rd_dsp::PulsarTrain::kMinEmissionRate;

    float firstPeriod = -1.f;
    bool varied = false;

    for (int index = 0; index < 50; ++index)
    {
        PulsarTrainTester::emit (train);
        const float period = PulsarTrainTester::emissionPeriod (train);

        if (firstPeriod < 0.f)
            firstPeriod = period;
        else if (period != firstPeriod)
            varied = true;

        CHECK (period >= minPeriod);
        CHECK (period <= maxPeriod);
    }

    CHECK (varied);
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

TEST_CASE ("PulsarTrain::setFormantRange / setFormantDensity move the underlying RandomizedParam", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& formantRand = PulsarTrainTester::formantRandom (train);

    train.setFormantRange (300.f, 800.f);
    CHECK (formantRand.getStart() == 300.f);
    CHECK (formantRand.getEnd() == 800.f);

    train.setFormantDensity (0.5f);
    CHECK (formantRand.getDensity() == 0.5f);
}

TEST_CASE ("PulsarTrain formant density 0 collapses draws to center; density 1 varies within range", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& formantRand = PulsarTrainTester::formantRandom (train);

    // density 0: every draw is the center, regardless of range
    train.setFormantFreq (440.f);
    train.setFormantRange (300.f, 800.f);
    train.setFormantDensity (0.f);
    for (int index = 0; index < 20; ++index)
        CHECK (formantRand.getRandomizedValue() == 440.f);

    // density 1: every draw is random within [min,max] and varies
    train.setFormantDensity (1.f);
    float firstValue = -1.f;
    bool varied = false;
    for (int index = 0; index < 50; ++index)
    {
        const float value = formantRand.getRandomizedValue();
        CHECK (value >= 300.f);
        CHECK (value <= 800.f);
        if (firstValue < 0.f)
            firstValue = value;
        else if (value != firstValue)
            varied = true;
    }
    CHECK (varied);
}

TEST_CASE ("PulsarTrain::setEmissionRate queues update; _updateEmissionPeriod computes period", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);
    PulsarTrainTester::flushUpdate (train);

    constexpr float margin = 1e-3f;

    train.setEmissionRate (100.f);
    CHECK (PulsarTrainTester::updateNeeded (train) == true);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (480.f).margin (margin));

    train.setEmissionRate (rd_dsp::PulsarTrain::kMaxEmissionRate);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (320.f).margin (margin));

    train.setEmissionRate (rd_dsp::PulsarTrain::kMinEmissionRate);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (384000.f).margin (margin));

    train.setEmissionRate (0.f);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == 0.f);

    train.setEmissionRate (-1.f);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == 0.f);
}

TEST_CASE ("PulsarTrain::prepare queues update; _updateEmissionPeriod recomputes for new sampleRate", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.setEmissionRate (100.f);

    constexpr float margin = 1e-3f;

    train.prepare (48000.0, 512);
    CHECK (PulsarTrainTester::updateNeeded (train) == true);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (480.f).margin (margin));

    train.prepare (44100.0, 512);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (441.f).margin (margin));

    train.prepare (96000.0, 512);
    PulsarTrainTester::flushUpdate (train);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (960.f).margin (margin));
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

TEST_CASE ("PulsarTrain::setWavePosition / getWavePosition round-trip the center", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    CHECK (train.getWavePosition() == 0.f);

    train.setWavePosition (0.3f);
    CHECK (train.getWavePosition() == 0.3f);

    train.setWavePosition (1.f);
    CHECK (train.getWavePosition() == 1.f);
}

TEST_CASE ("PulsarTrain wave-position random param defaults to full range, density 0", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& wavePosRand = PulsarTrainTester::wavePositionRandom (train);

    CHECK (wavePosRand.getStart() == 0.f);
    CHECK (wavePosRand.getEnd() == 1.f);
    CHECK (wavePosRand.getDensity() == 0.f);
}

TEST_CASE ("PulsarTrain::setWavePositionRange / setWavePositionDensity move the underlying RandomizedParam", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& wavePosRand = PulsarTrainTester::wavePositionRandom (train);

    train.setWavePositionRange (0.2f, 0.8f);
    CHECK (wavePosRand.getStart() == 0.2f);
    CHECK (wavePosRand.getEnd() == 0.8f);

    train.setWavePositionDensity (0.6f);
    CHECK (wavePosRand.getDensity() == 0.6f);
}

TEST_CASE ("PulsarTrain _emitPulsar applies the drawn wave position to the wavetable", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";
    train.loadWavetable (tablePath);

    rd_dsp::RandomizedParam& wavePosRand = PulsarTrainTester::wavePositionRandom (train);
    rd_dsp::Wavetable& wt = PulsarTrainTester::wavetable (train);

    constexpr int kDisplaySize = 128;

    // Set the center directly on the param (bypassing setWavePosition's own
    // wavetable push) so the only thing that can move the wavetable is _emitPulsar.
    wavePosRand.setCenter (0.f); // density 0 -> draw is the center
    PulsarTrainTester::emit (train);
    std::vector<float> bufA (kDisplaySize, 0.f);
    wt.fillDisplayBuffer (bufA.data(), kDisplaySize);

    wavePosRand.setCenter (1.f);
    PulsarTrainTester::emit (train);
    std::vector<float> bufB (kDisplaySize, 0.f);
    wt.fillDisplayBuffer (bufB.data(), kDisplaySize);

    bool anyDiff = false;
    for (int i = 0; i < kDisplaySize; ++i)
        if (bufA[i] != bufB[i]) { anyDiff = true; break; }

    CHECK (anyDiff);
}

TEST_CASE ("PulsarTrain::consumeWavePositionChanged flags a redraw only when the wave position changes", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);

    // Fresh: displayed position is current, nothing to redraw (matches flash semantics).
    CHECK (train.consumeWavePositionChanged() == false);

    // Control change flags a redraw, consumed exactly once.
    train.setWavePosition (0.5f);
    CHECK (train.consumeWavePositionChanged() == true);
    CHECK (train.consumeWavePositionChanged() == false);

    rd_dsp::RandomizedParam& wavePosRand = PulsarTrainTester::wavePositionRandom (train);

    // Density 0: every emission draws the same center -> no change -> no redraw.
    wavePosRand.setDensity (0.f); // center already 0.5 from setWavePosition
    PulsarTrainTester::emit (train);
    CHECK (train.consumeWavePositionChanged() == false);

    // Density 1: emitted position varies -> at least one emission flags a redraw.
    wavePosRand.setDensity (1.f);
    bool flagged = false;
    for (int i = 0; i < 50; ++i)
    {
        PulsarTrainTester::emit (train);
        if (train.consumeWavePositionChanged())
        {
            flagged = true;
            break;
        }
    }
    CHECK (flagged);
}

TEST_CASE ("PulsarTrain amp random param defaults to unity center, full range, density 0", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& ampRand = PulsarTrainTester::ampRandom (train);

    CHECK (ampRand.getCenterValue() == 1.f); // unity by default: unset amp = full output
    CHECK (ampRand.getStart() == 0.f);
    CHECK (ampRand.getEnd() == 1.f);
    CHECK (ampRand.getDensity() == 0.f);
}

TEST_CASE ("PulsarTrain::setAmp / getAmp round-trip the center", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    CHECK (train.getAmp() == 1.f);

    train.setAmp (0.5f);
    CHECK (train.getAmp() == 0.5f);

    train.setAmp (0.f);
    CHECK (train.getAmp() == 0.f);
}

TEST_CASE ("PulsarTrain::setAmpRange / setAmpDensity move the underlying RandomizedParam", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    rd_dsp::RandomizedParam& ampRand = PulsarTrainTester::ampRandom (train);

    train.setAmpRange (0.25f, 0.75f);
    CHECK (ampRand.getStart() == 0.25f);
    CHECK (ampRand.getEnd() == 0.75f);

    train.setAmpDensity (0.4f);
    CHECK (ampRand.getDensity() == 0.4f);
}

TEST_CASE ("PulsarTrain amp scales emission output by the drawn value", "[PulsarTrain]")
{
    constexpr double kSampleRate     = 48000.0;
    constexpr int    kBlockSize      = 512;
    constexpr float  kEmissionRate   = 100.f;   // period = 480 samples
    constexpr float  kFormantFreq    = 200.f;   // one cycle = 240 samples
    constexpr int    kCycleSamples   = 240;
    constexpr float  kAmp            = 0.5f;
    constexpr float  kMargin         = 1e-4f;

    rd_dsp::PulsarTrain train;
    train.prepare (kSampleRate, kBlockSize);

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";
    train.loadWavetable (tablePath);
    train.setWavePosition (0.f);                       // sine slot
    train.setWindowType  (rd_dsp::WindowType::kNone);  // all-ones window
    train.setEmissionRate (kEmissionRate);
    train.setFormantFreq  (kFormantFreq);
    train.setAmp          (kAmp);                      // density 0 -> every draw is 0.5
    train.start();

    std::vector<float> outL (kCycleSamples, 0.f);
    float* writes[1] = { outL.data() };
    train.process (nullptr, writes, 1, kCycleSamples);

    // Reference oscillator on the same wavetable, unscaled; expect output == ref * amp.
    rd_dsp::Oscillator refOsc (PulsarTrainTester::wavetable (train));
    refOsc.prepare (kSampleRate, kBlockSize);
    refOsc.setFreq (kFormantFreq);
    refOsc.start();

    std::vector<float> expected (kCycleSamples, 0.f);
    float* expWrites[1] = { expected.data() };
    refOsc.process (nullptr, expWrites, 1, kCycleSamples);

    for (int i = 0; i < kCycleSamples; ++i)
        CHECK (outL[i] == Catch::Approx (expected[i] * kAmp).margin (kMargin));
}

TEST_CASE ("PulsarTrain end-to-end: one 200Hz sine cycle then zeros within emission period", "[PulsarTrain]""[E2E]")
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
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";
    train.loadWavetable (tablePath);
    train.setWavePosition (0.f);                          // sine slot
    train.setWindowType  (rd_dsp::WindowType::kNone);     // all-ones window
    train.setEmissionRate (kEmissionRate);
    train.setFormantFreq  (kFormantFreq);
    train.start();

    // run two full emission periods; start() seeds count = period so first emit fires at sample 0
    constexpr int kTotal = 2 * kEmissionPeriod;
    std::vector<float> outL (kTotal, 0.f);
    float* writes[1] = { outL.data() };
    train.process (nullptr, writes, 1, kTotal);

    // expected one cycle: separate Oscillator on the same wavetable at formant freq
    rd_dsp::Oscillator refOsc (PulsarTrainTester::wavetable (train));
    refOsc.prepare (kSampleRate, kBlockSize);
    refOsc.setFreq (kFormantFreq);
    refOsc.start();

    std::vector<float> expected (kCycleSamples, 0.f);
    float* expWrites[1] = { expected.data() };
    refOsc.process (nullptr, expWrites, 1, kCycleSamples);

    // samples 0..kCycleSamples-1: first emission's cycle
    for (int i = 0; i < kCycleSamples; ++i)
        CHECK (outL[i] == Catch::Approx (expected[i]).margin (kMargin));

    // samples kCycleSamples..kEmissionPeriod-1: silent tail of first period
    for (int i = kCycleSamples; i < kEmissionPeriod; ++i)
        CHECK (outL[i] == Catch::Approx (0.f).margin (1e-6f));

    // samples kEmissionPeriod..kEmissionPeriod+kCycleSamples-1: second emission's cycle
    for (int i = 0; i < kCycleSamples; ++i)
        CHECK (outL[kEmissionPeriod + i] == Catch::Approx (expected[i]).margin (kMargin));

    // remainder of second period: zeros
    for (int i = kEmissionPeriod + kCycleSamples; i < kTotal; ++i)
        CHECK (outL[i] == Catch::Approx (0.f).margin (1e-6f));
}

TEST_CASE ("PulsarTrain::start emits immediately; ::stop silences output", "[PulsarTrain]")
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
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";
    train.loadWavetable (tablePath);
    train.setWavePosition (0.f);
    train.setWindowType  (rd_dsp::WindowType::kNone);
    train.setEmissionRate (kEmissionRate);
    train.setFormantFreq  (kFormantFreq);

    train.start();

    std::vector<float> outL (kEmissionPeriod, 0.f);
    float* writes[1] = { outL.data() };
    train.process (nullptr, writes, 1, kEmissionPeriod);

    rd_dsp::Oscillator refOsc (PulsarTrainTester::wavetable (train));
    refOsc.prepare (kSampleRate, kBlockSize);
    refOsc.setFreq (kFormantFreq);
    refOsc.start();

    std::vector<float> expected (kCycleSamples, 0.f);
    float* expWrites[1] = { expected.data() };
    refOsc.process (nullptr, expWrites, 1, kCycleSamples);

    // first cycle: matches ref oscillator
    for (int i = 0; i < kCycleSamples; ++i)
        CHECK (outL[i] == Catch::Approx (expected[i]).margin (kMargin));

    // remainder of first period: zeros
    for (int i = kCycleSamples; i < kEmissionPeriod; ++i)
        CHECK (outL[i] == Catch::Approx (0.f).margin (1e-6f));

    // stop -> next process call must be all zeros
    train.stop();
    std::vector<float> stopped (kEmissionPeriod, 1.f); // pre-fill non-zero to detect overwrite
    float* stopWrites[1] = { stopped.data() };
    train.process (nullptr, stopWrites, 1, kEmissionPeriod);
    for (int i = 0; i < kEmissionPeriod; ++i)
        CHECK (stopped[i] == Catch::Approx (0.f).margin (1e-6f));
}

TEST_CASE ("PulsarTrain output matches golden tukey-windowed sine over duty cycle", "[PulsarTrain]")
{
    constexpr double kSampleRate     = 48000.0;
    constexpr int    kBlockSize      = 512;
    constexpr float  kEmissionRate   = 100.f;   // period = 480 samples
    constexpr float  kFormantFreq    = 200.f;   // one cycle = 240 samples
    constexpr int    kEmissionPeriodSamples = 480;
    constexpr int    kDutyCycleSamples      = 240;
    constexpr int    kNumChannels           = 1;
    constexpr float  kSampleMatchMargin     = 1e-5f; // golden mirrors impl resampling exactly; float32 round-trip only

    rd_dsp::PulsarTrain pulsarTrain;
    pulsarTrain.prepare (kSampleRate, kBlockSize);

    const std::string wavetablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/WAVETABLES/GOLDEN_Wavetable_BasicShapes_8192.csv";
    pulsarTrain.loadWavetable   (wavetablePath);
    pulsarTrain.setWavePosition (0.f);
    pulsarTrain.setWindowType   (rd_dsp::WindowType::kTukey);
    pulsarTrain.setEmissionRate (kEmissionRate);
    pulsarTrain.setFormantFreq  (kFormantFreq);
    pulsarTrain.start();

    rd_dsp::RD_Buffer processBuffer (kNumChannels, kEmissionPeriodSamples);
    pulsarTrain.process (processBuffer.getReadArray(), processBuffer.getWriteArray(),
                         kNumChannels, kEmissionPeriodSamples);

    // Golden CSV: header "amplitude" then one amplitude per row (single channel).
    rd_dsp::RD_Buffer goldenBuffer (kNumChannels, kDutyCycleSamples);
    const std::string goldenPath =
        std::string (RD_DSP_TESTS_DIR) + "/PULSAR/GOLDEN/OUTPUT/GOLDEN_PULSAR_TUKEY_48000SR_200Hz_1C.csv";
    const bool goldenLoaded = rd_dsp::BufferFiller::fillFromCSV (goldenPath, goldenBuffer);
    REQUIRE (goldenLoaded);

    // Duty cycle window: PulsarTrain output equals golden windowed sine.
    for (int sampleIndex = 0; sampleIndex < kDutyCycleSamples; ++sampleIndex)
    {
        const float actualSample   = processBuffer.getSample (0, sampleIndex);
        const float expectedSample = goldenBuffer.getSample  (0, sampleIndex);
        CHECK (actualSample == Catch::Approx (expectedSample).margin (kSampleMatchMargin));
    }

    // Rest of emission period: silent until next emission.
    for (int sampleIndex = kDutyCycleSamples; sampleIndex < kEmissionPeriodSamples; ++sampleIndex)
    {
        const float actualSample = processBuffer.getSample (0, sampleIndex);
        CHECK (actualSample == Catch::Approx (0.f).margin (1e-6f));
    }
}
