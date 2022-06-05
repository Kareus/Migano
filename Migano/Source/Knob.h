#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SynthAudioSource.h"

class Knob : public juce::Component
{
public:
	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

	Knob(juce::AudioProcessorValueTreeState& valueTree, juce::Slider::Listener* listener);
	
	~Knob();

	void paint(juce::Graphics&) override;

	void resized() override;

	void updateADSRFrom(juce::ADSR::Parameters& parameters, bool notify = true);

	void updateADSRTo(juce::ADSR::Parameters* parameters);

	void updateSampleInfoFrom(SamplerInfo& info, bool notify = true);

	void updateSampleInfoTo(SamplerInfo* info);
	
	void init(AudioSound* sound);

	void updateSound(AudioSound* sound);

	void toggleLoopGUI();

	void checkValid();

private:
	juce::AudioProcessorValueTreeState& state;
	juce::Slider::Listener* listener;

	juce::Slider attack;
	juce::Label attackLabel;

	juce::Slider decay;
	juce::Label decayLabel;

	juce::Slider sustain;
	juce::Label sustainLabel;

	juce::Slider release;
	juce::Label releaseLabel;
	
	juce::Slider start;
	juce::Label startLabel;

	juce::Slider end;
	juce::Label endLabel;

	juce::ToggleButton useLoop{ "Use Loop" };

	juce::Slider loopStart;
	juce::Label loopStartLabel;

	juce::Slider loopEnd;
	juce::Label loopEndLabel;

	juce::ToggleButton loop_resampler{ "Resample Loop" };

	std::unique_ptr<SliderAttachment> attackAttachment;
	std::unique_ptr<SliderAttachment> decayAttachment;
	std::unique_ptr<SliderAttachment> sustainAttachment;
	std::unique_ptr<SliderAttachment> releaseAttachment;

	std::unique_ptr<ButtonAttachment> useLoopAttachment;
	std::unique_ptr<ButtonAttachment> loopResampleModeAttachment;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob)
};