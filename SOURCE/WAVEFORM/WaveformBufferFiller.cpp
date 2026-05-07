#include "WaveformBufferFiller.h"

#include <cmath>

namespace rd_dsp
{

namespace
{
    constexpr float kTwoPi = 6.28318530717958647692f;
}


WaveformBufferFiller::WaveformBufferFiller()
{
    
}

WaveformBufferFiller::~WaveformBufferFiller()
{
    
}

void WaveformBufferFiller::fillWaveformBuffer(Waveform& waveform, WaveType waveType)
{
    switch(waveType)
    {
        case WaveType::wSine:
            _fillWithSine(waveform.mBuffer);
            break;
        case WaveType::wTri:
            _fillWithTri(waveform.mBuffer);
            break;
        case WaveType::wSquare:
            _fillWithSquare(waveform.mBuffer);
            break;   
        case WaveType::wSaw:
            _fillWithSaw(waveform.mBuffer);
            break; 
    }
}

void WaveformBufferFiller::_fillWithSine(RD_Buffer& waveformBuffer)
{
    waveformBuffer.clear();
    const int numChannels = waveformBuffer.getNumChannels();
    const int numSamples  = waveformBuffer.getNumSamples();
    if (numChannels <= 0 || numSamples <= 0)
        return;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const float phase  = (static_cast<float>(sampleIndex) * kTwoPi) / static_cast<float>(numSamples);
        const float sample = std::sin(phase);
        for (int channel = 0; channel < numChannels; ++channel)
        {
            waveformBuffer.setSample(channel, sampleIndex, sample);
        }
    }
}


void WaveformBufferFiller::_fillWithTri(RD_Buffer& waveformBuffer)
{

}

void WaveformBufferFiller::_fillWithSquare(RD_Buffer& waveformBuffer)
{

}

void WaveformBufferFiller::_fillWithSaw(RD_Buffer& waveformBuffer)
{

}

} // namespace rd_dsp