#pragma once

#include "AudioSound.h"
#include "ADSREnvelope.h"

struct AudioVoice : public juce::SynthesiserVoice
{
    AudioVoice() : adsr(getSampleRate())
    {
        currentNote = 60;
    }

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<AudioSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound* sound, int currentPitchWheelPosition) override
    {
        adsr.enterStage(ADSREnvelope::ENVELOPE_STAGE_OFF);

        //TODO: is there any way to implement fast real-time pitch shift in renderNextBlock?
        bendAmount = bendRange * (float)(currentPitchWheelPosition - 8192) / 8192.0;
        currentNote = midiNoteNumber;

        audio = dynamic_cast<AudioSound*>(sound);

        currentSamplerInfo = audio->getSamplerSettings();
        level = velocity * 0.7;

        sampleRate = getSampleRate();

        processed = SampleUtils::resample(audio->buffers[midiNoteNumber], audio->getSampleRate(), sampleRate);
        loop = SampleUtils::resample(audio->buffers[60], audio->getSampleRate(), sampleRate);
        delta = std::pow(2.0, (midiNoteNumber + bendAmount - 60) / 12.0);

        sourceIndex = SampleUtils::SecondsToBufferIndex(currentSamplerInfo->start, sampleRate);

        adsr.setParameters(parameters);
        adsr.setSampleRate(sampleRate);
        adsr.enterStage(ADSREnvelope::ENVELOPE_STAGE_ATTACK);
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (adsr.getCurrentStage() != ADSREnvelope::ENVELOPE_STAGE_OFF && adsr.getCurrentStage() != ADSREnvelope::ENVELOPE_STAGE_RELEASE)
                adsr.enterStage(ADSREnvelope::ENVELOPE_STAGE_RELEASE);
        }
        else
        {
            clearCurrentNote();
            adsr.enterStage(ADSREnvelope::ENVELOPE_STAGE_OFF);
        }
    }

    void setADSRParams(float attack, float decay, float sustain, float release)
    {
        parameters.attack = attack;
        parameters.decay = decay;
        parameters.sustain = sustain;
        parameters.release = release;
    }

    void setADSRParams(juce::ADSR::Parameters& param)
    {
        setADSRParams(param.attack, param.decay, param.sustain, param.release);
    }

    void pitchWheelMoved(int newValue) override
    {
        bendAmount = bendRange * (float)(newValue - 8192) / 8192.0;
        delta = std::pow(2.0, (currentNote + bendAmount - 60) / 12.0);
    }

    void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        auto stage = adsr.getCurrentStage();
        if (stage != ADSREnvelope::ENVELOPE_STAGE_OFF)
        {
            int loopStart = SampleUtils::SecondsToBufferIndex(currentSamplerInfo->loopStart, sampleRate);
            int loopEnd = SampleUtils::SecondsToBufferIndex(currentSamplerInfo->loopEnd, sampleRate);
            int maxSamples = SampleUtils::SecondsToBufferIndex(currentSamplerInfo->end, sampleRate);
            adsr.setParameters(parameters);

            if (currentSamplerInfo->useLoop && stage != ADSREnvelope::ENVELOPE_STAGE_RELEASE)
            {
                if (sourceIndex < loopStart)
                {
                    int bufferIndex = 0;
                    auto length = juce::jmin(numSamples, loopStart - sourceIndex);

                    while (bufferIndex < length)
                    {
                        float sample = level * adsr.nextSample();

                        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample(i, startSample + bufferIndex, processed.getSample(i, sourceIndex) * sample);

                        bufferIndex++;
                        sourceIndex++;
                    }

                    if (length < numSamples)
                    {
                        bufferIndex = length;

                        if (currentSamplerInfo->loop_resampleMode)
                        {
                            double source = sourceIndex;

                            while (bufferIndex < numSamples)
                            {
                                if (loopEnd - loopStart <= 0) break;

                                float sample = level * adsr.nextSample();
                                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                                    outputBuffer.addSample(i, startSample + bufferIndex, loop.getSample(i, source) * sample);

                                bufferIndex++;
                                source += delta;

                                if (source >= loopEnd)
                                    source = source - loopEnd + loopStart;
                            }

                            sourceIndex = source;
                        }
                        else
                        {
                            while (bufferIndex < numSamples)
                            {
                                float sample = level * adsr.nextSample();
                                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                                    outputBuffer.addSample(i, startSample + bufferIndex, processed.getSample(i, sourceIndex) * sample);

                                bufferIndex++;
                                if (++sourceIndex >= loopEnd)
                                    sourceIndex = loopStart;
                            }
                        }
                    }
                }
                else
                {
                    int bufferIndex = 0;

                    if (currentSamplerInfo->loop_resampleMode)
                    {
                        double source = sourceIndex;

                        while (bufferIndex < numSamples)
                        {
                            if (loopEnd - loopStart <= 0) break;

                            float sample = level * adsr.nextSample();
                            for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                                outputBuffer.addSample(i, startSample + bufferIndex, loop.getSample(i, source) * sample);

                            bufferIndex++;
                            source += delta;

                            if (source >= loopEnd)
                                source = source - loopEnd + loopStart;
                        }

                        sourceIndex = source;
                    }
                    else
                    {
                        while (bufferIndex < numSamples)
                        {
                            float sample = level * adsr.nextSample();
                            for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                                outputBuffer.addSample(i, startSample + bufferIndex, processed.getSample(i, sourceIndex) * sample);

                            bufferIndex++;

                            if (++sourceIndex >= loopEnd)
                                sourceIndex = loopStart;
                        }
                    }
                }
            }
            else
            {
                if (sourceIndex >= maxSamples)
                {
                    adsr.enterStage(ADSREnvelope::ENVELOPE_STAGE_OFF);
                    clearCurrentNote();
                    return;
                }

                auto length = juce::jmin(numSamples, maxSamples - sourceIndex);

                if (length > 0)
                {
                    int bufferIndex = 0;

                    while (bufferIndex < length)
                    {
                        float sample = level * adsr.nextSample();
                        for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                            outputBuffer.addSample(i, startSample + bufferIndex, processed.getSample(i, sourceIndex) * sample);

                        bufferIndex++;
                        sourceIndex++;
                    }
                }
            }
        }
    }

    void setBendRange(float bendRange)
    {
        this->bendRange = bendRange;
    }

    using SynthesiserVoice::renderNextBlock;

private:
    double level = 1.0;

    PitchShifter shifter;
    juce::AudioSampleBuffer processed, loop;
    int sourceIndex = 0;
    int sampleRate = 0;

    AudioSound* audio = nullptr;
    ADSREnvelope adsr;
    juce::ADSR::Parameters parameters;
    SamplerInfo* currentSamplerInfo = nullptr;

    double bendRange = 2.0;
    double bendAmount = 0.0;
    int currentNote;

    double delta;
};
