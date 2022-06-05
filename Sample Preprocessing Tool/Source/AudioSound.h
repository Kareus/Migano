#pragma once

#include <JuceHeader.h>
#include "WavetableCreator.h"
#include "PitchShifter.h"
#include "SampleUitls.h"

struct SamplerInfo
{
    float start;
    float end;
    bool useLoop;
    float loopStart;
    float loopEnd;
    bool loop_resampleMode;

    bool isValid() const
    {
        return start <= end && (useLoop ? (loopStart >= start && loopEnd <= end && loopStart <= loopEnd) : 1);
    }
};

struct AudioSound : public juce::SynthesiserSound
{
    AudioSound(juce::AudioSampleBuffer& buffer, int sampleRate, juce::BigInteger midiNotes, float freq) : original(buffer), midiNotes(midiNotes), sampleRate(sampleRate), baseFrequency(freq)
    {
        parameters.attack = 0.0f;
        parameters.decay = 0.0f;
        parameters.sustain = 1.0f;
        parameters.release = 0.0f;

        info.start = 0;
        info.end = getDuration();
        info.useLoop = false;
        info.loopStart = 0;
        info.loopEnd = 0;
        info.loop_resampleMode = false;

        preprocess(buffer);
    }

    AudioSound(juce::File file, juce::BigInteger midiNotes, float freq) : midiNotes(midiNotes), baseFrequency(freq)
    {
        WavetableCreator creator;
        original = creator.createWavetableFromAudio(file, &sampleRate);

        parameters.attack = 0.0f;
        parameters.decay = 0.0f;
        parameters.sustain = 1.0f;
        parameters.release = 0.0f;

        info.start = 0;
        info.end = getDuration();
        info.useLoop = false;
        info.loopStart = 0;
        info.loopEnd = 0;
        info.loop_resampleMode = false;

        preprocess(original);
    }

    AudioSound(juce::File soundBank)
    {
        std::unique_ptr<juce::InputStream> stream(soundBank.createInputStream());

        sampleRate = stream->readInt();
        int noteSize = stream->readInt();

        midiNotes.clear();
        for (int i = 0; i < noteSize; i++)
            stream->readBool() ? midiNotes.setBit(i) : midiNotes.clearBit(i);

        for (int i = 0; i < noteSize; i++)
        {
            if (!midiNotes[i]) continue;

            int numSamples = stream->readInt();
            int numChannels = stream->readInt();

            juce::AudioSampleBuffer buffer(numChannels, numSamples);

            for (int channel = numChannels; --channel >= 0;)
                stream->read(buffer.getWritePointer(channel), numSamples * sizeof(float));

            //we don't add the soundbank buffer to list for fast test
        }

        info.start = stream->readFloat();
        info.end = stream->readFloat();
        info.useLoop = stream->readBool();
        info.loopStart = stream->readFloat();
        info.loopEnd = stream->readFloat();
        info.loop_resampleMode = stream->readBool();

        jassert(info.isValid());

        parameters.attack = stream->readFloat();
        parameters.decay = stream->readFloat();
        parameters.sustain = stream->readFloat();
        parameters.release = stream->readFloat();

        baseFrequency = stream->readFloat();
        currentSemitone = stream->readFloat();

        int numSamples = stream->readInt();
        int numChannels = stream->readInt();
        original = juce::AudioSampleBuffer(numChannels, numSamples);

        for (int channel = numChannels; --channel >= 0;)
            stream->read(original.getWritePointer(channel), numSamples * sizeof(float));

        //re-process sample notes
        int from = SampleUtils::keyNameToNumber("C5"), to = SampleUtils::keyNameToNumber("G6");
        midiNotes.clear();
        midiNotes.setRange(from, to - from + 1, true);
        preprocess(original);
    }

    void preprocess(juce::AudioSampleBuffer& buffer)
    {
        bool state = playable;
        playable = false;
        original = buffer;
        currentSemitone = 0;

        PitchShifter shifter;
        juce::AudioSampleBuffer processor(1, buffer.getNumSamples() * buffer.getNumChannels());
        juce::AudioSampleBuffer output = processor;
        juce::AudioSampleBuffer result(buffer.getNumChannels(), buffer.getNumSamples());

        juce::AudioDataConverters::interleaveSamples(buffer.getArrayOfReadPointers(), processor.getWritePointer(0), buffer.getNumSamples(), buffer.getNumChannels());
        shifter.init(1, sampleRate);

        int highest = midiNotes.getHighestBit();

        for (int index = 0; index <= highest; index++)
        {
            if (!midiNotes[index]) continue;

            double pitch = SampleUtils::getFrequencyFromNote(index + currentSemitone) / baseFrequency;
            shifter.setPitch(pitch);
            shifter.put(processor);
            shifter.receive(output);

            juce::AudioDataConverters::deinterleaveSamples(output.getReadPointer(0), result.getArrayOfWritePointers(), result.getNumSamples(), result.getNumChannels());
            buffers.emplace(index, result);
        }

        playable = state;
    }

