/**
 * Created by Ryan Devens 2026-05-24
 */

#include "PulsarData.h"

namespace rd_dsp
{
PulsarData::PulsarData()
{
    formantFreq.setRange (150.0f, 2000.0f);
    formantFreq.setDensity (0.0f);

    wavePosition.setRange (0.0f, 1.0f);
    wavePosition.setDensity (0.0f);

    amp.setRange (0.0f, 1.0f);
    amp.setDensity (0.0f);

    pan.setRange (-1.0f, 1.0f);
    pan.setDensity (0.0f);
}

PulsarParamValues PulsarData::resolve()
{
    PulsarParamValues values;
    values.formantFreq = formantFreq.getRandomizedValue();
    values.wavePosition = wavePosition.getRandomizedValue();
    values.amp = amp.getRandomizedValue();
    values.pan = pan.getRandomizedValue();
    return values;
}

} // namespace rd_dsp
