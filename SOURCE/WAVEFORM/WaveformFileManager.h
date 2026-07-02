/*
 * This file was created by Ryan Devens on 2026-07-01
 * with peace and love
 */

#pragma once

#include <string>

#include <nlohmann/json.hpp>
#include <rapidcsv.h>

#include "Wavetable.h"

namespace rd_dsp
{

/**
 * This class is meant to manage reading files pertinent to waveforms
 *
 * */
class WaveformFileManager
{
public:

    // this is destructive to contents of wavetable, overwrites
    static void fillFromCSV(rd_dsp::Wavetable& waveTable, const std::string& csvPath)
    {
        waveTable.clear();
        rapidcsv::Document waveCSV(csvPath, rapidcsv::LabelParams(-1, -1));
        std::size_t numWaveforms = waveCSV.GetRowCount();

        // iterate through rows from csv and make a waveform for each
        for(std::size_t index = 0; index < numWaveforms; index++)
        {
            std::vector<double> sampleRow = waveCSV.GetRow<double>(index);
            auto waveform = std::make_unique<rd_dsp::Waveform>();
            if(!fillFromRow(*waveform, sampleRow))
                break;
            waveTable.addWaveform(std::move(waveform));
        }


    }

    // this is destructive to contents of waveform, overwrites
    static bool fillFromCSV(rd_dsp::Waveform& waveform, const std::string& csvPath)
    {
        rapidcsv::Document waveCSV(csvPath, rapidcsv::LabelParams(-1, -1));
        std::size_t numWaveforms = waveCSV.GetRowCount();
        if(numWaveforms == 0)
            return false;

        std::vector<double> sampleRow = waveCSV.GetRow<double>(0);
        return fillFromRow(waveform, sampleRow);
    }

    // this is destructive to contents of waveform, overwrites
    static bool fillFromRow(rd_dsp::Waveform& waveform, const std::vector<double>& samples)
    {
        const int numSamples = static_cast<int>(samples.size());
        if(numSamples <= 0)
            return false;

        waveform.setSize (numSamples);

        for (int i = 0; i < numSamples; ++i)
            waveform.setSample (i, samples[static_cast<std::size_t> (i)]);

        return true;
    }

    //
    static void writeWaveformToCSV(const rd_dsp::Waveform& waveform, const std::string& csvPath)
    {

    }

private:

};

}
