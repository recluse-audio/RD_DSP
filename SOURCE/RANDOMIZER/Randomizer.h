/**
 * Created by Ryan Devens 2026-05-23
 */

#pragma once

#include "../RANGE/Range.h"

#include <atomic>
#include <random>

namespace rd_dsp
{
/**
 * Draws a random world value from a Range, with the twist of mDensity:
 * the probability that a fresh random value replaces the supplied centre.
 *
 * You give it a centre default within [start, end] and it *might* come back
 * different. The draw respects the range's skew.
 *
 *  mDensity is the percentage of times a random value should be generated.
 */
class Randomizer
{
public:
    void setRange (float start, float end); // world bounds, keeps current skew
    void setSkew (float skew);

    float getStart() const;
    float getEnd() const;
    float getSkew() const;

    float getNextRandom (float start, float centre, float end);

    // Normalized [0,1] base draw: density gate over a uniform draw, skew-independent.
    float getNextNormalizedRandom (float normalizedCentre);

    // Map base [0,1] <-> world through the configured range (delegates to Range).
    float convertTo0to1 (float worldValue) const;
    float convertFrom0to1 (float normalizedValue) const;

    void setDensity (float density);
    float getDensity() const;

private:
    // Config is written from the control thread, read on the audio thread, so
    // every field is atomic. The Range is rebuilt locally per draw/convert from
    // these scalars -- it carries no shared state of its own.
    std::atomic<float> mStart   { 0.0f };
    std::atomic<float> mEnd     { 1.0f };
    std::atomic<float> mSkew    { 1.0f };
    std::atomic<float> mDensity { 1.0f };
    std::minstd_rand mRandomNumberGenerator;

    Range _makeRange() const;        // build a Range from the current atomic bounds/skew
    float _getNormalizedRandom();    // raw draw mapped to [0,1]
    float _getRandomValueInRange();  // [0,1] draw mapped through the range
};

} // namespace rd_dsp
