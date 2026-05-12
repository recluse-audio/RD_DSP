/**
 * Created on 2026-05-11 by Ryan Devens w/ peace and love
 */

#pragma once

#include <string>
#include <vector>

namespace rd_dsp
{

class CsvLoader
{
public:
    static bool load (const std::string& path,
                      std::vector<std::vector<float>>& outRows,
                      bool skipHeader = true);
};

} // namespace rd_dsp
