/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SynthAudioSource.h"

#include "MidiKeyboard.h"
#include "Knob.h"

//==============================================================================
/**
*/
class MiganoAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Slider::Listener
{
public:

    MiganoAudioProcessorEditor (MiganoAudioProcessor&, juce::AudioProcessorValueTreeState& valueTree);
    ~MiganoAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::string SOUNDBANK_PATH;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MiganoAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    MidiKeyboard keyboard;
    Knob knob;

    juce::ComboBox presets;
    juce::Label presetLabel;
    SynthAudioSource* synth;

    void sliderValueChanged(juce::Slider* slider) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiganoAudioProcessorEditor)
};
