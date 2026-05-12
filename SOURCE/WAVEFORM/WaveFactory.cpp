#include "WaveFactory.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace rd_dsp
{

WaveFactory::WaveFactory()  = default;
WaveFactory::~WaveFactory() = default;

std::unique_ptr<Waveform> WaveFactory::loadWaveformFromCSV (std::string csvPath)
{
    std::ifstream file (csvPath);
    if (! file.is_open())
        return nullptr;

    std::vector<float> amplitudes;
    std::string        line;
    bool               isFirstLine = true;

    while (std::getline (file, line))
    {
        if (line.empty())
            continue;

        if (isFirstLine)
        {
            isFirstLine = false;
            // Skip header if last token is non-numeric.
            try
            {
                const auto lastComma = line.find_last_of (',');
                const auto tail      = (lastComma == std::string::npos)
                                           ? line
                                           : line.substr (lastComma + 1);
                (void) std::stof (tail);
            }
            catch (...)
            {
                continue;
            }
        }

        // Amplitude = last column.
        const auto  lastComma = line.find_last_of (',');
        const auto  tail      = (lastComma == std::string::npos)
                                    ? line
                                    : line.substr (lastComma + 1);
        amplitudes.push_back (std::stof (tail));
    }

    auto wave = std::make_unique<Waveform>();
    wave->setSize (static_cast<int> (amplitudes.size()));

    for (int i = 0; i < static_cast<int> (amplitudes.size()); ++i)
        wave->setSample (i, amplitudes[static_cast<size_t> (i)]);

    return wave;
}

} // namespace rd_dsp
