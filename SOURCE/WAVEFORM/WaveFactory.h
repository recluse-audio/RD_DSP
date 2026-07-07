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
static const int kDefaultWaveformSize = 8192;
constexpr float kTwoPi = 6.28318530717958647692f;

// Shared with the Python golden generator (gen_golden_waveform.py). Keep in sync.
constexpr float kDefaultTargetRMS   = 0.70710678118654752f; // 1 / sqrt(2)
constexpr float kDefaultPeakCeiling = 0.95f;                // tanh soft-clip ceiling
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

    void fillWaveformWithHarmonics(rd_dsp::Waveform& waveform);

    // Scaling stages, applied on top of the RAW harmonic-sum waveform. Each matches
    // gen_golden_waveform.py step-for-step: compute in double, narrow to float32 on store.
    // RMS-normalize the whole waveform to targetRms (no-op if RMS is zero).
    void rmsScale(rd_dsp::Waveform& waveform, float targetRms = kDefaultTargetRMS);
    // tanh soft-clip each sample: ceiling * tanh(s / ceiling).
    void peakScale(rd_dsp::Waveform& waveform, float ceiling = kDefaultPeakCeiling);
private:
    // this is used to keep waveforms within a desirable rms
    float mNormalizeCoefficient = 1.f;
    // generates a sine wave based on the given harmonic's data vals
    // writes ADDITIVELY to waveform
    void _writeHarmonicToWaveform(rd_dsp::Waveform& waveform, int harmonic);

    // creates HarmonicData for each harmonic sine wave we will create
    void _initHarmonicData();
    // this turns off all harmonics except the fundamental
    void _muteAllHarmonics();
    std::vector<HarmonicData> mHarmonicData;


};

} // namespace rd_dsp
