/**
 * Created on 2026-05-13 by Ryan Devens with peace and love
 */

#include <catch2/catch_test_macros.hpp>

#include "SYNTH/Synth.h"
#include "SYNTH/SynthVoice.h"
#include "OSCILLATOR/Oscillator.h"
#include "WAVEFORM/Wavetable.h"

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
