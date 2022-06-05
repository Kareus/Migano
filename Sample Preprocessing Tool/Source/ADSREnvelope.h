#pragma once

//original source: http://www.martin-finke.de/blog/articles/audio-plugins-011-envelopes/

#include <cmath>
#include <JuceHeader.h>

class ADSREnvelope
{
public:
    enum EnvelopeStage
    {
        ENVELOPE_STAGE_OFF = 0,
        ENVELOPE_STAGE_ATTACK,
        ENVELOPE_STAGE_DECAY,
        ENVELOPE_STAGE_SUSTAIN,
        ENVELOPE_STAGE_RELEASE,
        kNumEnvelopeStages
    };

    void enterStage(EnvelopeStage newStage);

    double nextSample();

    void setSampleRate(double newSampleRate);

    inline EnvelopeStage getCurrentStage() const { return currentStage; };

    const double minimumLevel;

    ADSREnvelope(double sampleRate = 44100.0, juce::ADSR::Parameters params = juce::ADSR::Parameters(0, 0, 1, 0));

    void setParameters(juce::ADSR::Parameters params);

private:
    EnvelopeStage currentStage;
    double currentLevel;
    double multiplier;
    double sampleRate;
    double stageValue[kNumEnvelopeStages];
    juce::ADSR::Parameters params;

    void calculateMultiplier(double startLevel, double endLevel, unsigned long long lengthInSamples);

    unsigned long long currentSampleIndex;
    unsigned long long nextStageSampleIndex;
};