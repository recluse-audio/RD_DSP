/**
 * Created by Ryan Devens on 2026-05-07 with peace & love
 */

#pragma once

#include <string>

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

    // Read a CSV (rows = samples, cols = channels, single header row skipped) and
    // copy values into bufferToFill. Buffer dimensions must already match the CSV
    // shape. Returns true on success, false if the file cannot be opened, parsed,
    // or its shape disagrees with the buffer. Not for use on the audio hot path.
    static bool fillFromCSV (const std::string& csvPath, RD_Buffer& bufferToFill) noexcept;
};

} // namespace rd_dsp
