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
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";
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
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";
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
    constexpr float  kSampleMatchMargin     = 5e-3f; // wavetable sine vs pure math sine

    rd_dsp::PulsarTrain pulsarTrain;
    pulsarTrain.prepare (kSampleRate, kBlockSize);

    const std::string wavetablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";
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
