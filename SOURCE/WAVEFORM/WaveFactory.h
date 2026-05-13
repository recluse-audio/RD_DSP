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

class WaveFactory
{
public:
    WaveFactory();
    ~WaveFactory();

    std::unique_ptr<Waveform> loadWaveformFromCSV (std::string csvPath);
    std::unique_ptr<Wavetable> loadWavetableFromCSV (std::string csvPath);

    static std::unique_ptr<Waveform> waveformFromRow (const std::vector<float>& samples);
};

} // namespace rd_dsp
