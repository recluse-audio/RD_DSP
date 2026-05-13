/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include "SYNTH/Synth.h"
#include "SYNTH/SynthVoice.h"
#include "OSCILLATOR/Oscillator.h"
#include "WAVEFORM/Wavetable.h"
#include "RD_BUFFER/RD_Buffer.h"
#include "RD_BUFFER/BufferFiller.h"

#ifndef RD_DSP_TESTS_DIR
#define RD_DSP_TESTS_DIR "."
#endif

namespace rd_dsp
{
class SynthTester
{
public:
    static std::vector<SynthVoice>& voices (Synth& s) { return s.mSynthVoices; }
    static double sampleRate (const Synth& s) { return s.mSampleRate; }
    static int    blockSize  (const Synth& s) { return s.mBlockSize; }
    static Wavetable& wavetable (Synth& s) { return *s.mWavetable; }

    static SynthVoice* findFreeVoice   (Synth& s)            { return s._findFreeVoice(); }
    static SynthVoice* findActiveVoice (Synth& s, int note)  { return s._findActiveVoice (note); }
};

class SynthVoiceTester
{
public:
    static Oscillator* oscillator (SynthVoice& v) { return v.mOscillator.get(); }
    static double sampleRate (const SynthVoice& v) { return v.mSampleRate; }
    static int    blockSize  (const SynthVoice& v) { return v.mBlockSize; }
    static Wavetable& wavetable (SynthVoice& v) { return v.mWavetable; }

    static double oscillatorSampleRate (SynthVoice& v) { return v.mOscillator->mSampleRate; }
    static int    oscillatorBlockSize  (SynthVoice& v) { return v.mOscillator->mBlockSize; }
    static const Wavetable* oscillatorWavetable (SynthVoice& v) { return &v.mOscillator->mWavetable; }
    static float  oscillatorFrequency  (SynthVoice& v) { return v.mOscillator->mFrequency; }
    static bool   oscillatorIsRunning  (SynthVoice& v) { return v.mOscillator->mIsRunning; }
    static void   resetOscillatorPhase (SynthVoice& v) { v.mOscillator->mCurrentIndex = 0.f; }
};
} // namespace rd_dsp

using rd_dsp::SynthTester;
using rd_dsp::SynthVoiceTester;

TEST_CASE ("Synth::getEngineName returns rd_dsp::Synth", "[Synth]")
{
    rd_dsp::Synth synth;
    REQUIRE (synth.getEngineName() == "rd_dsp::Synth");
}

TEST_CASE ("Synth::setNumVoices adjusts voice count", "[Synth]")
{
    rd_dsp::Synth synth;

    // ctor seeds with 1 voice
    REQUIRE (synth.getNumVoices() == 1);

    synth.setNumVoices (5);
    CHECK (synth.getNumVoices() == 5);

    synth.setNumVoices (10); // upper bound
    CHECK (synth.getNumVoices() == 10);

    synth.setNumVoices (0); // lower bound
    CHECK (synth.getNumVoices() == 0);

    // out-of-range values are ignored; count must not change
    synth.setNumVoices (3);
    REQUIRE (synth.getNumVoices() == 3);

    synth.setNumVoices (-1);
    CHECK (synth.getNumVoices() == 3);

    synth.setNumVoices (11);
    CHECK (synth.getNumVoices() == 3);
}

TEST_CASE ("Synth instantiates voice+oscillator hierarchy sharing wavetable", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (4);

    auto& voices = SynthTester::voices (synth);
    REQUIRE (voices.size() == 4);

    const rd_dsp::Wavetable* sharedWavetable = &SynthTester::wavetable (synth);

    for (auto& voice : voices)
    {
        // voice references the synth's wavetable
        CHECK (&SynthVoiceTester::wavetable (voice) == sharedWavetable);

        // oscillator constructed in voice ctor
        REQUIRE (SynthVoiceTester::oscillator (voice) != nullptr);

        // oscillator references the same wavetable
        CHECK (SynthVoiceTester::oscillatorWavetable (voice) == sharedWavetable);
    }
}

TEST_CASE ("Synth::_findFreeVoice returns first inactive voice or nullptr", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (3);
    auto& voices = SynthTester::voices (synth);

    // all voices inactive on fresh synth -> first voice returned
    rd_dsp::SynthVoice* free0 = SynthTester::findFreeVoice (synth);
    REQUIRE (free0 != nullptr);
    CHECK (free0 == &voices[0]);

    // activate voice 0 -> next free is voice 1
    voices[0].noteOn (60, 1.0f);
    rd_dsp::SynthVoice* free1 = SynthTester::findFreeVoice (synth);
    REQUIRE (free1 != nullptr);
    CHECK (free1 == &voices[1]);

    // activate voice 1 -> next free is voice 2
    voices[1].noteOn (62, 1.0f);
    rd_dsp::SynthVoice* free2 = SynthTester::findFreeVoice (synth);
    REQUIRE (free2 != nullptr);
    CHECK (free2 == &voices[2]);

    // activate all -> nullptr
    voices[2].noteOn (64, 1.0f);
    CHECK (SynthTester::findFreeVoice (synth) == nullptr);

    // free middle voice -> that one returned
    voices[1].noteOff (0.f);
    rd_dsp::SynthVoice* freeMid = SynthTester::findFreeVoice (synth);
    CHECK (freeMid == &voices[1]);
}

