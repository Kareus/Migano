/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MiganoAudioProcessorEditor::MiganoAudioProcessorEditor (MiganoAudioProcessor& p, juce::AudioProcessorValueTreeState& valueTree)
    : AudioProcessorEditor(p), audioProcessor(p), valueTreeState(valueTree),
      synth(audioProcessor.getSynth()), knob(valueTree, (juce::Slider::Listener*)this), keyboard(audioProcessor.getSynth(), &knob, audioProcessor.getKeyboardState())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    bool _STANDALONE = juce::JUCEApplicationBase::isStandaloneApp();

    if (_STANDALONE)
    {
        auto window = juce::ResizableWindow::getTopLevelWindow(0);

        if (window)
        {
            auto look = dynamic_cast<juce::LookAndFeel_V4*>(&window->getLookAndFeel());

            auto scheme = look->getCurrentColourScheme();
            scheme.setUIColour(juce::LookAndFeel_V4::ColourScheme::widgetBackground, juce::Colours::black);
            look->setColourScheme(scheme);
        }

        addAndMakeVisible(keyboard);
    }

    auto& look = getLookAndFeel();
    look.setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(17, 17, 17));
    look.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colour(47, 47, 47));
    look.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::white);
    look.setColour(juce::Slider::thumbColourId, juce::Colours::white);

    setWantsKeyboardFocus(false);
    setMouseClickGrabsKeyboardFocus(false);

    addAndMakeVisible(knob);

    addAndMakeVisible(presets);
    presets.setTextWhenNoChoicesAvailable("No Preset Found.");
    presets.setTextWhenNothingSelected("Nothing");
    presets.onChange = [this]
    {
        int id = presets.getSelectedId() - 1;
        audioProcessor.setPresetID(id);
        knob.updateSound(synth->getCurrentAudioSound());
        saveState();
    };

    presets.addItemList(audioProcessor.getPresetList(), 1);

    addAndMakeVisible(presetLabel);
    presetLabel.setText("Preset:", juce::dontSendNotification);
    presetLabel.attachToComponent(&presets, true);

    if (presets.getNumItems() > 0)
    {
        int id = audioProcessor.getPresetID();
        presets.setSelectedId(id + 1, juce::dontSendNotification);
        knob.init(synth->getCurrentAudioSound());
        synth->updateADSR();
        saveState();
    }

    if (_STANDALONE)
        setSize(600, 375);
    else
        setSize(600, 275);

}

MiganoAudioProcessorEditor::~MiganoAudioProcessorEditor()
{
}

//==============================================================================
void MiganoAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MiganoAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    if (juce::JUCEApplicationBase::isStandaloneApp())
        keyboard.setBounds(keyboard.getBounds().translated(0, 280));

    knob.setBounds(knob.getBounds());
    presets.setBounds(getWidth() - 190, 130, 150, 20);
}

void MiganoAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    knob.checkValid(); //start-end range limit check

    saveState();
}

void MiganoAudioProcessorEditor::saveState()
{
    if (synth->isAvailable())
    {
        auto adsr = synth->getCurrentAudioSound()->getParameters();
        auto setting = synth->getCurrentAudioSound()->getSamplerSettings();
        knob.updateSampleInfoTo(setting);
        knob.updateADSRTo(synth->getADSRParameters());
        synth->updateADSR();

        valueTreeState.getParameter("attack")->setValueNotifyingHost(adsr->attack);
        valueTreeState.getParameter("decay")->setValueNotifyingHost(adsr->decay);
        valueTreeState.getParameter("sustain")->setValueNotifyingHost(adsr->sustain);
        valueTreeState.getParameter("release")->setValueNotifyingHost(adsr->release);

        valueTreeState.getParameter("useLoop")->setValueNotifyingHost(setting->useLoop);
        valueTreeState.getParameter("loop_resampleMode")->setValueNotifyingHost(setting->loop_resampleMode);
    }
}