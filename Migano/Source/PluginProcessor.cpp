/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "config.h"

//==============================================================================
MiganoAudioProcessor::MiganoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters(*this, nullptr, juce::Identifier("Params"),
        {
            //adsr
            std::make_unique<juce::AudioParameterFloat>("attack", "Attack", juce::NormalisableRange<float>(0.0, MAX_DURATION, 0.001), 0.0),
            std::make_unique<juce::AudioParameterFloat>("decay", "Decay", juce::NormalisableRange<float>(0.0, MAX_DURATION, 0.001), 0.0),
            std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", juce::NormalisableRange<float>(0.0, 1.0, 0.001), 1.0),
            std::make_unique<juce::AudioParameterFloat>("release", "Release", juce::NormalisableRange<float>(0.0, MAX_DURATION, 0.001), 0.0),

            //sample (I didn't attach the sample start/end knobs, as it is difficult to fix automation exception
            std::make_unique<juce::AudioParameterBool>("useLoop", "Use Loop", false),
            std::make_unique<juce::AudioParameterBool>("loop_resampleMode", "Resample Loop", false)
        }
    )
#endif
    , synth(keyboardState), presetID(0)
{
    SOUNDBANK_PATH = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("MiganoSoundBank").getFullPathName().toUTF8();

#if _REAL_TIME_ || _PREPROCESS_ALL_
    juce::File bank(SOUNDBANK_PATH);

    addSoundItem(bank.getChildFile("ddeol.wav"));
    addSoundItem(bank.getChildFile("ddie.wav"));
    addSoundItem(bank.getChildFile("na.wav"));
    addSoundItem(bank.getChildFile("reul.wav"));
    addSoundItem(bank.getChildFile("bwa.wav"));
    addSoundItem(bank.getChildFile("go.wav"));
    addSoundItem(bank.getChildFile("bbang.wav"));
    addSoundItem(bank.getChildFile("zzi.wav"));

#else
    readAllSoundBanks(SOUNDBANK_PATH);
#endif
}

MiganoAudioProcessor::~MiganoAudioProcessor()
{
}

//==============================================================================
const juce::String MiganoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MiganoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MiganoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MiganoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MiganoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MiganoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MiganoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MiganoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MiganoAudioProcessor::getProgramName (int index)
{
    return {};
}

void MiganoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MiganoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepareToPlay(samplesPerBlock, sampleRate);
}

void MiganoAudioProcessor::releaseResources()
{
    synth.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MiganoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MiganoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool MiganoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MiganoAudioProcessor::createEditor()
{
    return new MiganoAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void MiganoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    juce::XmlElement* root = new juce::XmlElement("root");

    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    root->addChildElement(xml.get());
    xml.release();

    //custom
    juce::XmlElement* preset = new juce::XmlElement("presetID");
    preset->addTextElement(juce::String(presetID));
    root->addChildElement(preset);

    auto sound = synth.getCurrentAudioSound();
    if (sound != nullptr)
    {
        auto sample = synth.getCurrentAudioSound()->getSamplerSettings();

        juce::XmlElement* start = new juce::XmlElement("start");
        start->addTextElement(juce::String(sample->start));
        root->addChildElement(start);

        juce::XmlElement* end = new juce::XmlElement("end");
        end->addTextElement(juce::String(sample->end));
        root->addChildElement(end);

        juce::XmlElement* loopStart = new juce::XmlElement("loopStart");
        loopStart->addTextElement(juce::String(sample->loopStart));
        root->addChildElement(loopStart);

        juce::XmlElement* loopEnd = new juce::XmlElement("loopEnd");
        loopEnd->addTextElement(juce::String(sample->loopEnd));
        root->addChildElement(loopEnd);
    }

    std::unique_ptr<juce::XmlElement> to_save(root);
    copyXmlToBinary(*to_save, destData);
}

void MiganoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr)
    {
        if (xml->hasTagName("root"))
        {
            juce::XmlElement* valueTree = xml->getChildByName(parameters.state.getType());

            if (valueTree == nullptr)
                return;

            parameters.replaceState(juce::ValueTree::fromXml(*valueTree));

            juce::XmlElement* preset = xml->getChildByName("presetID");
            if (preset != nullptr)
            {
                juce::XmlElement* id = preset->getFirstChildElement();
                if (id != nullptr && id->isTextElement())
                    presetID = id->getText().getIntValue();
            }

            juce::XmlElement* start = xml->getChildByName("start");
            if (start != nullptr)
            {
                juce::XmlElement* val = start->getFirstChildElement();
                if (val != nullptr && val->isTextElement())
                    loadedInfo.start = val->getText().getFloatValue();
            }

            juce::XmlElement* end = xml->getChildByName("end");
            if (end != nullptr)
            {
                juce::XmlElement* val = end->getFirstChildElement();
                if (val != nullptr && val->isTextElement())
                    loadedInfo.end = val->getText().getFloatValue();
            }

            juce::XmlElement* loopStart = xml->getChildByName("loopStart");
            if (loopStart != nullptr)
            {
                juce::XmlElement* val = loopStart->getFirstChildElement();
                if (val != nullptr && val->isTextElement())
                    loadedInfo.loopStart = val->getText().getFloatValue();
            }

            juce::XmlElement* loopEnd = xml->getChildByName("loopEnd");
            if (start != nullptr)
            {
                juce::XmlElement* val = loopEnd->getFirstChildElement();
                if (val != nullptr && val->isTextElement())
                    loadedInfo.loopEnd = val->getText().getFloatValue();
            }

            loadedInfo.useLoop = parameters.getParameter("useLoop")->getValue();
            loadedInfo.loop_resampleMode = parameters.getParameter("loop_resampleMode")->getValue();

            synth.setCurrentSoundIndex(presetID);

            auto adsr = synth.getADSRParameters();
            adsr->attack = *(float*)parameters.getRawParameterValue("attack");
            adsr->decay = *(float*)parameters.getRawParameterValue("decay");
            adsr->sustain = *(float*)parameters.getRawParameterValue("sustain");
            adsr->release = *(float*)parameters.getRawParameterValue("release");

            synth.updateADSR();

            auto sound = synth.getCurrentAudioSound();
            if (sound != nullptr)
                *sound->getSamplerSettings() = loadedInfo;
        }
    }
}

//

SynthAudioSource* MiganoAudioProcessor::getSynth()
{
    return &synth;
}

juce::MidiKeyboardState& MiganoAudioProcessor::getKeyboardState()
{
    return keyboardState;
}

void MiganoAudioProcessor::setPresetID(int presetID)
{
    this->presetID = presetID;
    synth.setCurrentSoundIndex(presetID);
}

int MiganoAudioProcessor::getPresetID() const
{
    return presetID;
}

void MiganoAudioProcessor::addSoundItem(juce::File file)
{
    if (!file.existsAsFile()) return;

    synth.addSound(file);

    auto name = file.getRelativePathFrom(juce::File(SOUNDBANK_PATH));
    name = name.substring(0, name.lastIndexOf("."));
    presetList.add(name);
}

void MiganoAudioProcessor::readAllSoundBanks(const std::string& path)
{
    juce::File dir(path);

    if (dir.exists() && dir.isDirectory())
    {
        auto banks = dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, "*.msb");

        for (auto file : banks)
            addSoundItem(file);
    }
}

const juce::StringArray& MiganoAudioProcessor::getPresetList() const
{
    return presetList;
}

// 

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MiganoAudioProcessor();
}