TEST_CASE ("Synth::_findActiveVoice returns voice holding the midi note or nullptr", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (3);
    auto& voices = SynthTester::voices (synth);

    // nothing playing -> nullptr for any note
    CHECK (SynthTester::findActiveVoice (synth, 60) == nullptr);

    voices[0].noteOn (60, 1.0f);
    voices[1].noteOn (64, 1.0f);
    voices[2].noteOn (67, 1.0f);

    CHECK (SynthTester::findActiveVoice (synth, 60) == &voices[0]);
    CHECK (SynthTester::findActiveVoice (synth, 64) == &voices[1]);
    CHECK (SynthTester::findActiveVoice (synth, 67) == &voices[2]);

    // unknown note -> nullptr
    CHECK (SynthTester::findActiveVoice (synth, 42) == nullptr);

    // noteOff clears the current note; lookup no longer finds it
    voices[1].noteOff (0.f);
    CHECK (SynthTester::findActiveVoice (synth, 64) == nullptr);
    CHECK (SynthTester::findActiveVoice (synth, 60) == &voices[0]);
    CHECK (SynthTester::findActiveVoice (synth, 67) == &voices[2]);
}

TEST_CASE ("SynthVoice::midiToHertz converts MIDI note numbers to frequencies", "[SynthVoice]")
{
    // A4 = MIDI 69 = 440 Hz (reference)
    CHECK (rd_dsp::SynthVoice::midiToHertz (69) == Catch::Approx (440.0).margin (1e-4));

    // octave relationships
    CHECK (rd_dsp::SynthVoice::midiToHertz (57) == Catch::Approx (220.0).margin (1e-4)); // A3
    CHECK (rd_dsp::SynthVoice::midiToHertz (81) == Catch::Approx (880.0).margin (1e-4)); // A5
    CHECK (rd_dsp::SynthVoice::midiToHertz (45) == Catch::Approx (110.0).margin (1e-4)); // A2

    // middle C
    CHECK (rd_dsp::SynthVoice::midiToHertz (60) == Catch::Approx (261.6255653006).margin (1e-3));

    // semitone above A4 = A#4
    CHECK (rd_dsp::SynthVoice::midiToHertz (70) == Catch::Approx (466.1637615181).margin (1e-3));

    // MIDI 0
    CHECK (rd_dsp::SynthVoice::midiToHertz (0) == Catch::Approx (8.1757989156).margin (1e-4));
}

TEST_CASE ("SynthVoice::noteOn activates voice and stores midi note", "[SynthVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::SynthVoice voice (wavetable);

    // fresh voice inactive, no note
    REQUIRE (voice.isActive() == false);
    REQUIRE (voice.getCurrentActiveNote() == -1);

    voice.noteOn (60, 1.0f);
    CHECK (voice.isActive() == true);
    CHECK (voice.getCurrentActiveNote() == 60);

    // second noteOn overwrites stored note
    voice.noteOn (72, 0.5f);
    CHECK (voice.isActive() == true);
    CHECK (voice.getCurrentActiveNote() == 72);
}

TEST_CASE ("SynthVoice::noteOn sets oscillator frequency from midi note", "[SynthVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::SynthVoice voice (wavetable);

    voice.noteOn (69, 1.0f); // A4
    CHECK (SynthVoiceTester::oscillatorFrequency (voice) == Catch::Approx (440.0f).margin (1e-3));

    voice.noteOn (60, 1.0f); // middle C
    CHECK (SynthVoiceTester::oscillatorFrequency (voice) == Catch::Approx (261.6255653f).margin (1e-2));

    voice.noteOn (81, 0.5f); // A5
    CHECK (SynthVoiceTester::oscillatorFrequency (voice) == Catch::Approx (880.0f).margin (1e-3));
}

TEST_CASE ("SynthVoice::noteOn starts oscillator, noteOff stops it", "[SynthVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::SynthVoice voice (wavetable);

    REQUIRE (SynthVoiceTester::oscillatorIsRunning (voice) == false);

    voice.noteOn (60, 1.0f);
    CHECK (SynthVoiceTester::oscillatorIsRunning (voice) == true);

    voice.noteOff (0.0f);
    CHECK (SynthVoiceTester::oscillatorIsRunning (voice) == false);
}

