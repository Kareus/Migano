/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthAudioSource.h"

//==============================================================================
/**
*/
class MiganoAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MiganoAudioProcessor();
    ~MiganoAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    SynthAudioSource* getSynth();
    
    juce::MidiKeyboardState& getKeyboardState();

    void setPresetID(int presetID);

    int getPresetID() const;

    void addSoundItem(juce::File file);

    void readAllSoundBanks(const std::string& path);

    const juce::StringArray& getPresetList() const;

private:
    SynthAudioSource synth;
    juce::MidiKeyboardState keyboardState;
    int presetID;
    std::string SOUNDBANK_PATH;
    juce::StringArray presetList;
    SamplerInfo loadedInfo;

    juce::AudioProcessorValueTreeState parameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiganoAudioProcessor)
};
