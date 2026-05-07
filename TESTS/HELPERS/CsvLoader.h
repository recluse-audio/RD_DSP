/**
 * Test-side CSV loader. NOT part of core lib.
 * Skips a single header row and parses the rest as floats.
 */

#pragma once

#include <string>
#include <vector>

namespace rd_dsp_tests
{

bool loadCsv (const std::string& path,
              std::vector<std::vector<float>>& outRows,
              bool skipHeader = true);

} // namespace rd_dsp_tests