TEST_CASE ("SynthVoice::noteOff deactivates voice and clears midi note", "[SynthVoice]")
{
    rd_dsp::Wavetable wavetable;
    rd_dsp::SynthVoice voice (wavetable);

    voice.noteOn (64, 1.0f);
    REQUIRE (voice.isActive() == true);
    REQUIRE (voice.getCurrentActiveNote() == 64);

    voice.noteOff (0.0f);
    CHECK (voice.isActive() == false);
    CHECK (voice.getCurrentActiveNote() == -1);

    // noteOff on already-inactive voice stays inactive
    voice.noteOff (0.0f);
    CHECK (voice.isActive() == false);
    CHECK (voice.getCurrentActiveNote() == -1);
}

TEST_CASE ("Synth::noteOn routes to first free voice", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (3);
    auto& voices = SynthTester::voices (synth);

    synth.noteOn (60, 1.0f);
    CHECK (voices[0].isActive() == true);
    CHECK (voices[0].getCurrentActiveNote() == 60);
    CHECK (voices[1].isActive() == false);
    CHECK (voices[2].isActive() == false);

    synth.noteOn (64, 1.0f);
    CHECK (voices[1].isActive() == true);
    CHECK (voices[1].getCurrentActiveNote() == 64);
    CHECK (voices[2].isActive() == false);

    synth.noteOn (67, 1.0f);
    CHECK (voices[2].isActive() == true);
    CHECK (voices[2].getCurrentActiveNote() == 67);
}

TEST_CASE ("Synth::noteOn with no free voice is a no-op (no stealing yet)", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (2);
    auto& voices = SynthTester::voices (synth);

    synth.noteOn (60, 1.0f);
    synth.noteOn (62, 1.0f);
    REQUIRE (voices[0].getCurrentActiveNote() == 60);
    REQUIRE (voices[1].getCurrentActiveNote() == 62);

    // all voices busy -> new note ignored, existing voices untouched
    synth.noteOn (64, 1.0f);
    CHECK (voices[0].getCurrentActiveNote() == 60);
    CHECK (voices[1].getCurrentActiveNote() == 62);
}

TEST_CASE ("Synth::noteOff releases the voice holding that midi note", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (3);
    auto& voices = SynthTester::voices (synth);

    synth.noteOn (60, 1.0f);
    synth.noteOn (64, 1.0f);
    synth.noteOn (67, 1.0f);

    synth.noteOff (64, 0.0f);
    CHECK (voices[0].isActive() == true);
    CHECK (voices[0].getCurrentActiveNote() == 60);
    CHECK (voices[1].isActive() == false);
    CHECK (voices[1].getCurrentActiveNote() == -1);
    CHECK (voices[2].isActive() == true);
    CHECK (voices[2].getCurrentActiveNote() == 67);

    // noteOff for note no voice is holding -> no change
    synth.noteOff (42, 0.0f);
    CHECK (voices[0].isActive() == true);
    CHECK (voices[2].isActive() == true);

    // released voice is reusable
    synth.noteOn (70, 1.0f);
    CHECK (voices[1].isActive() == true);
    CHECK (voices[1].getCurrentActiveNote() == 70);
}

TEST_CASE ("Synth::noteOff on synth with no voices is a no-op", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (0);

    synth.noteOff (60, 0.0f); // must not crash
    SUCCEED();
}

TEST_CASE ("Synth::process matches golden synth sine CSVs across noteOn changes (M69/M76/M81)", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (1);
    synth.prepare (44100.0, 32);

    auto& voices = SynthTester::voices (synth);
    REQUIRE (voices.size() == 1);
    auto& voice = voices[0];

    struct ProcessBlockCall
    {
        int kMidiNote;
        int kCSV_ReadSize;
        const char* kGoldenCSV;
    };

    const ProcessBlockCall processBlockCalls[] =
    {
        { 69, 100, "GOLDEN_SYNTH_SINE_M69_44100SR.csv" }, // 440 Hz
        { 76,  67, "GOLDEN_SYNTH_SINE_M76_44100SR.csv" }, // ~659.26 Hz
        { 81,  50, "GOLDEN_SYNTH_SINE_M81_44100SR.csv" }, // 880 Hz
    };

    constexpr int    processBlockSize = 32;
    constexpr double margin           = 1e-3;
    constexpr int    amplitudeChannel = 2;

    int previousMidiNote = -1;
    for (const auto& call : processBlockCalls)
    {
        // release prior note (if any) so the same voice is free to retrigger
        if (previousMidiNote >= 0)
            synth.noteOff (previousMidiNote, 0.0f);

        synth.noteOn (call.kMidiNote, 1.0f);

        // align phase with golden CSV origin
        SynthVoiceTester::resetOscillatorPhase (voice);

        rd_dsp::RD_Buffer processBuffer (1, processBlockSize);
        synth.process (processBuffer.getReadArray(), processBuffer.getWriteArray(),
                       1, processBlockSize);

        rd_dsp::RD_Buffer csvBuffer (3, call.kCSV_ReadSize);
        const std::string csvPath =
            std::string (RD_DSP_TESTS_DIR) + "/SYNTH/OUTPUT/" + call.kGoldenCSV;
        REQUIRE (rd_dsp::BufferFiller::fillFromCSV (csvPath, csvBuffer));

        for (int i = 0; i < processBlockSize; ++i)
        {
            const float expected = csvBuffer.getSample (amplitudeChannel, i);
            CHECK (processBuffer.getSample (0, i) == Catch::Approx (expected).margin (margin));
        }

        previousMidiNote = call.kMidiNote;
    }
}

