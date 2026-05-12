/**
 * Created on 2026-05-11 by Ryan Devens w/ peace and love
 */

#pragma once
#include "Waveform.h"

#include <memory>
#include <vector>

namespace rd_dsp
{

/**
 */
class Wavetable
{
public:
    Wavetable();
    ~Wavetable();

    // given a 0-1 value, this maps between the two currently active waveforms
    void setNormalizedWavePosition(float normalizedWavePos);

    float getSampleAtIndex(float index);

    void addWaveform(std::unique_ptr<Waveform>);

    int getNumWaveforms() const noexcept;
private:
    bool mIsLoading = true;
    std::vector<std::unique_ptr<Waveform>> mWaveforms;
    float mNormalizedWavePos = 0.f;

    // given the mNormalizedWavePos above, what would that be when scaled to mWaveforms.size()
    float _getPositionForWavetableSize();

};

} // namespace rd_dsp
