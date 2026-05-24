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

TEST_CASE ("NormalizedRandomizer getRandom returns value within [min,max]", "[NormalizedRandomizer]")
{
    NormalizedRandomizer randomizer;
    randomizer.setNormalizedRange (0.2f, 0.5f, 0.8f);

    for (int callIndex = 0; callIndex < 100; ++callIndex)
    {
        const float value = randomizer.getNextRandom();
        CHECK (value >= randomizer.getMin());
        CHECK (value <= randomizer.getMax());
    }
}
