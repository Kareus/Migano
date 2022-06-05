#pragma once

#include <JuceHeader.h>
#include "SynthAudioSource.h"

class Knob : public juce::Component
{
public:
	Knob(juce::Slider::Listener* listener);
	
	~Knob();

	void paint(juce::Graphics&) override;

	void resized() override;

	void updateADSRFrom(juce::ADSR::Parameters& parameters);

	void updateADSRTo(juce::ADSR::Parameters* parameters);

	void updateSampleInfoFrom(SamplerInfo& info);

	void updateSampleInfoTo(SamplerInfo* info);

	void updateSound(AudioSound* sound);

	void updateSemitone(AudioSound* sound);

	void toggleLoopGUI();

	void checkValid(juce::Slider* slider);

	bool isSemitoneBender(juce::Slider* slider) const;

private:
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

	juce::Slider bender;
	juce::Label benderLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob)
};