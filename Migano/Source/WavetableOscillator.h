#pragma once

#include <JuceHeader.h>

class WavetableOscillator
{
public:
    WavetableOscillator(const juce::AudioSampleBuffer& wavetableToUse);

    void setFrequency(float frequency, float sampleRate);

    forcedinline float WavetableOscillator::getNextSample() noexcept
    {
        jassert(isPlaying());

        auto index0 = (unsigned int)currentIndex;
        auto index1 = index0 + 1;

        auto frac = currentIndex - (float)index0;

        auto* table = wavetable.getReadPointer(0);
        auto value0 = table[index0];
        auto value1 = table[index1];

        auto currentSample = value0 + frac * (value1 - value0);

        if ((currentIndex += tableDelta) > (float)tableSize)
            currentIndex -= (float)tableSize;

        return currentSample;
    }

    void stop();

    bool isPlaying() const;

private:
    juce::AudioSampleBuffer wavetable;
    const int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f;

};