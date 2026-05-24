#include <catch2/catch_test_macros.hpp>

#include "../SOURCE/RANDOMIZER/NormalizedRandomizer.h"

using rd_dsp::NormalizedRandomizer;

TEST_CASE ("NormalizedRandomizer setNormalizedRange stores clamped, ordered range", "[NormalizedRandomizer]")
{
    NormalizedRandomizer randomizer;
    randomizer.setNormalizedRange (0.2f, 0.5f, 0.8f);

    CHECK (randomizer.getMin() == 0.2f);
    CHECK (randomizer.getCenter() == 0.5f);
    CHECK (randomizer.getMax() == 0.8f);

    CHECK (randomizer.getMin() <= randomizer.getCenter());
    CHECK (randomizer.getCenter() <= randomizer.getMax());
}

TEST_CASE ("NormalizedRandomizer setDensity stores clamped [0,1] value", "[NormalizedRandomizer]")
{
    NormalizedRandomizer randomizer;

    randomizer.setDensity (0.5f);
    CHECK (randomizer.getDensity() == 0.5f);

    randomizer.setDensity (2.0f);
    CHECK (randomizer.getDensity() == 1.0f);

    randomizer.setDensity (-1.0f);
    CHECK (randomizer.getDensity() == 0.0f);
}

TEST_CASE ("NormalizedRandomizer getNextRandom returns value within [min,max]", "[NormalizedRandomizer]")
{
    NormalizedRandomizer randomizer;

    for (int callIndex = 0; callIndex < 100; ++callIndex)
    {
        const float value = randomizer.getNextRandom (0.2f, 0.5f, 0.8f);
        CHECK (value >= randomizer.getMin());
        CHECK (value <= randomizer.getMax());
    }
}

TEST_CASE("NormalizedRandomizer getNextRandom returns center when density is 0", "[NormalizedRandomizer][Density]")
{
    NormalizedRandomizer randomizer;
    randomizer.setDensity (0.f);

    for (int index = 0; index < 100; index++)
    {
        const float value = randomizer.getNextRandom (0.3f, 0.5f, 0.9f);
        CHECK (value == 0.5f);
    }

    // Density 0.5: the gate should fire (randomize off center) about half the time.
    // Count draws that differ from center -- those are the ones the gate let through.
    constexpr float center = 0.5f;
    int randomizedCount = 0;
    randomizer.setDensity (0.5f);
    for (int index = 0; index < 100; index++)
    {
        const float value = randomizer.getNextRandom (0.0f, center, 1.f);
        if (value != center)
            randomizedCount++;
    }

    INFO (randomizedCount);
    // Statistical, not exact: with a fixed seed this is stable, ~50 expected.
    CHECK (randomizedCount > 30);
    CHECK (randomizedCount < 70);

}