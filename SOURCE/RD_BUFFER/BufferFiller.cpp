#include "BufferFiller.h"
#include "../HELPERS/CsvLoader.h"

#include <vector>

namespace rd_dsp
{

//-------------------------------------
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
void BufferFiller::fillIncremental (RD_Buffer& bufferToFill) noexcept
{
    const int numChannels = bufferToFill.getNumChannels();
    const int numSamples  = bufferToFill.getNumSamples();

    float* const* writePtr = bufferToFill.getWriteArray();

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float value = static_cast<float> (sampleIndex);
        for (int ch = 0; ch < numChannels; ++ch)
        {
            writePtr[ch][sampleIndex] = value;
        }
    }
}

//-------------------------------------
bool BufferFiller::fillFromCSV (const std::string& csvPath, RD_Buffer& bufferToFill) noexcept
{
    try
    {
        std::vector<std::vector<float>> rows;
        if (! CsvLoader::load (csvPath, rows, true))
            return false;

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

//-------------------------------------
bool BufferFiller::fillFromCSV (const std::string& csvPath, RD_Buffer& bufferToFill, int startIndex) noexcept
{
    if (startIndex < 0)
        return false;

    try
    {
        std::vector<std::vector<float>> rows;
        if (! CsvLoader::load (csvPath, rows, true))
            return false;

        const int numSamples  = bufferToFill.getNumSamples();
        const int numChannels = bufferToFill.getNumChannels();
        const int csvSamples  = static_cast<int> (rows.size());

        for (const auto& r : rows)
            if (static_cast<int> (r.size()) != numChannels)
                return false;

        if (startIndex >= csvSamples)
            return true; // nothing to copy, buffer untouched

        const int available = csvSamples - startIndex;
        const int copyCount = available < numSamples ? available : numSamples;

        float* const* writePtr = bufferToFill.getWriteArray();
        for (int i = 0; i < copyCount; ++i)
        {
            const auto& row = rows[static_cast<std::size_t> (startIndex + i)];
            for (int ch = 0; ch < numChannels; ++ch)
                writePtr[ch][i] = row[static_cast<std::size_t> (ch)];
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace rd_dsp
