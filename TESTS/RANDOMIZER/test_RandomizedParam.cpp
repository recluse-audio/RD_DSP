/**
 * Created by Ryan Devens 2026-05-24
 */

#include <catch2/catch_test_macros.hpp>

#include "RANDOMIZER/RandomizedParam.h"

using rd_dsp::RandomizedParam;

TEST_CASE ("RandomizedParam delegates range/skew/density to its randomizer", "[RandomizedParam]")
{
    RandomizedParam param;
    param.setRange (150.0f, 2000.0f);
    param.setSkew (0.5f);
    param.setDensity (0.25f);

    CHECK (param.getStart() == 150.0f);
    CHECK (param.getEnd() == 2000.0f);
    CHECK (param.getSkew() == 0.5f);
    CHECK (param.getDensity() == 0.25f);
}

TEST_CASE ("RandomizedParam stores the centre as-is (no clamp)", "[RandomizedParam]")
{
    RandomizedParam param;
    param.setRange (150.0f, 2000.0f);

    param.setCentre (500.0f);
    CHECK (param.getCentre() == 500.0f);

    // trust caller: a centre outside the range is kept verbatim
    param.setCentre (5000.0f);
    CHECK (param.getCentre() == 5000.0f);
}

TEST_CASE ("RandomizedParam draw returns centre when density is 0", "[RandomizedParam]")
{
    RandomizedParam param;
    param.setRange (150.0f, 2000.0f);
    param.setCentre (500.0f);
    param.setDensity (0.0f);

    for (int index = 0; index < 100; ++index)
        CHECK (param.draw() == 500.0f);
}

TEST_CASE ("RandomizedParam draw stays within range when density is 1", "[RandomizedParam]")
{
    RandomizedParam param;
    param.setRange (150.0f, 2000.0f);
    param.setCentre (500.0f);
    param.setDensity (1.0f);

    for (int index = 0; index < 100; ++index)
    {
        const float value = param.draw();
        CHECK (value >= 150.0f);
        CHECK (value <= 2000.0f);
    }
}
