/**
 * Created by Ryan Devens on 2026-05-07 with peace & love
 */

#pragma once

#include <cstddef>

namespace rd_dsp
{

/**
 * This class is meant to be my juce-agnostic version of the juce::AudioBuffer.
 * I am creating this so I can use the class without the juce library, like for a web synth.
 * 
 * This is meant to be overly simplified and at the moment is not templated, float only!
 */
class RD_Buffer
{
public:
    RD_Buffer() noexcept = default;
    RD_Buffer (int numChannels, int numSamples);

    ~RD_Buffer();

    RD_Buffer (const RD_Buffer&)            = delete;
    RD_Buffer& operator= (const RD_Buffer&) = delete;
    RD_Buffer (RD_Buffer&&)                 = delete;
    RD_Buffer& operator= (RD_Buffer&&)      = delete;

    // Resize, single allocation. Reallocates whenever shape changes.
    void setSize (int newNumChannels, int newNumSamples, bool clearExtraSpace = false);

    // Convenience wrappers preserving the original spec'd API. Each forwards to setSize.
    void setNumChannels (int newNumChannels);
    void setNumSamples  (int newNumSamples);

    int getNumChannels() const noexcept { return mNumChannels; }
    int getNumSamples()  const noexcept { return mNumSamples;  }

    float getSample (int channel, int sampleIndex) const noexcept;
    void  setSample (int channel, int sampleIndex, float value) noexcept;

    const float* getReadPointer  (int channel) const noexcept;
    float*       getWritePointer (int channel)       noexcept;

    const float* const* getReadArray()  const noexcept { return mChannels; }
    float* const*       getWriteArray()       noexcept { return mChannels; }

    // Zero every sample. Preserves channel/sample counts.
    void clear() noexcept;

private:
    void _allocate (int numChannels, int numSamples, bool zeroInit);
    void _release() noexcept;

    int mNumChannels { 0 };
    int mNumSamples { 0 };
    float** mChannels { nullptr };
    char* mAllocatedData { nullptr };
    std::size_t mAllocatedBytes { 0 };
};

} // namespace rd_dsp
