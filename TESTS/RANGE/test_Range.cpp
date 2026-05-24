#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../SOURCE/RANGE/Range.h"

using rd_dsp::Range;

TEST_CASE ("Range linear maps endpoints to 0 and 1", "[Range]")
{
    Range range (1.0f, 1500.0f); // skew defaults to 1 (linear)

    CHECK (range.convertTo0to1 (1.0f)    == Catch::Approx (0.0f).margin (1e-6));
    CHECK (range.convertTo0to1 (1500.0f) == Catch::Approx (1.0f).margin (1e-6));

    CHECK (range.convertFrom0to1 (0.0f) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (range.convertFrom0to1 (1.0f) == Catch::Approx (1500.0f).margin (1e-6));
}

TEST_CASE ("Range linear round-trips a midpoint", "[Range]")
{
    Range range (1.0f, 1500.0f);

    float mid = range.convertFrom0to1 (0.5f);
    CHECK (mid == Catch::Approx (750.5f).margin (1e-4));
    CHECK (range.convertTo0to1 (mid) == Catch::Approx (0.5f).margin (1e-6));
}

TEST_CASE ("Range skew bends the curve and still round-trips", "[Range]")
{
    Range range (1.0f, 1500.0f, 0.5f); // skew < 1: low end stretched

    // endpoints unchanged by skew
    CHECK (range.convertFrom0to1 (0.0f) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (range.convertFrom0to1 (1.0f) == Catch::Approx (1500.0f).margin (1e-6));

    // 0.5 -> 0.5^0.5 = 0.70710678 -> 1 + 1499*0.70710678
    float mid = range.convertFrom0to1 (0.5f);
    CHECK (mid == Catch::Approx (1060.953f).margin (1e-2));

    // inverse undoes the skew
    CHECK (range.convertTo0to1 (mid) == Catch::Approx (0.5f).margin (1e-6));
}

TEST_CASE ("Range setSkewForCentre puts the world centre at 0.5", "[Range]")
{
    Range range (1.0f, 1500.0f);
    range.setSkewForCentre (100.0f);

    CHECK (range.convertFrom0to1 (0.5f) == Catch::Approx (100.0f).margin (1e-2));

    // endpoints stay anchored
    CHECK (range.convertFrom0to1 (0.0f) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (range.convertFrom0to1 (1.0f) == Catch::Approx (1500.0f).margin (1e-6));
}

TEST_CASE ("Range clamps out-of-range values", "[Range]")
{
    Range range (1.0f, 1500.0f); // linear

    CHECK (range.convertTo0to1 (-50.0f)  == Catch::Approx (0.0f).margin (1e-6));
    CHECK (range.convertTo0to1 (5000.0f) == Catch::Approx (1.0f).margin (1e-6));

    CHECK (range.convertFrom0to1 (-0.5f) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (range.convertFrom0to1 (1.5f)  == Catch::Approx (1500.0f).margin (1e-6));
}
