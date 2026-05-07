#include "BufferFiller.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace rd_dsp
{

void BufferFiller::fillWithAllOnes (RD_Buffer& bufferToFill) noexcept
{
    const int numChannels = bufferToFill.getNumChannels();
    const int numSamples  = bufferToFill.getNumSamples();

    float* const* writePtr = bufferToFill.getWriteArray();

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            writePtr[ch][sampleIndex] = 1.0f;
        }
    }
}

//-------------------------------------
bool BufferFiller::fillFromCSV (const std::string& csvPath, RD_Buffer& bufferToFill) noexcept
{
    try
    {
        std::ifstream stream (csvPath);
        if (! stream.is_open())
            return false;

        std::vector<std::vector<float>> rows;
        std::string line;
        bool isFirstLine = true;
        while (std::getline (stream, line))
        {
            if (isFirstLine)
            {
                isFirstLine = false;
                continue; // skip header
            }
            if (line.empty())
                continue;

            std::vector<float> row;
            std::stringstream ss (line);
            std::string cell;
            while (std::getline (ss, cell, ','))
                row.push_back (std::stof (cell));

            rows.push_back (std::move (row));
        }

        const int numSamples  = bufferToFill.getNumSamples();
        const int numChannels = bufferToFill.getNumChannels();

        if (static_cast<int> (rows.size()) != numSamples)
            return false;
        for (const auto& r : rows)
            if (static_cast<int> (r.size()) != numChannels)
                return false;

        float* const* writePtr = bufferToFill.getWriteArray();
        for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
            for (int ch = 0; ch < numChannels; ++ch)
                writePtr[ch][sampleIndex] = rows[static_cast<std::size_t> (sampleIndex)][static_cast<std::size_t> (ch)];

        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace rd_dsp
