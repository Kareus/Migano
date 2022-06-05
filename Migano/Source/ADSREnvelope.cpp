#include "ADSREnvelope.h"

ADSREnvelope::ADSREnvelope(double sampleRate, juce::ADSR::Parameters params) : minimumLevel(0.0001), currentStage(ENVELOPE_STAGE_OFF), currentLevel(minimumLevel), multiplier(1.0), sampleRate(sampleRate), currentSampleIndex(0), nextStageSampleIndex(0)
{
    stageValue[ENVELOPE_STAGE_OFF] = 0.0;
    setParameters(params);
}

void ADSREnvelope::setParameters(juce::ADSR::Parameters params)
{
    this->params = params;
    stageValue[ENVELOPE_STAGE_ATTACK] = params.attack;
    stageValue[ENVELOPE_STAGE_DECAY] = params.decay;
    stageValue[ENVELOPE_STAGE_SUSTAIN] = params.sustain;
    stageValue[ENVELOPE_STAGE_RELEASE] = params.release;
}

double ADSREnvelope::nextSample()
{
    if (currentStage != ENVELOPE_STAGE_OFF && currentStage != ENVELOPE_STAGE_SUSTAIN)
    {
        while (currentStage != ENVELOPE_STAGE_OFF && currentStage != ENVELOPE_STAGE_SUSTAIN && currentSampleIndex == nextStageSampleIndex)
        {
            EnvelopeStage newStage = static_cast<EnvelopeStage>((currentStage + 1) % kNumEnvelopeStages);
            enterStage(newStage);
        }

        if (currentStage != ENVELOPE_STAGE_OFF && currentStage != ENVELOPE_STAGE_SUSTAIN)
        {
            currentLevel += multiplier;
            currentSampleIndex++;
        }
    }

    return currentLevel;
}

void ADSREnvelope::calculateMultiplier(double startLevel, double endLevel, unsigned long long lengthInSamples)
{
    multiplier = (endLevel - startLevel) / (lengthInSamples);
}

void ADSREnvelope::enterStage(EnvelopeStage newStage)
{
    currentStage = newStage;
    currentSampleIndex = 0;
    if (currentStage == ENVELOPE_STAGE_OFF || currentStage == ENVELOPE_STAGE_SUSTAIN)
        nextStageSampleIndex = 0;
    else
        nextStageSampleIndex = stageValue[currentStage] * sampleRate;

    switch (newStage)
    {
        case ENVELOPE_STAGE_OFF:
            currentLevel = 0.0;
            multiplier = 1.0;
            break;

        case ENVELOPE_STAGE_ATTACK:
            currentLevel = minimumLevel;
            calculateMultiplier(currentLevel, 1.0, nextStageSampleIndex);
            break;

        case ENVELOPE_STAGE_DECAY:
            currentLevel = 1.0;
            calculateMultiplier(currentLevel, std::fmax(stageValue[ENVELOPE_STAGE_SUSTAIN], minimumLevel), nextStageSampleIndex);
            break;

        case ENVELOPE_STAGE_SUSTAIN:
            currentLevel = stageValue[ENVELOPE_STAGE_SUSTAIN];
            multiplier = 1.0;
            break;

        case ENVELOPE_STAGE_RELEASE:
            calculateMultiplier(currentLevel, minimumLevel, nextStageSampleIndex);
        break;

    default:
        break;
    }
}

void ADSREnvelope::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
}