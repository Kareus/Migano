#include "SampleUitls.h"

namespace SampleUtils
{
	int keyNameToNumber(const juce::String& keyName, const int octaveForMiddleC)
	{
		//source: https://forum.juce.com/t/midimessage-keynametonumber/9904
		static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#" , "B" , "", "Db", "", "Eb", "", "", "Gb", "", "Ab", "", "Bb" };

		int keyNumber, octave = 0, numPos = keyName.indexOfAnyOf("01234567890-");

		if (numPos == 0)
			keyNumber = keyName.getIntValue(); //apparently already a number!

		else
		{
			if (numPos > 0)
			{
				octave = keyName.substring(numPos).getIntValue() - octaveForMiddleC + 5;
			}
			else
			{
				octave = octaveForMiddleC; //default to octave of middle C if none found
				numPos = keyName.length();
			}

			juce::String name(keyName.substring(0, numPos).trim().toUpperCase());

			keyNumber = juce::StringArray(noteNames, 12).indexOf(name) % 12;
		}

		return keyNumber + octave * 12;
	}

	juce::String numberToKeyName(int key)
	{
		static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#" , "B" };

		juce::String ret = noteNames[key % 12];
		ret += juce::String(key / 12);
		return ret;
	}

	juce::AudioSampleBuffer resample(juce::AudioSampleBuffer& buffer, int rate_from, int rate_to)
	{
		if (rate_from == rate_to) return buffer;
		juce::AudioSampleBuffer result(buffer.getNumChannels(), std::ceil((double)buffer.getNumSamples() * rate_to / rate_from));
		juce::LagrangeInterpolator interpolator;

		double ratio = (double)rate_from / rate_to;
		for (int i = 0; i < buffer.getNumChannels(); i++)
			interpolator.process(ratio, buffer.getReadPointer(i), result.getWritePointer(i), result.getNumSamples());

		return result;
	}
}