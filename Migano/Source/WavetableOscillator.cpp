#include "WavetableOscillator.h"

WavetableOscillator::WavetableOscillator(const juce::AudioSampleBuffer& wavetableToUse)
    : wavetable(wavetableToUse),
    tableSize(wavetable.getNumSamples() - 1)
{
}

void WavetableOscillator::setFrequency(float frequency, float sampleRate)
{
    auto tableSizeOverSampleRate = (float)tableSize / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}

void WavetableOscillator::stop()
{
    currentIndex = 0.0f;
    tableDelta = 0.0f;
}

bool WavetableOscillator::isPlaying() const
{
    return tableDelta != 0.0f;
}