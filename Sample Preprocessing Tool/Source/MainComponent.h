#pragma once

#include <JuceHeader.h>

#include "MidiKeyboard.h"
#include "Knob.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    void loadSound();
    void saveSound();
    void updateButton();

private:
    //==============================================================================
    // Your private member variables go here...

    class SaveThread : private juce::Thread
    {
    public:
        SaveThread(MainComponent* main) : main(main), juce::Thread("SaveThread")
        {

        }

        void start(juce::File file)
        {
            this->file = file;
            startThread();
        }

    private:
        void run() override
        {
            auto synth = main->keyboard.getSynth();
            auto sound = synth->getCurrentAudioSound();

            sound->createSoundBank(file);

            main->saving = false;
            const juce::MessageManagerLock lock(juce::Thread::getCurrentThread());
            if (lock.lockWasGained())
                main->saveLabel.setText("Completed!", juce::dontSendNotification);

        }

        MainComponent* main;
        juce::File file;
    };

    class LoadThread : private juce::Thread
    {
    public:
        LoadThread(MainComponent* main) : main(main), juce::Thread("LoadThread")
        {

        }

        void start(juce::File file)
        {
            this->file = file;
            startThread();
        }

    private:
        void run() override
        {
            main->keyboard.getSynth()->setSound(file);

            main->loading = false;
            main->haveToInit = true;

            const juce::MessageManagerLock lock(juce::Thread::getCurrentThread());
            if (lock.lockWasGained())
                main->saveLabel.setText("", juce::dontSendNotification);
        }

        MainComponent* main;
        juce::File file;
    };

    void timerCallback() override;

    MidiKeyboard keyboard;
    Knob knob;
    juce::TextButton loader;
    juce::TextButton saver;
    juce::Label rootNote;
    juce::Label saveLabel;

    SaveThread saveThread;
    LoadThread loadThread;

    std::unique_ptr<juce::FileChooser> chooser;
    bool canChoose;
    bool saving;
    bool loading;
    bool haveToInit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
