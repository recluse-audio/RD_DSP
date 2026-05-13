/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <vector>

#include "PULSAR/Pulsar.h"
#include "OSCILLATOR/Oscillator.h"
#include "WAVEFORM/Wavetable.h"
#include "WINDOW/Window.h"

namespace rd_dsp
{
class PulsarTester
{
public:
    static Oscillator* oscillator (Pulsar& p) { return p.mOscillator.get(); }
    static Wavetable& wavetable (Pulsar& p) { return p.mWavetable; }
    static Window& window (Pulsar& p) { return p.mWindow; }
    static float windowPos (Pulsar& p) { return p.mWindowPos; }
    static float windowIncrement (Pulsar& p) { return p.mWindowIncrement; }

    static const Wavetable* oscillatorWavetable (Pulsar& p) { return &p.mOscillator->mWavetable; }
    static double oscillatorSampleRate (Pulsar& p) { return p.mOscillator->mSampleRate; }
    static int    oscillatorBlockSize  (Pulsar& p) { return p.mOscillator->mBlockSize; }
};
} // namespace rd_dsp

using rd_dsp::PulsarTester;

TEST_CASE ("Pulsar::getEngineName returns rd_dsp::Pulsar", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    rd_dsp::Pulsar pulsar (wavetable, window);
    REQUIRE (pulsar.getEngineName() == "rd_dsp::Pulsar");
}

TEST_CASE ("Pulsar holds references to provided wavetable/window and constructs oscillator on wavetable", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    rd_dsp::Pulsar pulsar (wavetable, window);

    CHECK (&PulsarTester::wavetable (pulsar) == &wavetable);
    CHECK (&PulsarTester::window (pulsar) == &window);
    REQUIRE (PulsarTester::oscillator (pulsar) != nullptr);
    CHECK (PulsarTester::oscillatorWavetable (pulsar) == &wavetable);
}

TEST_CASE ("Pulsar::prepare propagates sampleRate/blockSize to oscillator", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    rd_dsp::Pulsar pulsar (wavetable, window);

    constexpr double kSampleRate = 48000.0;
    constexpr int    kBlockSize  = 256;
    pulsar.prepare (kSampleRate, kBlockSize);

    CHECK (PulsarTester::oscillatorSampleRate (pulsar) == kSampleRate);
    CHECK (PulsarTester::oscillatorBlockSize  (pulsar) == kBlockSize);
}

TEST_CASE ("Pulsar::emit resets windowPos and sets increment to span dutyCycle", "[Pulsar]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::Window window;
    window.setSize (48000); // 1 second @ 48kHz
    rd_dsp::Pulsar pulsar (wavetable, window);
    pulsar.prepare (48000.0, 512);

    constexpr float margin = 1e-3f;

    pulsar.emit (440.f, 4800); // 100ms duty cycle
    CHECK (PulsarTester::windowPos (pulsar) == 0.f);
    CHECK (PulsarTester::windowIncrement (pulsar) == Catch::Approx (48000.f / 4800.f).margin (margin));

    pulsar.emit (220.f, 1000);
    CHECK (PulsarTester::windowPos (pulsar) == 0.f);
    CHECK (PulsarTester::windowIncrement (pulsar) == Catch::Approx (48000.f / 1000.f).margin (margin));
}

TEST_CASE ("Pulsar::processSingleSample matches block-wise process sample-for-sample", "[Pulsar]")
{
    constexpr double kSampleRate = 48000.0;
    constexpr int    kBlockSize  = 256;
    constexpr int    kDutyCycle  = 240;
    constexpr float  kFormant    = 200.f;
    constexpr float  kMargin     = 1e-5f;

    rd_dsp::Wavetable wtBlock,  wtSingle;
    rd_dsp::Window    winBlock, winSingle;
    winBlock.setSize  (static_cast<int> (kSampleRate));
    winSingle.setSize (static_cast<int> (kSampleRate));
    winBlock.setShape  (rd_dsp::Window::Shape::kHanning);
    winSingle.setShape (rd_dsp::Window::Shape::kHanning);

    rd_dsp::Pulsar pBlock  (wtBlock,  winBlock);
    rd_dsp::Pulsar pSingle (wtSingle, winSingle);
    pBlock.prepare  (kSampleRate, kBlockSize);
    pSingle.prepare (kSampleRate, kBlockSize);

    pBlock.emit  (kFormant, kDutyCycle);
    pSingle.emit (kFormant, kDutyCycle);

    constexpr int kN = kDutyCycle + 32; // include tail past duty cycle
    std::vector<float> blockOut (kN, 0.f);
    float* writes[1] = { blockOut.data() };
    pBlock.process (nullptr, writes, 1, kN);

    for (int i = 0; i < kN; ++i)
    {
        const float s = pSingle.processSingleSample();
        CHECK (s == Catch::Approx (blockOut[i]).margin (kMargin));
    }
}
