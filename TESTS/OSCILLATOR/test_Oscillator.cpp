#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include "../../SOURCE/OSCILLATOR/Oscillator.h"
#include "../../SOURCE/WAVEFORM/Wavetable.h"
#include "../../SOURCE/RD_BUFFER/RD_Buffer.h"
#include "../../SOURCE/RD_BUFFER/BufferFiller.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

using rd_dsp::Oscillator;
using rd_dsp::Wavetable;
using Catch::Approx;

namespace rd_dsp
{
class OscillatorTester
{
public:
    static float phaseIncrement (const Oscillator& o) { return o.mPhaseIncrement; }
    static float currentIndex  (const Oscillator& o) { return o.mCurrentIndex; }
    static double sampleRate    (const Oscillator& o) { return o.mSampleRate; }
    static int    waveformSize  (const Oscillator& o) { return o.mWavetable.getWaveformSize(); }
    static int    blockSize     (const Oscillator& o) { return o.mBlockSize; }
    static bool   isRunning     (const Oscillator& o) { return o.mIsRunning; }
    static bool   phaseIncrementUpdateNeeded (const Oscillator& o) { return o.mPhaseIncrementUpdateNeeded; }
    static void   resetPhase    (Oscillator& o) { o.mCurrentIndex = 0.f; }

    static float calculatePhaseIncrement (float freq, double sampleRate, int waveformSize)
    {
        return Oscillator::_calculatePhaseIncrement (freq, sampleRate, waveformSize);
    }

    // Test-only knob: resize wavetable contents and refresh phase increment.
    // Lets tests pin the waveform size so their arithmetic does not depend on
    // the production default. Refills with basic shapes; slot 0 stays sine.
    static void setWaveformSize (Oscillator& o, int newSize)
    {
        o.mWavetable.fillWithBasicShapes (newSize);
        o._updatePhaseIncrement();
    }
};
} // namespace rd_dsp

using rd_dsp::OscillatorTester;

TEST_CASE ("Oscillator default constructs with sine waveform", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    CHECK (OscillatorTester::waveformSize (osc) > 0);
    CHECK (OscillatorTester::phaseIncrement (osc) == 0.f);
}

TEST_CASE ("Oscillator::prepare stores sample rate and block size", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare (48000.0, 256);
    CHECK (OscillatorTester::sampleRate (osc) == 48000.0);
    CHECK (OscillatorTester::blockSize  (osc) == 256);
}

// Tests pin waveform size to 2048 via OscillatorTester so their arithmetic
// stays valid regardless of the production default.
// Phase increment formula: inc = freq * waveformSize / sampleRate

// Helper: trigger deferred phase-increment update by running a single-sample
// process. setFreq only flags the update; process performs it.
static void flushPhaseIncrement (Oscillator& osc)
{
    osc.start();
    rd_dsp::RD_Buffer scratch (1, 1);
    osc.process (scratch);
    osc.stop();
}

TEST_CASE ("Oscillator::setFreq flags phase increment update without applying it", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    OscillatorTester::setWaveformSize (osc, 2048);
    osc.prepare (44100.0, 1);
    osc.setFreq (441.f);

    CHECK (OscillatorTester::phaseIncrementUpdateNeeded (osc));
    // increment unchanged from prepare-time value (0 here, since freq was 0)
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (0.f));
}

TEST_CASE ("Oscillator::setFreq computes phase increment from freq, waveform size, sample rate", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    OscillatorTester::setWaveformSize (osc, 2048);
    osc.prepare (44100.0, 1);
    osc.setFreq (441.f);
    flushPhaseIncrement (osc);

    // 441 * 2048 / 44100 = 0.01 * 2048 = 20.48
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));
    CHECK_FALSE (OscillatorTester::phaseIncrementUpdateNeeded (osc));
}

TEST_CASE ("Oscillator phase increment scales with frequency", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    OscillatorTester::setWaveformSize (osc, 2048);
    osc.prepare (44100.0, 1);

    // 441 * 2048 / 44100 = 20.48
    osc.setFreq (441.f);
    flushPhaseIncrement (osc);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));

    // 882 * 2048 / 44100 = 40.96
    osc.setFreq (882.f);
    flushPhaseIncrement (osc);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (40.96f));
}

