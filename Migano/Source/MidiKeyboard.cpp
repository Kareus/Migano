#include "MidiKeyboard.h"
#include "config.h"

MidiKeyboard::MidiKeyboard(SynthAudioSource* synth, Knob* knob, juce::MidiKeyboardState& state, juce::MidiKeyboardComponent::Orientation orientation) :
    keyboardState(state), keyboardComponent(state, orientation), startTime(juce::Time::getMillisecondCounterHiRes() * 0.001),
    synth(synth), knob(knob)
{
    setOpaque(true);

    bool _STANDALONE = juce::JUCEApplicationBase::isStandaloneApp();

    if (_STANDALONE)
    {
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
    }
    else
        setMidiInput(0);

    addAndMakeVisible(keyboardComponent);
    keyboardComponent.clearKeyMappings();
    keyboardComponent.addKeyListener(this);
    keyboardComponent.setOctaveForMiddleC(5);

    if (_STANDALONE)
    {
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
        startTimer(400);
    }

    setSize(600, 100);
}

MidiKeyboard::~MidiKeyboard()
{
    if (juce::JUCEApplicationBase::isStandaloneApp())
        deviceManager.removeMidiInputDeviceCallback(juce::MidiInput::getAvailableDevices()[midiInputList.getSelectedItemIndex()].identifier, synth->getMidiCollector());
}

void MidiKeyboard::timerCallback()
{
    grab();
    stopTimer();
}

void MidiKeyboard::paint(juce::Graphics& g)
{
    knob->updateADSRFrom(*synth->getADSRParameters());

    g.fillAll(juce::Colours::black);
}

void MidiKeyboard::resized()
{
    auto area = getLocalBounds();

    if (juce::JUCEApplicationBase::isStandaloneApp())
        midiInputList.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(8));

    keyboardComponent.setBounds(area.reduced(8));
}

/** Starts listening to a MIDI input device, enabling it if necessary. */
void MidiKeyboard::setMidiInput(int index)
{
    auto list = juce::MidiInput::getAvailableDevices();

    deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, synth->getMidiCollector());

    auto newInput = list[index];

    if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
        deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);

    deviceManager.addMidiInputDeviceCallback(newInput.identifier, synth->getMidiCollector());
    midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

    lastInputIndex = index;
}

bool MidiKeyboard::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    if (juce::JUCEApplicationBase::isStandaloneApp())
    {
        if (key.getTextCharacter() == 'z')
        {
            octave--;
            updateKeyMappings();
        }
        else if (key.getTextCharacter() == 'x')
        {
            octave++;
            updateKeyMappings();
        }
    }

    return true;
}

void MidiKeyboard::updateKeyMappings()
{
    if (juce::JUCEApplicationBase::isStandaloneApp())
    {
        synth->getSynth()->allNotesOff(0, false);

        if (octave < 0) octave = 0;
        if (octave > 9) octave = 9;
        keyboardComponent.setKeyPressBaseOctave(octave);
    }
}

int MidiKeyboard::getOctave() const
{
    return octave;
}

void MidiKeyboard::grab()
{
    if (keyboardComponent.isShowing() && juce::JUCEApplicationBase::isStandaloneApp())
        keyboardComponent.grabKeyboardFocus();
}