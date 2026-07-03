/**
 * Created on 2026-05-11 by Ryan Devens w/ peace and love
 */

#pragma once
#include "Waveform.h"
#include "Wavetable.h"

#include <memory>
#include <string>
#include <vector>

namespace rd_dsp
{

static const int kMaxAudioFriendlyHarmonics = 16;

//
struct HarmonicData
{
    float ratio = 1.f;
    float gain = 0.125;
    float phaseOffset = 0.f;
};


/*
** Note to self, for sake of sanity the fundamental is harmonic [0]
 */
class WaveFactory
{
public:
    WaveFactory();
    ~WaveFactory();

    // the int harmonic arg here is both an index arg and somewhat a stored param
    // it walks a mirky line but is used safely and intuitively in each scenario
    bool setHarmonicDataValues(int harmonicIndex, float gain, float phaseOffset, float ratio);

    // gets a ref to a given harmonic's data
    const HarmonicData* getHarmonicData(int harmonicIndex);
private:
    // creates HarmonicData for each harmonic sine wave we will create
    void _initHarmonicData();
    // this turns off all harmonics except the fundamental
    void _muteAllHarmonics();
    std::vector<HarmonicData> mHarmonicData;


};

} // namespace rd_dsp
