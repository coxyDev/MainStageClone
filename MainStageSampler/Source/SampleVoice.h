/*
  ==============================================================================

    SampleVoice.h
    Created: 6 Jun 2025 12:10:02pm
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SampleSound.h"

//==============================================================================
/**
    A subclass of SynthesiserVoice that can play a SampleSound.

    To use it, create a Synthesiser, add some SampleVoice objects to it, then
    give it some SampleSound objects to play.
*/
class SampleVoice : public juce::SynthesiserVoice
{
public:
    //==============================================================================
    /** Creates a SampleVoice. */
    SampleVoice();

    /** Destructor. */
    ~SampleVoice() override;

    //==============================================================================
    /** Returns true if this voice can play the given sound. */
    bool canPlaySound(juce::SynthesiserSound*) override;

    /** Called to start a new note. */
    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int pitchWheel) override;

    /** Called to stop a note. */
    void stopNote(float velocity, bool allowTailOff) override;

    /** Called to let the voice know that the pitch wheel has been moved. */
    void pitchWheelMoved(int newValue) override;

    /** Called to let the voice know that a midi CC message has been received. */
    void controllerMoved(int controllerNumber, int newValue) override;

    /** Renders the next block of audio data. */
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

    using Ptr = juce::ReferenceCountedObjectPtr<SampleVoice>;

private:
    //==============================================================================
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleVoice)
};