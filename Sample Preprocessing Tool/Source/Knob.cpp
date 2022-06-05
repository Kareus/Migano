#include "Knob.h"
#include "config.h"

Knob::Knob(juce::Slider::Listener* listener) :
	attack(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), decay(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	sustain(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), release(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	start(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), end(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	loopStart(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow), loopEnd(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
	bender(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow)
{
	setSize(600, 280);

	setWantsKeyboardFocus(false);
	setMouseClickGrabsKeyboardFocus(false);

	setOpaque(false);

	addAndMakeVisible(attack);
	attack.setRange(0, MAX_DURATION, 0.001);
	attack.setTextValueSuffix(" s");
	attack.setMouseClickGrabsKeyboardFocus(false);
	attack.addListener(listener);

	addAndMakeVisible(attackLabel);
	attackLabel.setText("Attack", juce::dontSendNotification);
	attackLabel.setJustificationType(juce::Justification::centred);
	attackLabel.attachToComponent(&attack, false);

	addAndMakeVisible(decay);
	decay.setRange(0, MAX_DURATION, 0.001);
	decay.setTextValueSuffix(" s");
	decay.setMouseClickGrabsKeyboardFocus(false);
	decay.addListener(listener);

	addAndMakeVisible(decayLabel);
	decayLabel.setText("Decay", juce::dontSendNotification);
	decayLabel.setJustificationType(juce::Justification::centred);
	decayLabel.attachToComponent(&decay, false);

	addAndMakeVisible(sustain);
	sustain.setRange(0, 1, 0.001);
	sustain.setMouseClickGrabsKeyboardFocus(false);
	sustain.addListener(listener);

	addAndMakeVisible(sustainLabel);
	sustainLabel.setText("Sustain", juce::dontSendNotification);
	sustainLabel.setJustificationType(juce::Justification::centred);
	sustainLabel.attachToComponent(&sustain, false);

	addAndMakeVisible(release);
	release.setRange(0, MAX_DURATION, 0.001);
	release.setTextValueSuffix(" s");
	release.setMouseClickGrabsKeyboardFocus(false);
	release.addListener(listener);

	addAndMakeVisible(releaseLabel);
	releaseLabel.setText("Release", juce::dontSendNotification);
	releaseLabel.setJustificationType(juce::Justification::centred);
	releaseLabel.attachToComponent(&release, false);

	addAndMakeVisible(start);
	start.setRange(0, MAX_DURATION, 0.001);
	start.setTextValueSuffix(" s");
	start.setMouseClickGrabsKeyboardFocus(false);
	start.addListener(listener);

	addAndMakeVisible(startLabel);
	startLabel.setText("Start", juce::dontSendNotification);
	startLabel.setJustificationType(juce::Justification::centred);
	startLabel.attachToComponent(&start, false);

	addAndMakeVisible(end);
	end.setRange(0, MAX_DURATION, 0.001);
	end.setTextValueSuffix(" s");
	end.setMouseClickGrabsKeyboardFocus(false);
	end.addListener(listener);

	addAndMakeVisible(endLabel);
	endLabel.setText("End", juce::dontSendNotification);
	endLabel.setJustificationType(juce::Justification::centred);
	endLabel.attachToComponent(&end, false);

	addAndMakeVisible(useLoop);
	useLoop.setToggleState(false, juce::dontSendNotification);
	useLoop.setMouseClickGrabsKeyboardFocus(false);
	useLoop.onClick = [this, listener] { toggleLoopGUI(); listener->sliderValueChanged(nullptr); };

	addAndMakeVisible(loopStart);
	loopStart.setRange(0, MAX_DURATION, 0.001);
	loopStart.setTextValueSuffix(" s");
	loopStart.setMouseClickGrabsKeyboardFocus(false);
	loopStart.addListener(listener);

	loopStartLabel.setText("Loop Start", juce::dontSendNotification);
	loopStartLabel.setJustificationType(juce::Justification::centred);
	loopStartLabel.attachToComponent(&loopStart, false);

	addAndMakeVisible(loopEnd);
	loopEnd.setRange(0, MAX_DURATION, 0.001);
	loopEnd.setTextValueSuffix(" s");
	loopEnd.setMouseClickGrabsKeyboardFocus(false);
	loopEnd.addListener(listener);

	loopEndLabel.setText("Loop End", juce::dontSendNotification);
	loopEndLabel.setJustificationType(juce::Justification::centred);
	loopEndLabel.attachToComponent(&loopEnd, false);

	addAndMakeVisible(loop_resampler);
	loop_resampler.setToggleState(false, juce::dontSendNotification);
	loop_resampler.setMouseClickGrabsKeyboardFocus(false);
	loop_resampler.onClick = [this, listener] { listener->sliderValueChanged(nullptr); };

	addAndMakeVisible(bender);
	bender.setRange(-12, 12, 0.01);
	bender.setMouseClickGrabsKeyboardFocus(false);
	bender.addListener(listener);

	addAndMakeVisible(benderLabel);
	benderLabel.setText("Semitone", juce::dontSendNotification);
	benderLabel.setJustificationType(juce::Justification::centred);
	benderLabel.attachToComponent(&bender, false);

	toggleLoopGUI();
}

Knob::~Knob()
{

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

	bender.setBounds(left + (width + gap) * 2, top + height + 60, width, height);
}

void Knob::updateSound(AudioSound* sound)
{
	float duration = sound->getDuration();

	start.setRange(0, duration, 0.0001);
	end.setRange(0, duration, 0.0001);
	loopStart.setRange(0, duration, 0.0001);
	loopEnd.setRange(0, duration, 0.0001);

	updateSampleInfoFrom(*sound->getSamplerSettings());

	bender.setValue(sound->getSemitone());
}

void Knob::paint(juce::Graphics& g)
{
}

void Knob::updateADSRFrom(juce::ADSR::Parameters& parameters)
{
	attack.setValue(parameters.attack);
	decay.setValue(parameters.decay);
	sustain.setValue(parameters.sustain);
	release.setValue(parameters.release);
}

void Knob::updateADSRTo(juce::ADSR::Parameters* parameters)
{
	parameters->attack = attack.getValue();
	parameters->decay = decay.getValue();
	parameters->sustain = sustain.getValue();
	parameters->release = release.getValue();
}

void Knob::updateSampleInfoFrom(SamplerInfo& info)
{
	start.setValue(info.start, juce::dontSendNotification);
	end.setValue(info.end, juce::dontSendNotification);
	useLoop.setToggleState(info.useLoop, juce::dontSendNotification);
	loopStart.setValue(info.loopStart, juce::dontSendNotification);
	loopEnd.setValue(info.loopEnd, juce::dontSendNotification);
	loop_resampler.setToggleState(info.loop_resampleMode, juce::dontSendNotification);

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

void Knob::updateSemitone(AudioSound* sound)
{
	sound->setSemitone(bender.getValue());
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

void Knob::checkValid(juce::Slider* slider)
{
	if (start.getValue() >= end.getValue()) end.setValue(start.getValue(), juce::dontSendNotification);

	if (loopStart.getValue() <= start.getValue()) loopStart.setValue(start.getValue(), juce::dontSendNotification);
	if (loopStart.getValue() >= end.getValue()) loopStart.setValue(end.getValue(), juce::dontSendNotification);

	if (loopEnd.getValue() <= loopStart.getValue()) loopEnd.setValue(loopStart.getValue(), juce::dontSendNotification);
	if (loopEnd.getValue() >= end.getValue()) loopEnd.setValue(end.getValue(), juce::dontSendNotification);
}

bool Knob::isSemitoneBender(juce::Slider* slider) const
{
	return slider == &bender;
}