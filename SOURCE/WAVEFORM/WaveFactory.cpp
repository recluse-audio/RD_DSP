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
            harmonicData.gain = 0.f;

        mHarmonicData.push_back(harmonicData);
    }
}


} // namespace rd_dsp
