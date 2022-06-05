#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : keyboard(), knob((juce::Slider::Listener*)&keyboard), canChoose(true), saveThread(this), loadThread(this)
{
    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }

    setWantsKeyboardFocus(false);
    setMouseClickGrabsKeyboardFocus(false);

    addAndMakeVisible(keyboard);

    addAndMakeVisible(knob);
    keyboard.init(&knob);

    addAndMakeVisible(loader);
    loader.setMouseClickGrabsKeyboardFocus(false);
    loader.setButtonText("Load");
    loader.onClick = [this] { loadSound(); };

    addAndMakeVisible(saver);
    saver.setMouseClickGrabsKeyboardFocus(false);
    saver.setButtonText("Save");
    saver.onClick = [this] { saveSound(); };

    addAndMakeVisible(saveLabel);
    saveLabel.setMouseClickGrabsKeyboardFocus(false);

    addAndMakeVisible(rootNote);
    rootNote.setText("Root Note: " + juce::MidiMessage::getMidiNoteName(keyboard.getRootKey(), true, true, 5), juce::dontSendNotification);
    rootNote.setMouseClickGrabsKeyboardFocus(false);

    keyboard.initRootNote(&rootNote);
    keyboard.initProgress(&saveLabel);
    setSize(600, 375);
    saving = false;
    loading = false;
    haveToInit = false;

    updateButton();
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    // You can add your drawing code here!

}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    keyboard.setBounds(keyboard.getBounds().translated(0, 280));
    knob.setBounds(knob.getBounds());
    loader.setBounds(420, 250, 60, 25);
    saver.setBounds(490, 250, 60, 25);
    saveLabel.setBounds(415, 220, 200, 25);
    rootNote.setBounds(300, 250, 120, 25);
}

void MainComponent::loadSound()
{
    if (!canChoose) return;

    canChoose = false;
    updateButton();

    chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
        juce::File{},
        "*.wav;*.mp3;*.aiff;*.ogg;*.msb");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            _sleep(1000); //bug occurs when selecting too fast

            if (file != juce::File{})
            {
                loading = true;

                loadThread.start(file);
                startTimer(100);
            }
            else
            {
                canChoose = true;
                keyboard.grabKeyboardFocus();
                updateButton();
            }
        });
}

void MainComponent::saveSound()
{
    if (!canChoose || !keyboard.getSynth()->isAvailable()) return;

    canChoose = false;
    updateButton();
    saveLabel.setText("Processing...", juce::dontSendNotification);

    chooser = std::make_unique<juce::FileChooser>("Select the folder to process the soundbank file...",
        juce::File(),
        "*.msb");
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            _sleep(1000);

            if (file != juce::File())
            {
                saving = true;

                saveThread.start(file);
                startTimer(100);
            }
            else
            {
                canChoose = true;
                keyboard.grabKeyboardFocus();
                updateButton();
            }
        });
}

void MainComponent::updateButton()
{
    if (!canChoose)
    {
        loader.setEnabled(false);
        saver.setEnabled(false);
    }
    else
    {
        loader.setEnabled(true);
        if (keyboard.getSynth()->isAvailable())
        {
            saver.setEnabled(true);
            keyboard.grabKeyboardFocus();
        }
    }

}

void MainComponent::timerCallback()
{
    if (!saving && !loading)
    {
        stopTimer();

        canChoose = true;
        updateButton();

        if (haveToInit)
        {
            haveToInit = false;
            keyboard.init(&knob);
        }

        return;
    }

    if (saving)
    {
        auto synth = keyboard.getSynth();
        auto sound = synth->getCurrentAudioSound();
        saveLabel.setText("Processing... : " + std::to_string(sound->getSavingIndex()) + " / 128", juce::dontSendNotification);
    }
    else if (loading)
    {
        saveLabel.setText("Loading...", juce::dontSendNotification);
    }
}