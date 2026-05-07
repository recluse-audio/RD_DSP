#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include "../../SOURCE/OSCILLATOR/Oscillator.h"
#include "../../SOURCE/RD_BUFFER/RD_Buffer.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

using rd_dsp::Oscillator;
using rd_dsp::Waveform;
using Catch::Approx;

namespace rd_dsp
{
class OscillatorTester
{
public:
    static float phaseIncrement (const Oscillator& o) { return o.mPhaseIncrement; }
    static float currentIndex  (const Oscillator& o) { return o.mCurrentIndex; }
    static double sampleRate    (const Oscillator& o) { return o.mSampleRate; }
    static int    waveformSize  (const Oscillator& o) { return o.mWaveform->getNumSamples(); }

    static float calculatePhaseIncrement (float freq, double sampleRate, int waveformSize)
    {
        return Oscillator::_calculatePhaseIncrement (freq, sampleRate, waveformSize);
    }

    // Test-only knob: resize internal waveform and refresh phase increment.
    // Lets tests pin the waveform size so their arithmetic does not depend on
    // the production default.
    static void setWaveformSize (Oscillator& o, int newSize)
    {
        o.mWaveform->setSize (newSize);
        o.mWaveform->setWaveType (Waveform::WaveType::wSine);
        o._updatePhaseIncrement();
    }
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

// Tests pin waveform size to 2048 via OscillatorTester so their arithmetic
// stays valid regardless of the production default.
// Phase increment formula: inc = freq * waveformSize / sampleRate

TEST_CASE ("Oscillator::setFreq computes phase increment from freq, waveform size, sample rate", "[Oscillator]")
{
    Oscillator osc;
    OscillatorTester::setWaveformSize (osc, 2048);
    osc.prepare (44100.0);
    osc.setFreq (441.f);

    // 441 * 2048 / 44100 = 0.01 * 2048 = 20.48
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));
}

TEST_CASE ("Oscillator phase increment scales with frequency", "[Oscillator]")
{
    Oscillator osc;
    OscillatorTester::setWaveformSize (osc, 2048);
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
    OscillatorTester::setWaveformSize (osc, 2048);
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

TEST_CASE ("Oscillator::_calculatePhaseIncrement matches golden CSV", "[Oscillator]")
{
    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/OSCILLATOR/GOLDEN/PHASE_INCREMENT/GOLDEN_PHASE_INCREMENT_8096.csv";

    std::ifstream in (csvPath);
    REQUIRE (in.is_open());

    std::string line;
    std::getline (in, line); // header

    int rowCount = 0;
    while (std::getline (in, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss (line);
        std::string tok;

        std::getline (ss, tok, ','); float  freq          = std::stof  (tok);
        std::getline (ss, tok, ','); double sampleRate    = std::stod  (tok);
        std::getline (ss, tok, ','); int    waveformSize  = std::stoi  (tok);
        std::getline (ss, tok, ','); float  expectedInc   = std::stof  (tok);

        const float computed = OscillatorTester::calculatePhaseIncrement (freq, sampleRate, waveformSize);
        CHECK (computed == Approx (expectedInc));

        ++rowCount;
    }

    CHECK (rowCount > 0);
}

TEST_CASE("Oscillator writes its waveform to buffer in process()", "[Oscillator]")
{
    // Oscillator osc;
    // osc.prepare(44100.0);
    // osc.setFreq(441.f);

    // int numChannels = 2;
    // int numSamples = 128;

    // rd_dsp::RD_Buffer processBuffer(2, 128);

    // // Confirm all samples are 0's before processing
    // for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    // {
    //     for (int ch = 0; ch < numChannels; ++ch)
    //     {
    //         float sampleValue = processBuffer.getSample(ch, sampleIndex);
    //         CHECK(sampleValue == 0.f);
    //     }
    // }

    // // Confirm mCurrentIndex is at 0.0 before processing
    // CHECK(OscillatorTester::currentIndex(osc) == Approx(0.f));

    // // Do processing, write values to processBuffer
    // osc.process(processBuffer);


    


}