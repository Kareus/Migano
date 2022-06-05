#include "Knob.h"
#include "config.h"

Knob::Knob(juce::AudioProcessorValueTreeState& valueTree, juce::Slider::Listener* listener) : state(valueTree), listener(listener),
	attack(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), decay(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	sustain(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), release(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	start(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), end(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	loopStart(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), loopEnd(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow)
{
	setSize(600, 280);

	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);

	setOpaque(false);

	//attack
	addAndMakeVisible(attack);
	attack.setTextValueSuffix(" s");
	attack.setMouseClickGrabsKeyboardFocus(false);

	addAndMakeVisible(attackLabel);
	attackLabel.setText("Attack", juce::dontSendNotification);
	attackLabel.setJustificationType(juce::Justification::centred);
	attackLabel.attachToComponent(&attack, false);

	attackAttachment.reset(new SliderAttachment(state, "attack", attack));
	attack.addListener(listener);

	//decay
	addAndMakeVisible(decay);
	decay.setTextValueSuffix(" s");
	decay.setMouseClickGrabsKeyboardFocus(false);

	addAndMakeVisible(decayLabel);
	decayLabel.setText("Decay", juce::dontSendNotification);
	decayLabel.setJustificationType(juce::Justification::centred);
	decayLabel.attachToComponent(&decay, false);

	decayAttachment.reset(new SliderAttachment(state, "decay", decay));
	decay.addListener(listener);

	//sustain
	addAndMakeVisible(sustain);
	sustain.setMouseClickGrabsKeyboardFocus(false);

	addAndMakeVisible(sustainLabel);
	sustainLabel.setText("Sustain", juce::dontSendNotification);
	sustainLabel.setJustificationType(juce::Justification::centred);
	sustainLabel.attachToComponent(&sustain, false);

	sustainAttachment.reset(new SliderAttachment(state, "sustain", sustain));
	sustain.addListener(listener);

	//release
	addAndMakeVisible(release);
	release.setTextValueSuffix(" s");
	release.setMouseClickGrabsKeyboardFocus(false);

	addAndMakeVisible(releaseLabel);
	releaseLabel.setText("Release", juce::dontSendNotification);
	releaseLabel.setJustificationType(juce::Justification::centred);
	releaseLabel.attachToComponent(&release, false);

	releaseAttachment.reset(new SliderAttachment(state, "release", release));
	release.addListener(listener);

	//below not linked with value tree state (except two buttons), so cannot link with automation, as each sample has specific range and limit (like start <= end)
	//sample start
	addAndMakeVisible(start);
	start.setRange(0, MAX_DURATION, 0.001);
	start.setTextValueSuffix(" s");
	start.setMouseClickGrabsKeyboardFocus(false);
	start.addListener(listener);

	addAndMakeVisible(startLabel);
	startLabel.setText("Start", juce::dontSendNotification);
	startLabel.setJustificationType(juce::Justification::centred);
	startLabel.attachToComponent(&start, false);

	//sample end
	addAndMakeVisible(end);
	end.setRange(0, MAX_DURATION, 0.001);
	end.setTextValueSuffix(" s");
	end.setMouseClickGrabsKeyboardFocus(false);
	end.addListener(listener);

	addAndMakeVisible(endLabel);
	endLabel.setText("End", juce::dontSendNotification);
	endLabel.setJustificationType(juce::Justification::centred);
	endLabel.attachToComponent(&end, false);

	//whether sample loops
	addAndMakeVisible(useLoop);
	useLoopAttachment.reset(new ButtonAttachment(state, "useLoop", useLoop));

	useLoop.setMouseClickGrabsKeyboardFocus(false);
	useLoop.setToggleState(false, juce::dontSendNotification);
	useLoop.onClick = [this, listener] { toggleLoopGUI(); listener->sliderValueChanged(nullptr); };

	//sample loop start
	addAndMakeVisible(loopStart);
	loopStart.setRange(0, MAX_DURATION, 0.001);
	loopStart.setTextValueSuffix(" s");
	loopStart.setMouseClickGrabsKeyboardFocus(false);
	loopStart.addListener(listener);

	loopStartLabel.setText("Loop Start", juce::dontSendNotification);
	loopStartLabel.setJustificationType(juce::Justification::centred);
	loopStartLabel.attachToComponent(&loopStart, false);

	//sample loop end
	addAndMakeVisible(loopEnd);
	loopEnd.setRange(0, MAX_DURATION, 0.001);
	loopEnd.setTextValueSuffix(" s");
	loopEnd.setMouseClickGrabsKeyboardFocus(false);
	loopEnd.addListener(listener);

	loopEndLabel.setText("Loop End", juce::dontSendNotification);
	loopEndLabel.setJustificationType(juce::Justification::centred);
	loopEndLabel.attachToComponent(&loopEnd, false);

	//sample loop mode
	addAndMakeVisible(loop_resampler);
	loopResampleModeAttachment.reset(new ButtonAttachment(state, "loop_resampleMode", loop_resampler));

	loop_resampler.setMouseClickGrabsKeyboardFocus(false);
	loop_resampler.setToggleState(false, juce::dontSendNotification);
	loop_resampler.onClick = [this, listener] { listener->sliderValueChanged(nullptr); };

	toggleLoopGUI();
}

Knob::~Knob()
{
	attack.removeListener(listener);
	decay.removeListener(listener);
	sustain.removeListener(listener);
	release.removeListener(listener);

	start.removeListener(listener);
	end.removeListener(listener);

	loopStart.removeListener(listener);
	loopEnd.removeListener(listener);

}

void Knob::resized()
{
	constexpr auto left = 40;
	constexpr auto top = 40;
	constexpr auto width = 80;
	constexpr auto height = 80;
	constexpr auto gap = 10;

	attack.setBounds(left, top, width, height);
	decay.setBounds(left + (width + gap) * 1, top, width, height);
	sustain.setBounds(left + (width + gap) * 2, top, width, height);
	release.setBounds(left + (width + gap) * 3, top, width, height);
	
	start.setBounds(left + (width + gap) * 4, top, width, height);
	end.setBounds(left + (width + gap) * 5, top, width, height);

	useLoop.setBounds(left, top + height + 10, width, 20);

	loopStart.setBounds(left, top + height + 60, width, height);
	loopEnd.setBounds(left + (width + gap) * 1, top + height + 60, width, height);

	loop_resampler.setBounds(left + (width + gap) * 1, top + height + 10, width * 2, 20);
}

//for first update, don't notify
void Knob::init(AudioSound* sound)
{
	if (sound == nullptr) return;
	float duration = sound->getDuration();

	start.setRange(0, duration, 0.0001);
	end.setRange(0, duration, 0.0001);
	loopStart.setRange(0, duration, 0.0001);
	loopEnd.setRange(0, duration, 0.0001);

	updateADSRFrom(*sound->getParameters(), false);
	updateSampleInfoFrom(*sound->getSamplerSettings(), false);
}

void Knob::updateSound(AudioSound* sound)
{
	if (sound == nullptr) return;
	float duration = sound->getDuration();

	start.setRange(0, duration, 0.0001);
	end.setRange(0, duration, 0.0001);
	loopStart.setRange(0, duration, 0.0001);
	loopEnd.setRange(0, duration, 0.0001);

	updateADSRFrom(*sound->getParameters());
	updateSampleInfoFrom(*sound->getSamplerSettings());
}

void Knob::paint(juce::Graphics& g)
{
}

void Knob::updateADSRFrom(juce::ADSR::Parameters& parameters, bool notify)
{
	auto n = notify ? juce::sendNotification : juce::dontSendNotification;

	attack.setValue(parameters.attack, n);
	decay.setValue(parameters.decay, n);
	sustain.setValue(parameters.sustain, n);
	release.setValue(parameters.release, n);
}

void Knob::updateADSRTo(juce::ADSR::Parameters* parameters)
{
	parameters->attack = attack.getValue();
	parameters->decay = decay.getValue();
	parameters->sustain = sustain.getValue();
	parameters->release = release.getValue();
}

void Knob::updateSampleInfoFrom(SamplerInfo& info, bool notify)
{
	auto n = notify ? juce::sendNotification : juce::dontSendNotification;

	start.setValue(info.start, n);
	end.setValue(info.end, n);
	loopStart.setValue(info.loopStart, n);
	loopEnd.setValue(info.loopEnd, n);
	useLoop.setToggleState(info.useLoop, n);
	loop_resampler.setToggleState(info.loop_resampleMode, n);

	toggleLoopGUI();
}

void Knob::updateSampleInfoTo(SamplerInfo* info)
{
	info->start = start.getValue();
	info->end = end.getValue();
	info->useLoop = useLoop.getToggleState();
	info->loopStart = loopStart.getValue();
	info->loopEnd = loopEnd.getValue();
	info->loop_resampleMode = loop_resampler.getToggleState();

	jassert(info->isValid());
}

void Knob::toggleLoopGUI()
{
	if (useLoop.getToggleState())
	{
		loopStart.setVisible(true);
		loopStartLabel.setVisible(true);
		loopEnd.setVisible(true);
		loopEndLabel.setVisible(true);
		loop_resampler.setVisible(true);
	}
	else
	{
		loopStart.setVisible(false);
		loopStartLabel.setVisible(false);
		loopEnd.setVisible(false);
		loopEndLabel.setVisible(false);
		loop_resampler.setVisible(false);
	}
}

void Knob::checkValid()
{
	if (start.getValue() >= end.getValue()) end.setValue(start.getValue(), juce::dontSendNotification);

	if (loopStart.getValue() <= start.getValue()) loopStart.setValue(start.getValue(), juce::dontSendNotification);
	if (loopStart.getValue() >= end.getValue()) loopStart.setValue(end.getValue(), juce::dontSendNotification);

	if (loopEnd.getValue() <= loopStart.getValue()) loopEnd.setValue(loopStart.getValue(), juce::dontSendNotification);
	if (loopEnd.getValue() >= end.getValue()) loopEnd.setValue(end.getValue(), juce::dontSendNotification);
}