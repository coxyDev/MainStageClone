/*
  ==============================================================================

    SampleSound.cpp
    Created: 6 Jun 2025 12:10:15pm
    Author:  Joel.Cox

  ==============================================================================
*/

#include "SampleSound.h"

SampleSound::SampleSound(const juce::String& soundName,
    juce::AudioBuffer<float>& source,
    const juce::BigInteger& notes,
    int midiNoteForNormalPitch,
    double attackTimeSecs,
    double releaseTimeSecs,
    double maxSampleLengthSeconds,
    juce::Range<int> velRange)
    : name(soundName),
    data(std::make_unique<juce::AudioBuffer<float>>()),
    attackTime(attackTimeSecs),
    releaseTime(releaseTimeSecs),
    maxSampleLength(maxSampleLengthSeconds),
    midiRootNote(midiNoteForNormalPitch),
    midiNotes(notes),
    velocityRange(velRange)
{
    *data = source;
    length = source.getNumSamples();
}

SampleSound::~SampleSound()
{
}

bool SampleSound::appliesToNote(int midiNoteNumber)
{
    return midiNotes[midiNoteNumber];
}

bool SampleSound::appliesToChannel(int /*midiChannel*/)
{
    // This sampler ignores the MIDI channel and always responds to all channels
    return true;
}

bool SampleSound::appliesToVelocity(int velocity) const
{
    return velocityRange.contains(velocity);
}