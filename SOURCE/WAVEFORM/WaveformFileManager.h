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
        std::size_t numSamples = waveCSV.GetColumnCount();

        // iterate through rows from csv and make a waveform for each
        for(std::size_t index = 0; index < numWaveforms; index++)
        {
            std::vector<double> sampleRow = waveCSV.GetRow<double>(index);
            auto waveform = getWaveformFromRow(sampleRow);
            if(waveform.get() == nullptr)
                break;
            waveTable.addWaveform(std::move(waveform));
        }


    }

    // this is destructive to contents of wavetable, overwrites
    static std::unique_ptr<rd_dsp::Waveform> loadWaveformFromCSV(const std::string& csvPath)
    {
        rapidcsv::Document waveCSV(csvPath, rapidcsv::LabelParams(-1, -1));
        std::size_t numWaveforms = waveCSV.GetRowCount();
        if(numWaveforms == 0)
            return nullptr;

        std::vector<double> sampleRow = waveCSV.GetRow<double>(0);
        return getWaveformFromRow(sampleRow);
    }

    static std::unique_ptr<rd_dsp::Waveform> getWaveformFromRow(const std::vector<double>& samples )
    {
        const int numSamples = static_cast<int>(samples.size());
        if(numSamples <= 0)
            return nullptr;

        auto wave = std::make_unique<rd_dsp::Waveform>();
        wave->setSize (numSamples);

        for (int i = 0; i < numSamples; ++i)
            wave->setSample (i, samples[static_cast<std::size_t> (i)]);

        return wave;
    }

    //
    static void writeWaveformToCSV(const rd_dsp::Waveform& waveform, const std::string& csvPath)
    {

    }

private:

};

}
