#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../../SOURCE/OSCILLATOR/Oscillator.h"

using rd_dsp::Oscillator;
using rd_dsp::Waveform;
using Catch::Approx;

namespace rd_dsp
{
class OscillatorTester
{
public:
    static float phaseIncrement (const Oscillator& o) { return o.mPhaseIncrement; }
    static double sampleRate    (const Oscillator& o) { return o.mSampleRate; }
    static int    waveformSize  (const Oscillator& o) { return o.mWaveform->getNumSamples(); }
};
} // namespace rd_dsp

using rd_dsp::OscillatorTester;

TEST_CASE ("Oscillator default constructs with sine waveform", "[Oscillator]")
{
    Oscillator osc;
    CHECK (OscillatorTester::waveformSize (osc) > 0);
    CHECK (OscillatorTester::phaseIncrement (osc) == 0.f);
}

TEST_CASE ("Oscillator::prepare stores sample rate", "[Oscillator]")
{
    Oscillator osc;
    osc.prepare (48000.0);
    CHECK (OscillatorTester::sampleRate (osc) == 48000.0);
}

// Default waveform size is 2048 samples.
// Phase increment formula: inc = freq * 2048 / sampleRate

TEST_CASE ("Oscillator::setFreq computes phase increment from freq, waveform size, sample rate", "[Oscillator]")
{
    Oscillator osc;
    osc.prepare (44100.0);
    osc.setFreq (441.f);

    // 441 * 2048 / 44100 = 0.01 * 2048 = 20.48
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));
}

TEST_CASE ("Oscillator phase increment scales with frequency", "[Oscillator]")
{
    Oscillator osc;
    osc.prepare (44100.0);

    // 441 * 2048 / 44100 = 20.48
    osc.setFreq (441.f);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));

    // 882 * 2048 / 44100 = 40.96
    osc.setFreq (882.f);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (40.96f));
}

TEST_CASE ("Oscillator::prepare after setFreq updates phase increment for new sample rate", "[Oscillator]")
{
    Oscillator osc;
    osc.prepare (44100.0);
    osc.setFreq (441.f);
    // 441 * 2048 / 44100 = 20.48
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));

    // halve the SR -> increment doubles. 441 * 2048 / 22050 = 40.96
    osc.prepare (22050.0);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (40.96f));

    // double original SR -> increment halves. 441 * 2048 / 88200 = 10.24
    osc.prepare (88200.0);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (10.24f));
}

TEST_CASE("Oscillator writes its waveform to buffer in process()", "[Oscillator]")
{
    Oscillator osc;
    osc.prepare(44100.0);
    osc.setFreq(441.f);

    RD_Buffer buffer;
    buffer.setSize(2, 32);
}