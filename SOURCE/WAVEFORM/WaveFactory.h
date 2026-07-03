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
    int harmonic = 0;
    float gain = 0.125;
    float phaseOffset = 0.f;
};


//
class WaveFactory
{
public:
    WaveFactory();
    ~WaveFactory();


private:
    // creates HarmonicData for each harmonic sine wave we will create
    void _initHarmonicData();
    // this turns off all harmonics except the fundamental
    void _muteAllHarmonics();
    std::vector<HarmonicData> mHarmonicData;


};

} // namespace rd_dsp
