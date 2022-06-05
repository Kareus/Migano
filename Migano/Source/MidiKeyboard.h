#pragma once

#include <JuceHeader.h>
#include "SynthAudioSource.h"
#include "Knob.h"

class MidiKeyboard : public juce::Component, private juce::Timer, private juce::KeyListener
{
public:
	MidiKeyboard(SynthAudioSource* synth, Knob* knob, juce::MidiKeyboardState& state, juce::MidiKeyboardComponent::Orientation orientation = juce::MidiKeyboardComponent::horizontalKeyboard);

	~MidiKeyboard() override;

	void paint(juce::Graphics& g) override;

	void resized() override;

	void updateKeyMappings();

	int getOctave() const;


	void grab();

private:
	bool keyPressed(const juce::KeyPress& key, juce::Component*) override;

	void timerCallback() override;

	void setMidiInput(int index);

	//==============================================================================
	juce::AudioDeviceManager deviceManager;
	int lastInputIndex = 0;
	bool isAddingFromMidiInput = false;

	juce::ComboBox midiInputList;
	juce::Label midiInputListLabel;

	juce::MidiKeyboardState& keyboardState;
	juce::MidiKeyboardComponent keyboardComponent;

	SynthAudioSource* synth;

	Knob* knob;
	double startTime;
	int octave = 5;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiKeyboard)
};