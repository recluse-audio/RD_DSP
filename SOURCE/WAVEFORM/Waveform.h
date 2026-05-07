/**
 * Created on 2026-05-07 by Ryan Devens w/ peace and love
 */

#pragma once
#include "../RD_BUFFER/RD_Buffer.h"

namespace rd_dsp
{
 /**
  * This class represents a waveform,
  * as in a cyclical series of amplitude values.
  * 
  * In this library it is limited by these rules:
  * 1. It is cyclical, so end amp should be nearly the same as starting amp
  * 2. It is normalized, anything above abs(1.0) is treated as clipped
  * 3. It has 2 zero-crossings
  * 
  * Waveform has these responsibilities:
  * - own RD_Buffer which will store the amp values of the waveform
  * - Read values from buffer's int indices based on float read positions.
  * 
  * 
  */
class Waveform
{
public:
    Waveform();
    ~Waveform();


private:
friend class WaveformBufferFiller; // accessess mBuffer and modifies it where noone else would dare
    RD_Buffer mBuffer;
};


} // namespace rd_dsp