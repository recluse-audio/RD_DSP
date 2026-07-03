/**
 * Created on 2026-05-11 by Ryan Devens w/ peace and love
 */

#pragma once
#include "Waveform.h"

#include <atomic>
#include <memory>
#include <vector>

namespace rd_dsp
{

class Wavetable
{
public:
    Wavetable();
    ~Wavetable();

    // given a 0-1 value, this maps between the two currently active waveforms
    void setNormalizedWavePosition(float normalizedWavePos);

    float getSampleAtIndex (float index) const noexcept;

    // Fills out[0..outSize) with the currently active interpolated waveform,
    // stretched/decimated to outSize. Allocation-free, no exceptions.
    // Caller owns the buffer. Writes nothing if empty or outSize <= 0.
    void fillDisplayBuffer (float* out, int outSize) const noexcept;

    // Same as fillDisplayBuffer, but averages source samples within each
    // output bin (box filter). Useful when outSize << waveformSize and the
    // point-sampled version aliases visibly. Allocation-free, no exceptions.
    void fillDisplayBufferAveraged (float* out, int outSize) const noexcept;

    void addWaveform(std::unique_ptr<Waveform>);
    void clear() noexcept;

    int getNumWaveforms() const noexcept;
    int getWaveformSize() const noexcept;

    // Resize all waveforms and (re)fill with the canonical 4 basic shapes
    // in order: sine, triangle, square, saw. Existing contents discarded.
    void fillWithBasicShapes (int numSamples);

    // Returns a pointer to the waveform at index, or nullptr if index is
    // out of range (including the empty-wavetable case).
    const Waveform* getWaveformAtIndex (int index) const noexcept;

private:
    std::vector<std::unique_ptr<Waveform>> mWaveforms;
    // Written from the control/audio thread, read on the GUI thread (fillDisplayBuffer)
    // and audio thread (getSampleAtIndex). Atomic to make those accesses race-free.
    std::atomic<float> mNormalizedWavePos { 0.f };

    // given the mNormalizedWavePos above, what would that be when scaled to mWaveforms.size()
    float _getPositionForWavetableSize() const noexcept;
};

} // namespace rd_dsp
