/*
  ==============================================================================

    SFZLoader.h
    Created: 6 Jun 2025 12:09:51pm
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SampleSound.h"

//==============================================================================
/**
    A simple SFZ file parser that can load basic SFZ instruments.

    This class provides functionality to parse SFZ files and create SampleSound
    objects that can be used with the sampler engine.
*/
class SFZLoader
{
public:
    //==============================================================================
    /** Creates an SFZLoader. */
    SFZLoader();

    /** Destructor. */
    ~SFZLoader();

    //==============================================================================
    /** Loads an SFZ file and returns an array of SampleSound objects.

        @param sfzFile The SFZ file to load
        @returns An array of SampleSound objects created from the SFZ file
    */
    juce::Array<SampleSound::Ptr> loadSFZ(const juce::File& sfzFile);

private:
    //==============================================================================
    struct SFZRegion
    {
        juce::String sample;
        int lokey = 0;
        int hikey = 127;
        int lovel = 0;
        int hivel = 127;
        int pitch_keycenter = 60;
        double ampeg_attack = 0.0;
        double ampeg_release = 0.1;
        int key = -1;
        juce::String group;
        int seq_length = 1;
        int seq_position = 1;
    };

    //==============================================================================
    /** Parses a single line of SFZ data. */
    void parseLine(const juce::String& line, SFZRegion& currentRegion,
        juce::Array<SFZRegion>& regions);

    /** Creates a SampleSound from an SFZ region. */
    SampleSound::Ptr createSampleSound(const SFZRegion& region,
        const juce::File& sfzFile);

    /** Loads an audio file and returns it as an AudioBuffer. */
    std::unique_ptr<juce::AudioBuffer<float>> loadAudioFile(const juce::File& audioFile);

    //==============================================================================
    juce::AudioFormatManager formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SFZLoader)
};