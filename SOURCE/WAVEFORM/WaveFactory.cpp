#include "WaveFactory.h"
#include "../HELPERS/CsvLoader.h"

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
        harmonicData.harmonic = harmonicIndex;
        harmonicData.phaseOffset = 0.f;
        // for default, only fundamental has gain
        if(harmonicIndex == 0)
            harmonicData.gain = 0.5f;
        else
            harmonicData.gain = 0.0f;

        mHarmonicData.push_back(harmonicData);
    }
}

bool WaveFactory::setHarmonicDataValues(int harmonic, float gain, float phaseOffset)
{
  bool setSuccess = false;

  return setSuccess;
}

const HarmonicData* WaveFactory::getHarmonicData(int harmonic)
{
    if(harmonic >= kMaxAudioFriendlyHarmonics)
        return nullptr;

    return &mHarmonicData[harmonic];

}


} // namespace rd_dsp
