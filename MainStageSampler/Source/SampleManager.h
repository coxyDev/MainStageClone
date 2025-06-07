/*
  ==============================================================================

    SampleManager.h
    Created: Helper for managing sample libraries
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
    Helper class for managing sample libraries and default paths.
*/
class SampleManager
{
public:
    //==============================================================================
    SampleManager();
    ~SampleManager();

    //==============================================================================
    /** Returns the default samples directory relative to the application. */
    static juce::File getDefaultSamplesDirectory();

    /** Scans the default samples directory for SFZ files. */
    static juce::StringArray findAvailableSFZFiles();

    /** Returns a user-friendly name from an SFZ file path. */
    static juce::String getLibraryNameFromFile(const juce::File& sfzFile);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleManager)
};