TEST_CASE ("Oscillator::prepare after setFreq updates phase increment for new sample rate", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    OscillatorTester::setWaveformSize (osc, 2048);
    osc.prepare (44100.0, 1);
    osc.setFreq (441.f);
    flushPhaseIncrement (osc);
    // 441 * 2048 / 44100 = 20.48
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (20.48f));

    // halve the SR -> increment doubles. 441 * 2048 / 22050 = 40.96
    osc.prepare (22050.0, 1);
    CHECK (OscillatorTester::phaseIncrement (osc) == Approx (40.96f));

    // double original SR -> increment halves. 441 * 2048 / 88200 = 10.24
    osc.prepare (88200.0, 1);
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
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare(44100.0, 128);
    osc.setFreq(441.f); // don't change, this refs a golden file
    osc.start();

    int numChannels = 2;
    int numSamples = 128;

    rd_dsp::RD_Buffer processBuffer(2, 128);

    // Confirm all samples are 0's before processing
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sampleValue = processBuffer.getSample(ch, sampleIndex);
            CHECK(sampleValue == 0.f);
        }
    }

    // Confirm mCurrentIndex is at 0.0 before processing
    CHECK(OscillatorTester::currentIndex(osc) == Approx(0.f));

    // Do processing, write values to processBuffer
    osc.process(processBuffer);

    // Golden CSV: 3 columns (index, phase_index, amplitude) x 100 sample rows.
    // Load directly into a 3ch x 100 buffer; compare amplitude column (ch 2)
    // against the first 100 samples of every channel in processBuffer.
    const int csvNumChannels = 3;
    const int csvNumSamples  = 100;
    rd_dsp::RD_Buffer csvBuffer (csvNumChannels, csvNumSamples);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/OSCILLATOR/GOLDEN/SINE/GOLDEN_OSC_SINE_441Hz_44100SR.csv";

    const bool loaded = rd_dsp::BufferFiller::fillFromCSV (csvPath, csvBuffer);
    REQUIRE (loaded);

    constexpr int amplitudeChannel = 2;
    constexpr double margin = 1e-3;
    for (int i = 0; i < csvNumSamples; ++i)
    {
        const float expected = csvBuffer.getSample (amplitudeChannel, i);
        for (int ch = 0; ch < numChannels; ++ch)
            CHECK (processBuffer.getSample (ch, i) == Approx (expected).margin (margin));
    }
}

TEST_CASE("Oscillator writes its waveform via raw pointer overload of process()", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare(44100.0, 128);
    osc.setFreq(441.f); // don't change, this refs a golden file
    osc.start();

    int numChannels = 2;
    int numSamples = 128;

    rd_dsp::RD_Buffer processBuffer(numChannels, numSamples);

    // Confirm all samples are 0's before processing
    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float sampleValue = processBuffer.getSample(ch, sampleIndex);
            CHECK(sampleValue == 0.f);
        }
    }

    // Confirm mCurrentIndex is at 0.0 before processing
    CHECK(OscillatorTester::currentIndex(osc) == Approx(0.f));

    // Drive the pointer-based overload directly.
    osc.process(processBuffer.getReadArray(), processBuffer.getWriteArray(), numChannels, numSamples);

    const int csvNumChannels = 3;
    const int csvNumSamples  = 100;
    rd_dsp::RD_Buffer csvBuffer (csvNumChannels, csvNumSamples);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/OSCILLATOR/GOLDEN/SINE/GOLDEN_OSC_SINE_441Hz_44100SR.csv";

    const bool loaded = rd_dsp::BufferFiller::fillFromCSV (csvPath, csvBuffer);
    REQUIRE (loaded);

    constexpr int amplitudeChannel = 2;
    constexpr double margin = 1e-3;
    for (int i = 0; i < csvNumSamples; ++i)
    {
        const float expected = csvBuffer.getSample (amplitudeChannel, i);
        for (int ch = 0; ch < numChannels; ++ch)
            CHECK (processBuffer.getSample (ch, i) == Approx (expected).margin (margin));
    }
}

//-------------------------------------
TEST_CASE ("Oscillator default is stopped", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    CHECK_FALSE (OscillatorTester::isRunning (osc));
}

//-------------------------------------
TEST_CASE ("Oscillator::start sets running state", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.start();
    CHECK (OscillatorTester::isRunning (osc));
}

