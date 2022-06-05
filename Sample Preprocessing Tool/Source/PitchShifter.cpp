/*
  ==============================================================================

    PitchShifter.cpp
    Created: 16 May 2022 5:48:01pm
    Author:  Kareus

  ==============================================================================
*/

#include "PitchShifter.h"

PitchShifter::PitchShifter() :
    shifter(nullptr)
{

}

PitchShifter::~PitchShifter()
{
    if (shifter) delete shifter;
}

void PitchShifter::init(int numChannels, unsigned int sampleRate)
{
    if (shifter)
    {
        flush();
        clear();

        delete shifter;
    }

    shifter = new soundtouch::SoundTouch();
    shifter->setChannels(numChannels);
    shifter->setSampleRate(sampleRate);
}

void PitchShifter::put(const float* sample, unsigned int numSamples)
{
    shifter->putSamples(sample, numSamples);
}

void PitchShifter::put(const juce::AudioSampleBuffer& buffer)
{
    shifter->putSamples(buffer.getReadPointer(0), buffer.getNumSamples());
}

void PitchShifter::clear()
{
    if (shifter) shifter->clear();
}

void PitchShifter::flush()
{
    if (shifter) shifter->flush();
}

void PitchShifter::setPitch(double pitch)
{
    shifter->setPitch(pitch);
}

void PitchShifter::setSemitone(double semitone)
{
    shifter->setPitchSemiTones(semitone);
}

unsigned int PitchShifter::receive(float* sample, unsigned int numSamples)
{
    return shifter->receiveSamples(sample, numSamples);
}

void PitchShifter::receive(juce::AudioSampleBuffer& buffer)
{
    shifter->receiveSamples(buffer.getWritePointer(0), buffer.getNumSamples());
}