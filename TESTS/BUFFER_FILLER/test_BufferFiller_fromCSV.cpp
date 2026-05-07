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

//-------------------------------------
TEST_CASE ("fillFromCSV with startIndex copies CSV[start..] into buffer[0..]", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 32);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/BUFFER_FILLER/GOLDEN/All_Ones.csv";

    const bool ok = rd_dsp::BufferFiller::fillFromCSV (csvPath, buffer, 10);
    REQUIRE (ok);

    // CSV has 32 ones, startIndex=10 => 22 samples copied. Remaining 10
    // samples in buffer are left untouched (still 0 from construction).
    for (int i = 0; i < 22; ++i)
        REQUIRE (buffer.getSample (0, i) == 1.0f);
    for (int i = 22; i < 32; ++i)
        REQUIRE (buffer.getSample (0, i) == 0.0f);
}

//-------------------------------------
TEST_CASE ("fillFromCSV with startIndex past CSV end leaves buffer untouched", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 32);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/BUFFER_FILLER/GOLDEN/All_Ones.csv";

    const bool ok = rd_dsp::BufferFiller::fillFromCSV (csvPath, buffer, 100);
    REQUIRE (ok);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == 0.0f);
}

//-------------------------------------
TEST_CASE ("fillFromCSV with startIndex on Incremental.csv copies offset values", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 32);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/BUFFER_FILLER/GOLDEN/Incremental.csv";

    constexpr int startIndex = 10;
    const bool ok = rd_dsp::BufferFiller::fillFromCSV (csvPath, buffer, startIndex);
    REQUIRE (ok);

    // CSV holds values [0..31]. startIndex=10 => buffer[0..21] = [10..31].
    // Remaining buffer[22..31] left untouched (0 from construction).
    const int csvSamples = 32;
    const int copyCount  = csvSamples - startIndex; // 22

    for (int i = 0; i < copyCount; ++i)
        REQUIRE (buffer.getSample (0, i) == static_cast<float> (startIndex + i));

    for (int i = copyCount; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == 0.0f);
}

//-------------------------------------
TEST_CASE ("fillFromCSV with startIndex=0 on Incremental.csv copies full sequence", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 32);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/BUFFER_FILLER/GOLDEN/Incremental.csv";

    const bool ok = rd_dsp::BufferFiller::fillFromCSV (csvPath, buffer, 0);
    REQUIRE (ok);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
        REQUIRE (buffer.getSample (0, i) == static_cast<float> (i));
}

//-------------------------------------
TEST_CASE ("fillFromCSV with negative startIndex returns false", "[BufferFiller]")
{
    rd_dsp::RD_Buffer buffer (1, 32);

    const std::string csvPath =
        std::string (RD_DSP_TESTS_DIR) + "/BUFFER_FILLER/GOLDEN/All_Ones.csv";

    const bool ok = rd_dsp::BufferFiller::fillFromCSV (csvPath, buffer, -1);
    REQUIRE_FALSE (ok);
}
