/**
 * Created by Ryan Devens 2026-05-24
 */

#include <catch2/catch_test_macros.hpp>

#include "PULSAR/PulsarData.h"

using rd_dsp::PulsarData;
using rd_dsp::PulsarParamValues;

TEST_CASE ("PulsarData holds four randomized params", "[PulsarData]")
{
    PulsarData data;

    CHECK (data.formantFreq.getCentre() == 0.0f);
    CHECK (data.wavePosition.getCentre() == 0.0f);
    CHECK (data.amp.getCentre() == 0.0f);
    CHECK (data.pan.getCentre() == 0.0f);
}

TEST_CASE ("PulsarData sets default ranges and density 0 per param", "[PulsarData]")
{
    PulsarData data;

    CHECK (data.formantFreq.getStart() == 150.0f);
    CHECK (data.formantFreq.getEnd() == 2000.0f);

    CHECK (data.wavePosition.getStart() == 0.0f);
    CHECK (data.wavePosition.getEnd() == 1.0f);

    CHECK (data.amp.getStart() == 0.0f);
    CHECK (data.amp.getEnd() == 1.0f);

    CHECK (data.pan.getStart() == -1.0f);
    CHECK (data.pan.getEnd() == 1.0f);

    CHECK (data.formantFreq.getDensity() == 0.0f);
    CHECK (data.wavePosition.getDensity() == 0.0f);
    CHECK (data.amp.getDensity() == 0.0f);
    CHECK (data.pan.getDensity() == 0.0f);
}

TEST_CASE ("PulsarData resolve returns centres when density 0", "[PulsarData]")
{
    PulsarData data; // density 0 per param by default
    data.formantFreq.setCentre (440.0f);
    data.wavePosition.setCentre (0.3f);
    data.amp.setCentre (0.8f);
    data.pan.setCentre (0.0f);

    const PulsarParamValues values = data.resolve();
    CHECK (values.formantFreq == 440.0f);
    CHECK (values.wavePosition == 0.3f);
    CHECK (values.amp == 0.8f);
    CHECK (values.pan == 0.0f);
}

TEST_CASE ("PulsarData resolve stays within ranges when density 1", "[PulsarData]")
{
    PulsarData data;
    data.formantFreq.setDensity (1.0f);
    data.amp.setDensity (1.0f);

    for (int index = 0; index < 100; ++index)
    {
        const PulsarParamValues values = data.resolve();
        CHECK (values.formantFreq >= 150.0f);
        CHECK (values.formantFreq <= 2000.0f);
        CHECK (values.amp >= 0.0f);
        CHECK (values.amp <= 1.0f);
    }
}
