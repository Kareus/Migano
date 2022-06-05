#pragma once

#include "config.h"

#if _REAL_TIME_ || _PREPROCESS_ALL_

#include <JuceHeader.h>
#include <SoundTouch.h>

class PitchShifter
{
public:
    PitchShifter();

    ~PitchShifter();

    void init(int numChannels, unsigned int sampleRate);

    void put(const float* sample, unsigned int numSamples);

    void put(const juce::AudioSampleBuffer& buffer);

    void clear();

    void flush();

    void setPitch(double pitch);

    unsigned int receive(float* dest, unsigned int numSamples);

    void receive(juce::AudioSampleBuffer& buffer);

private:
    soundtouch::SoundTouch* shifter;
};

#endif