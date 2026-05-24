#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "../SOURCE/RANDOMIZER/Randomizer.h"

using rd_dsp::Randomizer;

TEST_CASE ("Randomizer setRange stores world bounds and keeps skew", "[Randomizer]")
{
    Randomizer randomizer;
    randomizer.setSkew (0.5f);
    randomizer.setRange (20.0f, 2000.0f);

    CHECK (randomizer.getStart() == 20.0f);
    CHECK (randomizer.getEnd() == 2000.0f);
    CHECK (randomizer.getSkew() == 0.5f); // setRange preserves skew
}

TEST_CASE ("Randomizer setDensity stores clamped [0,1] value", "[Randomizer]")
{
    Randomizer randomizer;

    randomizer.setDensity (0.5f);
    CHECK (randomizer.getDensity() == 0.5f);

    randomizer.setDensity (2.0f);
    CHECK (randomizer.getDensity() == 1.0f);

    randomizer.setDensity (-1.0f);
    CHECK (randomizer.getDensity() == 0.0f);
}

TEST_CASE ("Randomizer getNextRandom returns value within [start,end]", "[Randomizer]")
{
    Randomizer randomizer;

    for (int callIndex = 0; callIndex < 100; ++callIndex)
    {
        const float value = randomizer.getNextRandom (20.0f, 500.0f, 2000.0f);
        CHECK (value >= randomizer.getStart());
        CHECK (value <= randomizer.getEnd());
    }
}

TEST_CASE("Randomizer getNextRandom returns center when density is 0", "[Randomizer][Density]")
{
    Randomizer randomizer;
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

TEST_CASE ("Randomizer getNextNormalizedRandom stays in [0,1] and honors density", "[Randomizer]")
{
    Randomizer randomizer;

    // density 1: every draw is a fresh uniform [0,1]
    for (int index = 0; index < 100; ++index)
    {
        const float value = randomizer.getNextNormalizedRandom (0.5f);
        CHECK (value >= 0.0f);
        CHECK (value <= 1.0f);
    }

    // density 0: always the supplied normalized centre
    randomizer.setDensity (0.0f);
    for (int index = 0; index < 100; ++index)
        CHECK (randomizer.getNextNormalizedRandom (0.4f) == 0.4f);
}

TEST_CASE ("Randomizer converters delegate to the configured range", "[Randomizer]")
{
    Randomizer randomizer;
    randomizer.setRange (1.0f, 1500.0f); // linear

    CHECK (randomizer.convertTo0to1 (1.0f)    == Catch::Approx (0.0f).margin (1e-6));
    CHECK (randomizer.convertTo0to1 (1500.0f) == Catch::Approx (1.0f).margin (1e-6));

    CHECK (randomizer.convertFrom0to1 (0.0f) == Catch::Approx (1.0f).margin (1e-6));
    CHECK (randomizer.convertFrom0to1 (1.0f) == Catch::Approx (1500.0f).margin (1e-6));
}