#include "WavetableCreator.h"

WavetableCreator::WavetableCreator()
{
	formatManager.registerBasicFormats();
}

juce::AudioSampleBuffer WavetableCreator::createWavetableFromAudio(juce::File file, int* sampleRate)
{
	juce::AudioSampleBuffer waveTable;
	if (file == juce::File{})
		return waveTable;

	std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

	if (reader.get())
	{
		auto duration = (float)reader->lengthInSamples / reader->sampleRate;

		if (duration >= 5)
			throw std::exception("Audio should be less than 5 seconds.");

		waveTable.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
		waveTable.clear();

		reader->read(&waveTable, 0, (int)reader->lengthInSamples, 0, true, true);
		if (sampleRate) *sampleRate = reader->sampleRate;
	}

	return waveTable;
}