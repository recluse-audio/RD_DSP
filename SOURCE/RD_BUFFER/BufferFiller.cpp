#include "BufferFiller.h"

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

} // namespace rd_dsp
