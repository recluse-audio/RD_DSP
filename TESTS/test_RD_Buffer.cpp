/**
 * test_RD_Buffer.cpp
 * Tagged [RD_Buffer].
 */

#include <catch2/catch_test_macros.hpp>
#include "RD_Buffer.h"

TEST_CASE ("Default-constructed RD_Buffer reports zero size", "[RD_Buffer]")
{
    rd::RD_Buffer buffer;

    REQUIRE (buffer.getNumChannels() == 0);
    REQUIRE (buffer.getNumSamples()  == 0);
}
