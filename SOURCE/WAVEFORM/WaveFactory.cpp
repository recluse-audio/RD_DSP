#include "WaveFactory.h"
#include "../HELPERS/CsvLoader.h"

#include <vector>

namespace rd_dsp
{

WaveFactory::WaveFactory()  = default;
WaveFactory::~WaveFactory() = default;

std::unique_ptr<Waveform> WaveFactory::waveformFromRow (const std::vector<float>& samples)
{
    const int numSamples = static_cast<int> (samples.size());
    if (numSamples <= 0)
        return nullptr;

    auto wave = std::make_unique<Waveform>();
    wave->setSize (numSamples);

    for (int i = 0; i < numSamples; ++i)
        wave->setSample (i, samples[static_cast<std::size_t> (i)]);

    return wave;
}

std::unique_ptr<Waveform> WaveFactory::loadWaveformFromCSV (std::string csvPath)
{
    std::vector<std::vector<float>> rows;
    if (! CsvLoader::load (csvPath, rows, true))
        return nullptr;

    // Amplitude = last column of each row; each row contributes one sample.
    std::vector<float> samples;
    samples.reserve (rows.size());
    for (const auto& row : rows)
    {
        if (row.empty())
            return nullptr;
        samples.push_back (row.back());
    }

    return waveformFromRow (samples);
}

std::unique_ptr<Wavetable> WaveFactory::loadWavetableFromCSV (std::string csvPath)
{
    std::vector<std::vector<float>> rows;
    if (! CsvLoader::load (csvPath, rows, false))
        return nullptr;

    auto table = std::make_unique<Wavetable>();

    // Each row = one full waveform; columns are samples of that waveform.
    for (const auto& row : rows)
    {
        auto wave = waveformFromRow (row);
        if (wave == nullptr)
            return nullptr;
        table->addWaveform (std::move (wave));
    }

    return table;
}

} // namespace rd_dsp
