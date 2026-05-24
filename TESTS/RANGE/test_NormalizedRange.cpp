#include <catch2/catch_test_macros.hpp>

#include "../SOURCE/RANGE/NormalizedRange.h"

using rd_dsp::NormalizedRange;

TEST_CASE ("NormalizedRange clamps members to [0,1] and keeps min<=center<=max", "[NormalizedRange]")
{
    NormalizedRange range (-0.5f, 0.5f, 2.0f);

    CHECK (range.getMin() == 0.0f);
    CHECK (range.getCenter() == 0.5f);
    CHECK (range.getMax() == 1.0f);

    CHECK (range.getMin() <= range.getCenter());
    CHECK (range.getCenter() <= range.getMax());
}

TEST_CASE ("NormalizedRange keeps logical range when min exceeds center and max", "[NormalizedRange]")
{
    NormalizedRange range (0.9f, 0.5f, 0.6f);

    CHECK (range.getMin() <= range.getCenter());
    CHECK (range.getCenter() <= range.getMax());

    // min/max swapped so max stays highest; center pulled into range.
    CHECK (range.getMin() == 0.6f);
    CHECK (range.getMax() == 0.9f);
    CHECK (range.getCenter() == 0.6f);
}

TEST_CASE ("NormalizedRange keeps logical range when max is less than center", "[NormalizedRange]")
{
    NormalizedRange range (0.2f, 0.8f, 0.5f);

    CHECK (range.getMin() <= range.getCenter());
    CHECK (range.getCenter() <= range.getMax());

    // center clamped down to max.
    CHECK (range.getMin() == 0.2f);
    CHECK (range.getMax() == 0.5f);
    CHECK (range.getCenter() == 0.5f);
}
