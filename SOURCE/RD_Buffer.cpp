#include "RD_Buffer.h"

namespace rd
{

int RD_Buffer::getNumChannels() const noexcept
{
    return mNumChannels;
}

int RD_Buffer::getNumSamples() const noexcept
{
    return mNumSamples;
}

} // namespace rd
