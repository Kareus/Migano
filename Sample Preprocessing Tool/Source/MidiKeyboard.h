#pragma once

#include <JuceHeader.h>
#include "SynthAudioSource.h"
#include "Knob.h"

class MidiKeyboard : public juce::AudioAppComponent, private juce::Timer, private juce::Slider::Listener, private juce::MouseListener
{
public:
	MidiKeyboard(juce::MidiKeyboardComponent::Orientation orientation = juce::MidiKeyboardComponent::horizontalKeyboard);

	void init(Knob* knob);

	void initRootNote(juce::Label* label);

	void initProgress(juce::Label* label);

	~MidiKeyboard() override;

	void paint(juce::Graphics& g) override;

	void resized() override;

	void updateKeyMappings();

	int getOctave() const;

	SynthAudioSource* getSynth();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

	void releaseResources() override;

	int getRootKey() const;

private:
	void update();

	void mouseDown(const juce::MouseEvent& e) override;

	void sliderValueChanged(juce::Slider* slider) override;

	void sliderDragEnded(juce::Slider* slider) override;

	//void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

	void timerCallback() override;

	void setMidiInput(int index);

	//==============================================================================
	juce::AudioDeviceManager deviceManager;
	int lastInputIndex = 0;
	bool isAddingFromMidiInput = false;

	juce::ComboBox midiInputList;
	juce::Label midiInputListLabel;

	juce::MidiKeyboardState keyboardState;
	juce::MidiKeyboardComponent keyboardComponent;

	SynthAudioSource synth;

	Knob* knob;
	juce::Label* rootNote;
	juce::Label* progress;

	double startTime;
	int octave = 5;
	int rootKey = 60;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiKeyboard)
};