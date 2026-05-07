/**
 * test_BufferFiller_fromCSV.cpp
 * Tagged [BufferFiller].
 */

#include <string>

#include <catch2/catch_test_macros.hpp>

#include "RD_BUFFER/RD_Buffer.h"
#include "RD_BUFFER/BufferFiller.h"

TEST_CASE ("fillFromCSV loads All_Ones.csv into a 1ch x 32 buffer", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 32);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == 0.0f);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/BUFFER_FILLER/GOLDEN/All_Ones.csv";

    const bool ok = rd_dsp::BufferFiller::fillFromCSV (csvPath, buffer);
    REQUIRE (ok);

    REQUIRE (buffer.getNumChannels() == 1);
    REQUIRE (buffer.getNumSamples()  == 32);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == 1.0f);
}
