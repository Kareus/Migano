#pragma once

#include "AudioSound.h"
#include "AudioVoice.h"

//==============================================================================
class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState, int voiceSize = 64)
        : keyboardState(keyState), soundIndex(0)
    {
        parameters.attack = 0.0f;
        parameters.decay = 0.0f;
        parameters.sustain = 1.0f;
        parameters.release = 0.0f;

        for (auto i = 0; i < voiceSize; ++i)
            synth.addVoice(new AudioVoice());

        updateADSR();
    }

    void setSound(juce::File file)
    {
        synth.clearSounds();

        if (file.getFileExtension() == ".msb") synth.addSound(new AudioSound(file));
        else
        {
            int from = SampleUtils::keyNameToNumber("C5"), to = SampleUtils::keyNameToNumber("G6");

            juce::BigInteger midiNotes;
            midiNotes.clear();
            midiNotes.setRange(from, to - from + 1, true);

            auto sound = new AudioSound(file, midiNotes, SampleUtils::getFrequencyFromNote(from));
            synth.addSound(sound);
        }
        
        parameters = *getCurrentAudioSound()->getParameters();
    }

    AudioSound* getAudioSound(int index)
    {
        return dynamic_cast<AudioSound*>(synth.getSound(index).get());
    }

    AudioSound* getCurrentAudioSound()
    {
        return getAudioSound(soundIndex);
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
        midiCollector.reset(sampleRate); // [10]
    }

    void releaseResources() override
    {
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        juce::MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples); // [11]

        keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
            bufferToFill.startSample, bufferToFill.numSamples);
    }

    juce::MidiMessageCollector* getMidiCollector()
    {
        return &midiCollector;
    }

    juce::Synthesiser* getSynth()
    {
        return &synth;
    }

    int getCurrentSoundIndex() const
    {
        return soundIndex;
    }

    juce::ADSR::Parameters* getADSRParameters()
    {
        return &parameters;
    }

    void updateADSR()
    {
        if (synth.getNumSounds() > 0)
        {
            auto param = getCurrentAudioSound()->getParameters();
            param->attack = parameters.attack;
            param->decay = parameters.decay;
            param->sustain = parameters.sustain;
            param->release = parameters.release;
        }

        for (auto i = 0; i < synth.getNumVoices(); i++)
        {
            auto voice = dynamic_cast<AudioVoice*>(synth.getVoice(i));
            voice->setADSRParams(parameters);
        }
    }

    bool isAvailable() const
    {
        return synth.getNumSounds() > 0 && synth.getNumVoices() > 0;
    }

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;

    int soundIndex = 0;
    juce::ADSR::Parameters parameters;
};