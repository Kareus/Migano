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

    void addSound(juce::File file)
    {
#if _REAL_TIME_
        synth.addSound(new AudioSound(file, juce::MidiMessage::getMidiNoteInHertz(SampleUtils::keyNameToNumber("C5"))));
#elif _PREPROCESS_ALL_
        juce::BigInteger midiNotes;
        midiNotes.setRange(0, 128, true);
        synth.addSound(new AudioSound(file, midiNotes));
#else //_LOAD_ALL_
        synth.addSound(new AudioSound(file));
#endif
        
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

    void setCurrentSoundIndex(int index)
    {
        soundIndex = index;

        for (auto i = 0; i < synth.getNumSounds(); i++)
        {
            auto sound = dynamic_cast<AudioSound*>(synth.getSound(i).get());
            sound->setPlayable(i == soundIndex);
        }
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

        renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    }

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, const juce::MidiBuffer& inputMidi, int startSample, int numSamples)
    {
        synth.renderNextBlock(outputBuffer, inputMidi, startSample, numSamples);
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