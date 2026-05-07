/**
 * RD_Buffer.h
 *
 * Framework-agnostic multi-channel sample buffer.
 * Conceptually equivalent to juce::AudioBuffer<float> but with no dependencies.
 *
 * NOTE: API is grown incrementally via TDD. Only members backed by passing tests
 * are declared here. Do not add methods speculatively.
 */

#pragma once

#include <cstddef>

namespace rd
{

class RD_Buffer
{
public:
    RD_Buffer() = default;

    int getNumChannels() const noexcept;
    int getNumSamples()  const noexcept;

private:
    int mNumChannels { 0 };
    int mNumSamples  { 0 };
};

} // namespace rd
