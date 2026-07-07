#include "WaveFactory.h"
#include "../HELPERS/CsvLoader.h"

#include <cmath>
#include <vector>

namespace rd_dsp
{


WaveFactory::WaveFactory()
{
    _initHarmonicData();
}

WaveFactory::~WaveFactory() = default;

void WaveFactory::_initHarmonicData()
{
    mHarmonicData.reserve(kMaxAudioFriendlyHarmonics);
    for(int harmonicIndex = 0; harmonicIndex < kMaxAudioFriendlyHarmonics; harmonicIndex++)
    {
        HarmonicData harmonicData;
        harmonicData.ratio = (float)(harmonicIndex + 1);
        harmonicData.phaseOffset = 0.f;
        // for default, only fundamental has gain
        if(harmonicIndex == 0)
            harmonicData.gain = 0.5f;
        else
            harmonicData.gain = 0.0f;

        mHarmonicData.push_back(harmonicData);
    }
}

bool WaveFactory::setHarmonicDataValues(int harmonicIndex, float gain, float phaseOffset, float ratio)
{
    // don't even bother if out of range
    if(harmonicIndex >= kMaxAudioFriendlyHarmonics )
      return false;

   auto& harmonicData = mHarmonicData[harmonicIndex];
   harmonicData.ratio = ratio;
   harmonicData.gain = gain;
   harmonicData.phaseOffset = phaseOffset;

   return true;
}

const HarmonicData* WaveFactory::getHarmonicData(int harmonicIndex)
{
    if(harmonicIndex >= kMaxAudioFriendlyHarmonics)
        return nullptr;

    return &mHarmonicData[harmonicIndex];

}

void WaveFactory::fillWaveformWithHarmonics(rd_dsp::Waveform& waveform)
{

    for(int harmonicIndex = 0; harmonicIndex < kMaxAudioFriendlyHarmonics; harmonicIndex++)
    {
        _writeHarmonicToWaveform(waveform, harmonicIndex);
    }
}

void WaveFactory::_writeHarmonicToWaveform(rd_dsp::Waveform& waveform, int harmonicIndex)
{
    const rd_dsp::HarmonicData* harmonicData = getHarmonicData(harmonicIndex);

    for(int sampleIndex = 0; sampleIndex < kDefaultWaveformSize; sampleIndex++)
    {
        const float phasePos = (kTwoPi * harmonicData->ratio * (float)sampleIndex) / (float)kDefaultWaveformSize;
        const float harmonicSample = std::sin(phasePos + harmonicData->phaseOffset) * harmonicData->gain;

        // This is the summed sample val at this sampleIndex before adding this harmonicSample
        const float prevSampleValue = waveform.getInterpolatedSampleAtIndex(sampleIndex);

        // this is the new value in waveform at given index after summing prev and new harmonic sample
        const float calculatedSample = prevSampleValue + harmonicSample;

        waveform.setSample(sampleIndex, calculatedSample);
    }
}

void WaveFactory::rmsScale(rd_dsp::Waveform& waveform, float targetRms)
{
    const int numSamples = waveform.getNumSamples();
    if(numSamples <= 0)
        return;

    // Compute RMS in double to match the Python golden generator.
    double sumOfSquares = 0.0;
    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
    {
        const double sample = static_cast<double>(waveform.getSample(sampleIndex));
        sumOfSquares += sample * sample;
    }

    const double rms = std::sqrt(sumOfSquares / static_cast<double>(numSamples));
    if(rms == 0.0)
        return;

    const double scale = static_cast<double>(targetRms) / rms;
    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
    {
        const double scaled = static_cast<double>(waveform.getSample(sampleIndex)) * scale;
        waveform.setSample(sampleIndex, static_cast<float>(scaled));
    }
}

void WaveFactory::peakScale(rd_dsp::Waveform& waveform, float ceiling)
{
    const int numSamples = waveform.getNumSamples();
    const double c = static_cast<double>(ceiling);
    for(int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++)
    {
        const double sample = static_cast<double>(waveform.getSample(sampleIndex));
        const double clipped = c * std::tanh(sample / c);
        waveform.setSample(sampleIndex, static_cast<float>(clipped));
    }
}

} // namespace rd_dsp
