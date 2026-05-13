#include "Synth.h"

#include "../WAVEFORM/Wavetable.h"
#include "../WAVEFORM/WaveFactory.h"
#include "../HELPERS/CsvLoader.h"

#include <cassert>

namespace rd_dsp
{

Synth::Synth()
    : mWavetable (std::make_unique<Wavetable>())
{
    this->setNumVoices(1);
}

Synth::~Synth() = default;

void Synth::prepare (double sampleRate, int maxBlockSize)
{
    mSampleRate = sampleRate; mBlockSize = maxBlockSize;
    for(auto& voice : mSynthVoices)
        voice.prepare(sampleRate, maxBlockSize);
}

void Synth::process (const float* const* readPointers, float* const* writePointers,
                     int numChannels, int numSamples)
{
    for(auto& voice : mSynthVoices)
    {
        if(voice.isActive())
            voice.process(readPointers, writePointers, numChannels, numSamples);
    }
}

void Synth::noteOn (int midiNote, float midiVelocity)
{
    auto* voice = this->_findFreeVoice();
    //if(mAllowNoteStealing) TODO

    // bail out
    if(voice == nullptr)
        return;

    voice->noteOn(midiNote, midiVelocity);
}

void Synth::noteOff (int midiNote, float midiVelocity)
{
    auto* voice = this->_findActiveVoice(midiNote);
    if(voice == nullptr)
        return; // shouldn't end up here

    voice->noteOff(midiVelocity);
}

void Synth::setWavePosition (float wavePos)
{
    if (mWavetable == nullptr)
        return;

    mWavetable->setNormalizedWavePosition (wavePos);
}

void Synth::loadWavetable (std::string tablePath)
{
    assert (mWavetable != nullptr && "Synth::loadWavetable called before mWavetable was initialized");
    if (mWavetable == nullptr)
        return;

    std::vector<std::vector<float>> rows;
    if (! CsvLoader::load (tablePath, rows, false))
        return;

    mWavetable->clear();

    for (const auto& row : rows)
    {
        auto wave = WaveFactory::waveformFromRow (row);
        if (wave == nullptr)
        {
            mWavetable->clear();
            return;
        }
        mWavetable->addWaveform (std::move (wave));
    }
}

void Synth::controlChange (int controlNumber, float normalizedValue)
{
}

void Synth::pitchBend (float signedNormalizedValue)
{
}

int Synth::getNumVoices() const noexcept
{
    return static_cast<int>(mSynthVoices.size());
}

void Synth::setNumVoices(int numVoices)
{
    // too big or too small
    if(numVoices < 0 || numVoices > 10 )
        return;

    assert (mWavetable != nullptr && "Synth::setNumVoices called before mWavetable was initialized");
    if(mWavetable == nullptr)
        return;

    mSynthVoices.clear();
    mSynthVoices.reserve(static_cast<std::size_t>(numVoices));

    for(int i = 0; i < numVoices; i++)
    {
        mSynthVoices.emplace_back(*mWavetable);
    }
}

SynthVoice* Synth::_findFreeVoice()
{
    // return first inactive voice
    for(auto& voice : mSynthVoices)
    {
        if(!voice.isActive())
            return &voice;
    }

    return nullptr;
}

SynthVoice* Synth::_findActiveVoice(int midiNoteNumber)
{
    // return first inactive voice
    for(auto& voice : mSynthVoices)
    {
        if(voice.getCurrentActiveNote() == midiNoteNumber)
            return &voice;
    }

    return nullptr;
}

} // namespace rd_dsp
