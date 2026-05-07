#include "RD_Buffer.h"

#include <cassert>
#include <cstring>

namespace rd_dsp
{

namespace
{
    inline std::size_t roundUpToFour (std::size_t n) noexcept
    {
        return (n + 3u) & ~static_cast<std::size_t> (3u);
    }

    inline std::size_t roundUpToSixteen (std::size_t n) noexcept
    {
        return (n + 15u) & ~static_cast<std::size_t> (15u);
    }
}

//-------------------------------------
RD_Buffer::RD_Buffer (int numChannels, int numSamples)
{
    setSize (numChannels, numSamples, true);
}

//-------------------------------------
RD_Buffer::~RD_Buffer()
{
    _release();
}

//-------------------------------------
void RD_Buffer::setSize (int newNumChannels, int newNumSamples, bool clearExtraSpace)
{
    assert (newNumChannels >= 0);
    assert (newNumSamples  >= 0);

    if (newNumChannels == mNumChannels && newNumSamples == mNumSamples)
    {
        if (clearExtraSpace)
            clear();
        return;
    }

    _release();
    _allocate (newNumChannels, newNumSamples, clearExtraSpace);
}

//-------------------------------------
void RD_Buffer::setNumChannels (int newNumChannels)
{
    setSize (newNumChannels, mNumSamples, true);
}

//-------------------------------------
void RD_Buffer::setNumSamples (int newNumSamples)
{
    setSize (mNumChannels, newNumSamples, true);
}

//-------------------------------------
float RD_Buffer::getSample (int channel, int sampleIndex) const noexcept
{
    assert (channel >= 0 && channel < mNumChannels);
    assert (sampleIndex >= 0 && sampleIndex < mNumSamples);
    return mChannels[channel][sampleIndex];
}

//-------------------------------------
void RD_Buffer::setSample (int channel, int sampleIndex, float value) noexcept
{
    assert (channel >= 0 && channel < mNumChannels);
    assert (sampleIndex >= 0 && sampleIndex < mNumSamples);
    mChannels[channel][sampleIndex] = value;
}

//-------------------------------------
void RD_Buffer::clear() noexcept
{
    if (mChannels == nullptr || mNumChannels <= 0 || mNumSamples <= 0)
        return;

    // Zero only the sample region. The channel-pointer table sits at the
    // start of mAllocatedData and must NOT be zeroed — doing so would
    // null every mChannels[i] and crash subsequent getSample/getReadPointer.
    const std::size_t samplesPerChannel = (static_cast<std::size_t> (mNumSamples) + 3u) & ~static_cast<std::size_t> (3u);
    const std::size_t sampleBytes = static_cast<std::size_t> (mNumChannels) * samplesPerChannel * sizeof (float);
    std::memset (mChannels[0], 0, sampleBytes);
}



//-------------------------------------
const float* RD_Buffer::getReadPointer (int channel) const noexcept
{
    assert (channel >= 0 && channel < mNumChannels);
    return mChannels[channel];
}

//-------------------------------------
float* RD_Buffer::getWritePointer (int channel) noexcept
{
    assert (channel >= 0 && channel < mNumChannels);
    return mChannels[channel];
}

//-------------------------------------
void RD_Buffer::_allocate (int numChannels, int numSamples, bool zeroInit)
{
    if (numChannels <= 0 || numSamples <= 0)
    {
        mNumChannels    = numChannels;
        mNumSamples     = numSamples;
        mChannels       = nullptr;
        mAllocatedData  = nullptr;
        mAllocatedBytes = 0;
        return;
    }

    const std::size_t samplesPerChannel = roundUpToFour (static_cast<std::size_t> (numSamples));
    const std::size_t channelListBytes  = roundUpToSixteen ((static_cast<std::size_t> (numChannels) + 1u) * sizeof (float*));
    const std::size_t sampleBytes       = static_cast<std::size_t> (numChannels) * samplesPerChannel * sizeof (float);
    const std::size_t totalBytes        = channelListBytes + sampleBytes;

    char* raw = zeroInit ? new char[totalBytes]() : new char[totalBytes];

    float** channelArray = reinterpret_cast<float**> (raw);
    float*  firstSample  = reinterpret_cast<float*>  (raw + channelListBytes);

    for (int c = 0; c < numChannels; ++c)
        channelArray[c] = firstSample + (static_cast<std::size_t> (c) * samplesPerChannel);

    channelArray[numChannels] = nullptr; // sentinel matching JUCE convention

    mNumChannels    = numChannels;
    mNumSamples     = numSamples;
    mChannels       = channelArray;
    mAllocatedData  = raw;
    mAllocatedBytes = totalBytes;
}

//-------------------------------------
void RD_Buffer::_release() noexcept
{
    delete[] mAllocatedData;
    mAllocatedData  = nullptr;
    mChannels       = nullptr;
    mNumChannels    = 0;
    mNumSamples     = 0;
    mAllocatedBytes = 0;
}


} // namespace rd_dsp
