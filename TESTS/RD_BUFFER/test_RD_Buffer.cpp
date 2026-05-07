/**
 * test_RD_Buffer.cpp
 * Tagged [RD_Buffer].
 */

#include <catch2/catch_test_macros.hpp>
#include "RD_BUFFER/RD_Buffer.h"

TEST_CASE ("Default-constructed RD_Buffer reports zero size", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer;

    REQUIRE (buffer.getNumChannels() == 0);
    REQUIRE (buffer.getNumSamples()  == 0);
}

//-------------------------------------
TEST_CASE ("Can set size in channels and samples", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer;
    CHECK (buffer.getNumChannels() == 0);
    CHECK (buffer.getNumSamples()  == 0);

    buffer.setNumChannels (2);
    CHECK (buffer.getNumChannels() == 2);

    buffer.setNumSamples (1024);
    CHECK (buffer.getNumSamples() == 1024);
}

//-------------------------------------
TEST_CASE ("setSize allocates properly and fills with all zeros", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer;
    buffer.setSize (2, 64, true);

    REQUIRE (buffer.getNumChannels() == 2);
    REQUIRE (buffer.getNumSamples()  == 64);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            REQUIRE (buffer.getSample (ch, i) == 0.0f);
}

//-------------------------------------
TEST_CASE ("setSample() / getSample() work", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer (2, 8);
    buffer.setSample (0, 3,  0.5f);
    buffer.setSample (1, 7, -0.25f);

    REQUIRE (buffer.getSample (0, 3) ==  0.5f);
    REQUIRE (buffer.getSample (1, 7) == -0.25f);
    REQUIRE (buffer.getSample (0, 0) ==  0.0f);
}

//-------------------------------------
TEST_CASE ("getWritePointer mutates underlying samples", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer (1, 4);
    float* w = buffer.getWritePointer (0);
    w[0] = 1.0f;
    w[1] = 2.0f;
    w[2] = 3.0f;
    w[3] = 4.0f;

    const float* r = buffer.getReadPointer (0);
    REQUIRE (r[0] == 1.0f);
    REQUIRE (r[3] == 4.0f);
}

//-------------------------------------
TEST_CASE ("clear zeros all samples", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer (2, 4);
    buffer.setSample (0, 0,  1.0f);
    buffer.setSample (1, 3, -1.0f);

    // ensure buffer sample was set properly
    REQUIRE(buffer.getSample(1, 3) == -1.0f);

    buffer.clear();

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            REQUIRE (buffer.getSample (ch, i) == 0.0f);
}

//-------------------------------------
TEST_CASE ("getWriteArray exposes float* per channel", "[RD_Buffer]")
{
    rd_dsp::RD_Buffer buffer (3, 2);
    float* const* arr = buffer.getWriteArray();

    REQUIRE (arr != nullptr);
    arr[0][0] = 0.1f;
    arr[2][1] = 0.9f;

    REQUIRE (buffer.getSample (0, 0) == 0.1f);
    REQUIRE (buffer.getSample (2, 1) == 0.9f);
}