    void reprocess(float freq, float semitone = 0)
    {
        bool state = playable;
        playable = false;
        baseFrequency = freq;
        currentSemitone = semitone;

        PitchShifter shifter;
        juce::AudioSampleBuffer processor(1, original.getNumSamples() * original.getNumChannels());
        juce::AudioSampleBuffer output = processor;
        juce::AudioSampleBuffer result(original.getNumChannels(), original.getNumSamples());

        juce::AudioDataConverters::interleaveSamples(original.getArrayOfReadPointers(), processor.getWritePointer(0), original.getNumSamples(), original.getNumChannels());
        shifter.init(1, sampleRate);

        int highest = midiNotes.getHighestBit();

        buffers.clear();

        for (int index = 0; index <= highest; index++)
        {
            if (!midiNotes[index]) continue;
            shifter.setPitch(SampleUtils::getFrequencyFromNote(index + currentSemitone) / baseFrequency);
            shifter.put(processor);
            shifter.receive(output);

            juce::AudioDataConverters::deinterleaveSamples(output.getReadPointer(0), result.getArrayOfWritePointers(), result.getNumSamples(), result.getNumChannels());
            buffers.emplace(index, result);
        }

        playable = state;
    }

    void createSoundBank(juce::File file)
    {
        std::unique_ptr<juce::FileOutputStream> stream(file.createOutputStream());
        stream->setPosition(0);
        stream->truncate();

        const int highest = 128;

        stream->writeInt(sampleRate);
        stream->writeInt(highest);

        for (int i = 0; i < highest; i++)
            stream->writeBool(true);

        PitchShifter shifter;

        int numSamples = original.getNumSamples();
        int numChannels = original.getNumChannels();

        for (savingIndex = 0; savingIndex < highest; savingIndex++)
        {
            stream->writeInt(numSamples);
            stream->writeInt(numChannels);

            //sample shift goes wrong with shared variables... idk, is it bug?
            juce::AudioSampleBuffer processor(1, original.getNumSamples() * original.getNumChannels());
            juce::AudioSampleBuffer output = processor;
            juce::AudioSampleBuffer result(original.getNumChannels(), original.getNumSamples());

            juce::AudioDataConverters::interleaveSamples(original.getArrayOfReadPointers(), processor.getWritePointer(0), original.getNumSamples(), original.getNumChannels());
            shifter.init(1, sampleRate);
            shifter.setPitch(SampleUtils::getFrequencyFromNote(savingIndex + currentSemitone) / baseFrequency);
            shifter.put(processor);
            shifter.receive(output);

            juce::AudioDataConverters::deinterleaveSamples(output.getReadPointer(0), result.getArrayOfWritePointers(), result.getNumSamples(), result.getNumChannels());
            for (int channel = numChannels; --channel >= 0;)
                stream->write(result.getReadPointer(channel), numSamples * sizeof(float));
        }

        stream->writeFloat(info.start);
        stream->writeFloat(info.end);
        stream->writeBool(info.useLoop);
        stream->writeFloat(info.loopStart);
        stream->writeFloat(info.loopEnd);
        stream->writeBool(info.loop_resampleMode);

        stream->writeFloat(parameters.attack);
        stream->writeFloat(parameters.decay);
        stream->writeFloat(parameters.sustain);
        stream->writeFloat(parameters.release);

        //for processing tool below
        stream->writeFloat(baseFrequency);
        stream->writeFloat(currentSemitone);

        stream->writeInt(numSamples);
        stream->writeInt(numChannels);

        for (int channel = numChannels; --channel >= 0;)
            stream->write(original.getReadPointer(channel), numSamples * sizeof(float));
        stream->flush();
    }

    int getSavingIndex() const
    {
        return savingIndex;
    }

    bool appliesToNote(int note) override
    {
        return playable && midiNotes[note];
    }

    bool appliesToChannel(int) override { return playable; }

    SamplerInfo* getSamplerSettings()
    {
        return &info;
    }

    void setPlayable(bool playable)
    {
        this->playable = playable;
    }

    void setSemitone(float semitone)
    {
        if (semitone != currentSemitone) reprocess(baseFrequency, semitone);
    }

    int getSampleRate() const
    {
        return sampleRate;
    }

    float getDuration() const
    {
        return (float)original.getNumSamples() / sampleRate;
    }

    float getBaseFrequency() const
    {
        return baseFrequency;
    }

    float getSemitone() const
    {
        return currentSemitone;
    }

    juce::ADSR::Parameters* getParameters()
    {
        return &parameters;
    }

private:
    friend class AudioVoice;
    SamplerInfo info;
    juce::AudioSampleBuffer original;
    std::unordered_map<int, juce::AudioSampleBuffer> buffers;
    juce::BigInteger midiNotes;
    bool playable = true;
    int sampleRate;
    int savingIndex = 0;
    float baseFrequency = 523.25;
    float currentSemitone = 0;
    juce::ADSR::Parameters parameters;
};