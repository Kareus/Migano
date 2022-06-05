#pragma once

#include <JuceHeader.h>

class WavetableCreator
{
public:
	WavetableCreator();

	juce::AudioSampleBuffer createWavetableFromAudio(juce::File file, int* sampleRate = nullptr);

private:
	juce::AudioFormatManager formatManager;
};