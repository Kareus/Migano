#include "MidiKeyboard.h"

MidiKeyboard::MidiKeyboard(juce::MidiKeyboardComponent::Orientation orientation) : keyboardComponent(keyboardState, orientation), startTime(juce::Time::getMillisecondCounterHiRes() * 0.001),
    synth(keyboardState), octave(5), rootNote(nullptr), progress(nullptr)
{
    setOpaque(true);

    addAndMakeVisible(midiInputListLabel);
    midiInputListLabel.setText("MIDI Input:", juce::dontSendNotification);
    midiInputListLabel.attachToComponent(&midiInputList, true);

    addAndMakeVisible(midiInputList);
    midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");

    auto midiInputs = juce::MidiInput::getAvailableDevices();

    juce::StringArray midiInputNames;

    for (auto input : midiInputs)
        midiInputNames.add(input.name);

    midiInputList.addItemList(midiInputNames, 1);
    midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex()); };

    // find the first enabled device and use that by default

    for (auto input : midiInputs)
    {
        if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
        {
            setMidiInput(midiInputs.indexOf(input));
            break;
        }
    }

    // if no enabled devices were found just use the first one in the list
    if (midiInputList.getSelectedId() == 0)
        setMidiInput(0);


    addAndMakeVisible(keyboardComponent);

    keyboardComponent.setOctaveForMiddleC(5);

    keyboardComponent.clearKeyMappings();
    keyboardComponent.setKeyPressForNote(juce::KeyPress('q', juce::ModifierKeys::noModifiers, 0), 0);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('2', juce::ModifierKeys::noModifiers, 0), 1);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('w', juce::ModifierKeys::noModifiers, 0), 2);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('3', juce::ModifierKeys::noModifiers, 0), 3);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('e', juce::ModifierKeys::noModifiers, 0), 4);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('r', juce::ModifierKeys::noModifiers, 0), 5);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('5', juce::ModifierKeys::noModifiers, 0), 6);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('t', juce::ModifierKeys::noModifiers, 0), 7);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('6', juce::ModifierKeys::noModifiers, 0), 8);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('y', juce::ModifierKeys::noModifiers, 0), 9);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('7', juce::ModifierKeys::noModifiers, 0), 10);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('u', juce::ModifierKeys::noModifiers, 0), 11);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('i', juce::ModifierKeys::noModifiers, 0), 12);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('9', juce::ModifierKeys::noModifiers, 0), 13);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('o', juce::ModifierKeys::noModifiers, 0), 14);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('0', juce::ModifierKeys::noModifiers, 0), 15);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('p', juce::ModifierKeys::noModifiers, 0), 16);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('[', juce::ModifierKeys::noModifiers, 0), 17);
    keyboardComponent.setKeyPressForNote(juce::KeyPress('+', juce::ModifierKeys::noModifiers, 0), 18);
    keyboardComponent.setKeyPressForNote(juce::KeyPress(']', juce::ModifierKeys::noModifiers, 0), 19);

    updateKeyMappings();

    keyboardComponent.addMouseListener(this, true);

    setAudioChannels(0, 2);
    startTimer(400);

    setSize(600, 100);
}

MidiKeyboard::~MidiKeyboard()
{
    shutdownAudio();
    deviceManager.removeMidiInputDeviceCallback(juce::MidiInput::getAvailableDevices()[midiInputList.getSelectedItemIndex()].identifier, synth.getMidiCollector());
}

void MidiKeyboard::init(Knob* knob)
{
    this->knob = knob;

    if (synth.isAvailable())
    {
        auto sound = synth.getCurrentAudioSound();
        knob->updateSound(sound);
        rootKey = SampleUtils::midiNoteFromFrequency(sound->getBaseFrequency());

        keyboardComponent.grabKeyboardFocus();
        update();
    }
}

void MidiKeyboard::initRootNote(juce::Label* label)
{
    this->rootNote = label;
    update();
}

void MidiKeyboard::initProgress(juce::Label* label)
{
    this->progress = label;
}

void MidiKeyboard::timerCallback()
{
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}

void MidiKeyboard::paint(juce::Graphics& g)
{
    knob->updateADSRFrom(*synth.getADSRParameters());

    g.fillAll(juce::Colours::black);
}

void MidiKeyboard::resized()
{
    auto area = getLocalBounds();

    midiInputList.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(8));
    keyboardComponent.setBounds(area.reduced(8));
}

void MidiKeyboard::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    synth.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MidiKeyboard::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    synth.getNextAudioBlock(bufferToFill);
}

void MidiKeyboard::releaseResources()
{
    synth.releaseResources();
}

/** Starts listening to a MIDI input device, enabling it if necessary. */
void MidiKeyboard::setMidiInput(int index)
{
    auto list = juce::MidiInput::getAvailableDevices();

    deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, synth.getMidiCollector());

    auto newInput = list[index];

    if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
        deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);

    deviceManager.addMidiInputDeviceCallback(newInput.identifier, synth.getMidiCollector());
    midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

    lastInputIndex = index;
}

void MidiKeyboard::sliderValueChanged(juce::Slider* slider)
{
    if (slider) knob->checkValid(slider); //start-end range limit check

    if (synth.isAvailable())
    {
        auto setting = synth.getCurrentAudioSound()->getSamplerSettings();
        knob->updateSampleInfoTo(setting);
        knob->updateADSRTo(synth.getADSRParameters());
    }

    synth.updateADSR();

}

void MidiKeyboard::updateKeyMappings()
{
    keyboardComponent.setKeyPressBaseOctave(octave);
}

int MidiKeyboard::getOctave() const
{
    return octave;
}

SynthAudioSource* MidiKeyboard::getSynth()
{
    return &synth;
}

void MidiKeyboard::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        progress->setText("", juce::dontSendNotification);

        auto pos = e.getPosition();
        int key = keyboardComponent.getNoteAtPosition(juce::Point<float>(pos.x, pos.y));

        if (key < 0) return;
        rootKey = key;

        update();

        if (synth.isAvailable())
        {
            auto sound = synth.getCurrentAudioSound();
            sound->reprocess(SampleUtils::getFrequencyFromNote(rootKey), sound->getSemitone());
        }
    }
}

void MidiKeyboard::sliderDragEnded(juce::Slider* slider)
{
    if (knob && knob->isSemitoneBender(slider) && synth.isAvailable())
        knob->updateSemitone(synth.getCurrentAudioSound());
}

int MidiKeyboard::getRootKey() const
{
    return rootKey;
}

void MidiKeyboard::update()
{
    if (rootNote) rootNote->setText("Root Note: " + juce::String(SampleUtils::numberToKeyName(rootKey)), juce::dontSendNotification);
}