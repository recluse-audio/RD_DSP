/**
 * Created on 2026-05-11 by Ryan Devens w/ peace and love
 */

#pragma once
#include "Waveform.h"

#include <memory>
#include <string>

namespace rd_dsp
{

class WaveFactory
{
public:
    WaveFactory();
    ~WaveFactory();

    std::unique_ptr<Waveform> loadWaveformFromCSV (std::string csvPath);
};

} // namespace rd_dsp
