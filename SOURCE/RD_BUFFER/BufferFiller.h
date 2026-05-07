/**
 * Created by Ryan Devens on 2026-05-07 with peace & love
 */

#pragma once

#include "RD_Buffer.h"

namespace rd_dsp
{

/**
 * @brief Functions that fill an RD_Buffer with various amplitude patterns.
 *
 * JUCE-free port of the BufferFiller helpers from the RD project. Only
 * fillWithAllOnes is implemented for now; more will be ported as needed.
 */
class BufferFiller
{
public:
    static void fillWithAllOnes (RD_Buffer& bufferToFill) noexcept;
};

} // namespace rd_dsp
