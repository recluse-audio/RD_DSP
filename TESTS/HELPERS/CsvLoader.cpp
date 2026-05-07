#include "CsvLoader.h"

#include <fstream>
#include <sstream>

namespace rd_dsp_tests
{

bool loadCsv (const std::string& path,
              std::vector<std::vector<float>>& outRows,
              bool skipHeader)
{
    std::ifstream stream (path);
    if (! stream.is_open())
        return false;

    outRows.clear();

    std::string line;
    bool isFirstLine = true;
    while (std::getline (stream, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            if (skipHeader)
                continue;
        }

        if (line.empty())
            continue;

        std::vector<float> row;
        std::stringstream ss (line);
        std::string cell;
        while (std::getline (ss, cell, ','))
        {
            try
            {
                row.push_back (std::stof (cell));
            }
            catch (...)
            {
                return false;
            }
        }
        outRows.push_back (std::move (row));
    }

    return true;
}

} // namespace rd_dsp_tests
