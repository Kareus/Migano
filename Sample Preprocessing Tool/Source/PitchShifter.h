/*
  ==============================================================================

    PitchShifter.h
    Created: 16 May 2022 5:48:10pm
    Author:  Kareus

  ==============================================================================
*/

#pragma once
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

    void setSemitone(double semitone);

    unsigned int receive(float* dest, unsigned int numSamples);

    void receive(juce::AudioSampleBuffer& buffer);

private:
    soundtouch::SoundTouch* shifter;
};