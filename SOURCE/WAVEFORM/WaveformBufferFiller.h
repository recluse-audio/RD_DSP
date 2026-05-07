/**
 * Created on 2026-05-07 by Ryan Devens w/ peace and love
 */

#pragma once
#include "../RD_BUFFER/RD_Buffer.h"
#include "Waveform.h"

namespace rd_dsp
{

 /**
  * This class fills a Waveform's buffer with a "proper" waveform in varied ways.
  */
class WaveformBufferFiller
{
public:
    WaveformBufferFiller();
    ~WaveformBufferFiller();

    enum class WaveType
    {
        wSine = 0,
        wTri = 1,
        wSquare = 2,
        wSaw = 3
    };

    void fillWaveformBuffer(Waveform& waveform, WaveType waveType);

private:
friend class WaveformBufferTester;// for testing duh
    void _fillWithSine(RD_Buffer& waveformBuffer);
    void _fillWithTri(RD_Buffer& waveformBuffer);
    void _fillWithSquare(RD_Buffer& waveformBuffer);
    void _fillWithSaw(RD_Buffer& waveformBuffer);
    
};


} // namespace rd_dsp