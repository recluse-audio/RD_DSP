/**
 * test_BufferFiller.cpp
 * Tagged [BufferFiller].
 */

#include <catch2/catch_test_macros.hpp>
#include "RD_BUFFER/RD_Buffer.h"
#include "RD_BUFFER/BufferFiller.h"

TEST_CASE ("fillWithAllOnes sets every sample to 1.0f", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (2, 16);

    rd_dsp::BufferFiller::fillWithAllOnes (buffer);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            REQUIRE (buffer.getSample (ch, i) == 1.0f);
}

//-------------------------------------
TEST_CASE ("fillWithAllOnes overwrites prior values", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 8);
    buffer.setSample (0, 3, -0.75f);
    buffer.setSample (0, 5,  0.25f);

    rd_dsp::BufferFiller::fillWithAllOnes (buffer);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == 1.0f);
}

//-------------------------------------
TEST_CASE ("fillWithAllOnes on empty buffer is a no-op", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer;

    rd_dsp::BufferFiller::fillWithAllOnes (buffer);

    REQUIRE (buffer.getNumChannels() == 0);
    REQUIRE (buffer.getNumSamples()  == 0);
}

//-------------------------------------
TEST_CASE ("fillIncremental sets sample[i] == float(i) on every channel", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (2, 32);

    rd_dsp::BufferFiller::fillIncremental (buffer);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            REQUIRE (buffer.getSample (ch, i) == static_cast<float> (i));
}

//-------------------------------------
TEST_CASE ("fillIncremental overwrites prior values", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 8);
    buffer.setSample (0, 3, -0.75f);
    buffer.setSample (0, 5,  0.25f);

    rd_dsp::BufferFiller::fillIncremental (buffer);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == static_cast<float> (i));
}

//-------------------------------------
TEST_CASE ("fillIncremental on empty buffer is a no-op", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer;

    rd_dsp::BufferFiller::fillIncremental (buffer);

    REQUIRE (buffer.getNumChannels() == 0);
    REQUIRE (buffer.getNumSamples()  == 0);
}