TEST_CASE ("Synth::loadWavetable replaces wavetable contents in place", "[Synth]")
{
    rd_dsp::Synth synth;

    // before load, default ctor populates basic shapes; just record the count
    auto& wt = SynthTester::wavetable (synth);
    REQUIRE (wt.getNumWaveforms() > 0);

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";

    synth.loadWavetable (tablePath);

    CHECK (wt.getNumWaveforms() == 4);
    CHECK (wt.getWaveformSize() == 8096);
}

TEST_CASE ("Synth::process at varied wave positions matches per-shape golden CSVs", "[Synth]")
{
    rd_dsp::Synth synth;

    const std::string tablePath =
        std::string (RD_DSP_TESTS_DIR) + "/WAVEFORM/GOLDEN/BASIC_TABLE/GOLDEN_BASIC_WAVEFORM_TABLE_8096.csv";
    synth.loadWavetable (tablePath);

    synth.setNumVoices (1);
    synth.prepare (44100.0, 32);
    synth.noteOn (69, 1.0f); // 440 Hz

    auto& voices = SynthTester::voices (synth);
    REQUIRE (voices.size() == 1);
    auto& voice = voices[0];

    struct WavePosCall
    {
        float kWavePos;
        const char* kGoldenCSV;
        int   kCSV_ReadSize;
    };

    // Basic table mapping (matches WaveFactory test): 0=sine, 0.25=triangle, 0.5=square, 0.75=saw
    const WavePosCall calls[] =
    {
        { 0.00f, "GOLDEN_SYNTH_SINE_M69_44100SR.csv",     100 },
        { 0.25f, "GOLDEN_SYNTH_TRIANGLE_M69_44100SR.csv", 100 },
        { 0.50f, "GOLDEN_SYNTH_SQUARE_M69_44100SR.csv",   100 },
        { 0.75f, "GOLDEN_SYNTH_SAW_M69_44100SR.csv",      100 },
    };

    constexpr int    processBlockSize = 32;
    constexpr double margin           = 1e-3;
    constexpr int    amplitudeChannel = 2;

    for (const auto& call : calls)
    {
        synth.setWavePosition (call.kWavePos);
        SynthVoiceTester::resetOscillatorPhase (voice);

        rd_dsp::RD_Buffer processBuffer (1, processBlockSize);
        synth.process (processBuffer.getReadArray(), processBuffer.getWriteArray(),
                       1, processBlockSize);

        rd_dsp::RD_Buffer csvBuffer (3, call.kCSV_ReadSize);
        const std::string csvPath =
            std::string (RD_DSP_TESTS_DIR) + "/SYNTH/OUTPUT/" + call.kGoldenCSV;
        REQUIRE (rd_dsp::BufferFiller::fillFromCSV (csvPath, csvBuffer));

        for (int i = 0; i < processBlockSize; ++i)
        {
            const float expected = csvBuffer.getSample (amplitudeChannel, i);
            CHECK (processBuffer.getSample (0, i) == Catch::Approx (expected).margin (margin));
        }
    }
}

TEST_CASE ("Synth::prepare propagates to every voice and oscillator", "[Synth]")
{
    rd_dsp::Synth synth;
    synth.setNumVoices (3);

    constexpr double kSampleRate = 48000.0;
    constexpr int    kBlockSize  = 256;
    synth.prepare (kSampleRate, kBlockSize);

    auto& voices = SynthTester::voices (synth);
    REQUIRE (voices.size() == 3);

    for (auto& voice : voices)
    {
        CHECK (SynthVoiceTester::sampleRate (voice) == kSampleRate);
        CHECK (SynthVoiceTester::blockSize  (voice) == kBlockSize);

        REQUIRE (SynthVoiceTester::oscillator (voice) != nullptr);
        CHECK (SynthVoiceTester::oscillatorSampleRate (voice) == kSampleRate);
        CHECK (SynthVoiceTester::oscillatorBlockSize  (voice) == kBlockSize);
    }
}
