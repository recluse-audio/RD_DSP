/**
 * Created on 2026-05-13 by Ryan Devens w/ peace and love
 */

#pragma once
#include "../RD_BUFFER/RD_Buffer.h"

namespace rd_dsp
{

/**
 * Single-channel window function look-up table.
 *
 * Owns an RD_Buffer of amplitude coefficients (0 .. 1) and fills it with a
 * chosen shape. Read via index or interpolated float index; the caller picks
 * the rate.
 *
 * Framework-free port of RD's Window class (which uses juce::AudioBuffer).
 */
class Window
{
public:
    enum class Shape
    {
        kNone    = 0,
        kHanning = 1,
        kTukey   = 2
    };

    Window();
    ~Window();

    void setSize(int numSamples);

    int   getNumSamples() const noexcept { return mBuffer.getNumSamples(); }
    float getSample(int sampleIndex) const noexcept
    {
        return mBuffer.getSample (0, sampleIndex);
    }

    void  setShape(Shape shape);
    Shape getShape() const noexcept { return mCurrentShape; }

    float getInterpolatedSampleAtIndex(float index);
private:
    friend class WindowTester;
    void _update();
    void _fillWithOnes();
    void _fillWithHanning();
    void _fillWithTukey();

    RD_Buffer mBuffer;
    Shape     mCurrentShape = Shape::kNone;
};

} // namespace rd_dsp
