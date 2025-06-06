/*
  ==============================================================================

    SampleSound.h
    Created: 6 Jun 2025 12:10:15pm
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    A subclass of SynthesiserSound that represents a sampled audio file.

    This class holds the audio data for a sample and defines which MIDI notes
    and velocity ranges should trigger it.
*/
class SampleSound : public juce::SynthesiserSound
{
public:
    //==============================================================================
    /** Creates a new sample sound from an audio file.

        @param name           A name for this sound
        @param source         The audio data to use for the sample
        @param midiNotes      The set of MIDI note numbers that should trigger this sound
        @param midiNoteForNormalPitch The MIDI note number at which the sample should be played with no pitch change
        @param attackTimeSecs Attack time in seconds
        @param releaseTimeSecs Release time in seconds
        @param maxSampleLengthSeconds Maximum length to play before forcing a stop
        @param velocityRange  The velocity range that triggers this sample (0-127)
    */
    SampleSound(const juce::String& name,
        juce::AudioBuffer<float>& source,
        const juce::BigInteger& midiNotes,
        int midiNoteForNormalPitch,
        double attackTimeSecs,
        double releaseTimeSecs,
        double maxSampleLengthSeconds,
        juce::Range<int> velocityRange = juce::Range<int>(0, 127));

    /** Destructor. */
    ~SampleSound() override;

    //==============================================================================
    /** Returns the name of this sample. */
    const juce::String& getName() const noexcept { return name; }

    /** Returns the audio data. */
    juce::AudioBuffer<float>* getAudioData() noexcept { return data.get(); }

    /** Returns the attack time in seconds. */
    double getAttackTime() const noexcept { return attackTime; }

    /** Returns the release time in seconds. */
    double getReleaseTime() const noexcept { return releaseTime; }

    /** Returns the MIDI note at which this sample plays at normal pitch. */
    int getRootMidiNote() const noexcept { return midiRootNote; }

    /** Returns the velocity range for this sample. */
    juce::Range<int> getVelocityRange() const noexcept { return velocityRange; }

    //==============================================================================
    /** Returns true if this sound should be triggered by the given MIDI note. */
    bool appliesToNote(int midiNoteNumber) override;

    /** Returns true if this sound should be triggered by the given velocity. */
    bool appliesToChannel(int midiChannel) override;

    /** Returns true if this sound should be triggered by the given velocity. */
    bool appliesToVelocity(int velocity) const;

    using Ptr = juce::ReferenceCountedObjectPtr<SampleSound>;

private:
    //==============================================================================
    friend class SampleVoice;

    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    double attackTime, releaseTime, maxSampleLength;
    int midiRootNote;
    juce::BigInteger midiNotes;
    juce::Range<int> velocityRange;
    int length;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSound)
};