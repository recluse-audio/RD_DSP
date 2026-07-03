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


} // namespace rd_dsp
