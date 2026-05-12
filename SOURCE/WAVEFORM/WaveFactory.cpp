#include "WaveFactory.h"
#include "../HELPERS/CsvLoader.h"

#include <vector>

namespace rd_dsp
{

WaveFactory::WaveFactory()  = default;
WaveFactory::~WaveFactory() = default;

std::unique_ptr<Waveform> WaveFactory::loadWaveformFromCSV (std::string csvPath)
{
    std::vector<std::vector<float>> rows;
    if (! CsvLoader::load (csvPath, rows, true))
        return nullptr;

    const int numSamples = static_cast<int> (rows.size());

    auto wave = std::make_unique<Waveform>();
    wave->setSize (numSamples);

    // Amplitude = last column of each row.
    for (int i = 0; i < numSamples; ++i)
    {
        const auto& row = rows[static_cast<std::size_t> (i)];
        if (row.empty())
            return nullptr;
        wave->setSample (i, row.back());
    }

    return wave;
}

} // namespace rd_dsp
