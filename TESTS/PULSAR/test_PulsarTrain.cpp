/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <atomic>
#include <string>

#include "PULSAR/PulsarTrain.h"
#include "PULSAR/Pulsar.h"
#include "WAVEFORM/Wavetable.h"

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
    static double sampleRate (const PulsarTrain& t) { return t.mSampleRate; }
    static int    blockSize  (const PulsarTrain& t) { return t.mBlockSize; }

    static float emissionPeriod (const PulsarTrain& t)
    {
        return t.mEmissionPeriod.load (std::memory_order_relaxed);
    }
};
} // namespace rd_dsp

using rd_dsp::PulsarTrainTester;

TEST_CASE ("PulsarTrain::getEngineName returns rd_dsp::PulsarTrain", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    REQUIRE (train.getEngineName() == "rd_dsp::PulsarTrain");
}

TEST_CASE ("PulsarTrain constructs wavetable and pulsar in ctor", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    REQUIRE (PulsarTrainTester::pulsar (train) != nullptr);

    // wavetable instance exists and is accessible
    rd_dsp::Wavetable& wt = PulsarTrainTester::wavetable (train);
    (void) wt;
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

TEST_CASE ("PulsarTrain::_updateEmissionPeriod fires from setEmissionRate", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.prepare (48000.0, 512);

    constexpr float margin = 1e-3f;

    // 100 Hz at 48000 SR -> 480 samples
    train.setEmissionRate (100.f);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (480.f).margin (margin));

    // kMaxEmissionRate (150) at 48000 SR -> 320 samples
    train.setEmissionRate (rd_dsp::PulsarTrain::kMaxEmissionRate);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (320.f).margin (margin));

    // kMinEmissionRate (0.125) at 48000 SR -> 384000 samples
    train.setEmissionRate (rd_dsp::PulsarTrain::kMinEmissionRate);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (384000.f).margin (margin));

    // 0 emission rate -> 0 (guard)
    train.setEmissionRate (0.f);
    CHECK (PulsarTrainTester::emissionPeriod (train) == 0.f);

    // negative emission rate -> 0 (guard)
    train.setEmissionRate (-1.f);
    CHECK (PulsarTrainTester::emissionPeriod (train) == 0.f);
}

TEST_CASE ("PulsarTrain::_updateEmissionPeriod fires from prepare with new sampleRate", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;
    train.setEmissionRate (100.f);

    constexpr float margin = 1e-3f;

    // 100 Hz at 48000 SR -> 480 samples
    train.prepare (48000.0, 512);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (480.f).margin (margin));

    // 100 Hz at 44100 SR -> 441 samples
    train.prepare (44100.0, 512);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (441.f).margin (margin));

    // 100 Hz at 96000 SR -> 960 samples
    train.prepare (96000.0, 512);
    CHECK (PulsarTrainTester::emissionPeriod (train) == Catch::Approx (960.f).margin (margin));
}

TEST_CASE ("PulsarTrain::setWavePosition does not crash", "[PulsarTrain]")
{
    rd_dsp::PulsarTrain train;

    train.setWavePosition (0.0f);
    train.setWavePosition (0.5f);
    train.setWavePosition (1.0f);
    SUCCEED();
}