//-------------------------------------
TEST_CASE ("Oscillator::stop clears running state", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.start();
    REQUIRE (OscillatorTester::isRunning (osc));

    osc.stop();
    CHECK_FALSE (OscillatorTester::isRunning (osc));
}

//-------------------------------------
TEST_CASE ("Oscillator::process is a no-op when stopped", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare (44100.0, 64);
    osc.setFreq (441.f);

    rd_dsp::RD_Buffer buffer (2, 64);
    osc.process (buffer); // not started

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            CHECK (buffer.getSample (ch, i) == 0.f);
}

//-------------------------------------
TEST_CASE ("Oscillator::process writes samples after start", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare (44100.0, 64);
    osc.setFreq (441.f);
    osc.start();

    rd_dsp::RD_Buffer buffer (2, 64);
    osc.process (buffer);

    bool anyNonZero = false;
    for (int ch = 0; ch < buffer.getNumChannels() && ! anyNonZero; ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            if (buffer.getSample (ch, i) != 0.f)
            {
                anyNonZero = true;
                break;
            }

    CHECK (anyNonZero);
}

//-------------------------------------
TEST_CASE ("Oscillator::process is a no-op after stop following start", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare (44100.0, 64);
    osc.setFreq (441.f);
    osc.start();
    osc.stop();

    rd_dsp::RD_Buffer buffer (2, 64);
    osc.process (buffer);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            CHECK (buffer.getSample (ch, i) == 0.f);
}

//-------------------------------------
TEST_CASE ("Oscillator process across freq changes matches golden CSVs (441 -> 663 -> 882)", "[Oscillator]")
{
    Wavetable wt;
    Oscillator osc (wt);
    osc.prepare (44100.0, 32);
    osc.start();

    struct ProcessBlockCall
    {
        float kFreq;
        float kIncrement;
        int kCSV_ReadSize;
        const char* kGoldenCSV;
    }; 

    const ProcessBlockCall processBlockCalls[] =
    {
        { 441.f, 80.96f, 100, "GOLDEN_OSC_SINE_441Hz_44100SR.csv" },
        { 663.f, 121.71542f, 67, "GOLDEN_OSC_SINE_663Hz_44100SR.csv" },
        { 882.f, 161.92f, 50, "GOLDEN_OSC_SINE_882Hz_44100SR.csv" },
    };

    constexpr int    processBlockSize = 32;
    constexpr double margin           = 1e-3;
    constexpr int    amplitudeChannel = 2;

    float previousIncrement = OscillatorTester::phaseIncrement (osc); // 0 prior to any setFreq

    for (const auto& processBlockCall : processBlockCalls)
    {
        osc.setFreq (processBlockCall.kFreq);

        // Flag set, increment not yet applied.
        CHECK (OscillatorTester::phaseIncrementUpdateNeeded (osc));
        CHECK (OscillatorTester::phaseIncrement (osc) == Approx (previousIncrement));

        // Reset phase so each block starts from index 0, matching golden CSV origin.
        OscillatorTester::resetPhase (osc);

        rd_dsp::RD_Buffer processBuffer (1, processBlockSize);
        osc.process (processBuffer);

        // process() applied the increment and cleared the flag.
        CHECK_FALSE (OscillatorTester::phaseIncrementUpdateNeeded (osc));
        CHECK (OscillatorTester::phaseIncrement (osc) == Approx (processBlockCall.kIncrement));

        // Load golden CSV (3 cols x csvSamples rows) into a matching buffer.
        rd_dsp::RD_Buffer csvBuffer (3, processBlockCall.kCSV_ReadSize);
        const std::string csvPath =
            std::string (RD_DSP_TESTS_DIR) + "/OSCILLATOR/GOLDEN/SINE/" + processBlockCall.kGoldenCSV;
        REQUIRE (rd_dsp::BufferFiller::fillFromCSV (csvPath, csvBuffer));

        for (int i = 0; i < processBlockSize; ++i)
        {
            const float expected = csvBuffer.getSample (amplitudeChannel, i);
            CHECK (processBuffer.getSample (0, i) == Approx (expected).margin (margin));
        }

        previousIncrement = processBlockCall.kIncrement;
    }

}