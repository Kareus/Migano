#pragma once

#include <JuceHeader.h>

namespace SampleUtils
{
	inline int SecondsToBufferIndex(double second, int sampleRate)
	{
		return (int)(second * sampleRate);
	}

	inline int midiNoteFromFrequency(float freq)
	{
		return std::round(std::log(freq / 440.0) / log(2) * 12 + 57);
	}

	template <typename T>
	inline float getFrequencyFromNote(T note)
	{
		return 440.0 * std::pow(2.0, (note - 57) / 12.0);
	}

	int keyNameToNumber(const juce::String& keyName, const int octaveForMiddleC = 5);

	juce::String numberToKeyName(int key);

	juce::AudioSampleBuffer resample(juce::AudioSampleBuffer& buffer, int rate_from, int rate_to);
}