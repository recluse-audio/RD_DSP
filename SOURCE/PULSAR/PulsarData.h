/**
 * Created by Ryan Devens 2026-05-24
 */

#pragma once

#include "../RANDOMIZER/RandomizedParam.h"

namespace rd_dsp
{
/**
 * One emission's resolved (drawn) parameter values. Pan is carried but its
 * operations are not wired yet (TODO).
 */
struct PulsarParamValues
{
    float formantFreq = 0.0f;
    float wavePosition = 0.0f;
    float amp = 0.0f;
    float pan = 0.0f; // TODO(pan): carried but no panning operations yet
};

/**
 * Per-pulsar randomization data: a RandomizedParam per parameter. On emission
 * each param is potentially randomized within its range (density-gated).
 * Pan is a placeholder this task -- no operations wired yet (TODO).
 */
struct PulsarData
{
    PulsarData(); // sets default bounds per param + density 0 (never randomize yet)

    PulsarParamValues resolve(); // draw every param once for one emission

    RandomizedParam formantFreq;
    RandomizedParam wavePosition;
    RandomizedParam amp;
    RandomizedParam pan; // TODO(pan): range/draw work, but no panning ops wired yet
};

} // namespace rd_dsp
