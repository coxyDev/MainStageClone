/*
  ==============================================================================

    SampleManager.cpp
    Created: Helper for managing sample libraries
    Author:  Joel.Cox

  ==============================================================================
*/

#include "SampleManager.h"

SampleManager::SampleManager()
{
}

SampleManager::~SampleManager()
{
}

juce::File SampleManager::getDefaultSamplesDirectory()
{
    // Try to find samples directory relative to the application
    auto appDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();

    // Look for Samples folder in several common locations
    auto samplesDir = appDir.getChildFile("Samples");
    if (samplesDir.exists())
        return samplesDir;

    // Try going up one level (if app is in build/bin folder)
    samplesDir = appDir.getParentDirectory().getChildFile("Samples");
    if (samplesDir.exists())
        return samplesDir;

    // Try going up two levels (if app is in build/bin/Release folder)
    samplesDir = appDir.getParentDirectory().getParentDirectory().getChildFile("Samples");
    if (samplesDir.exists())
        return samplesDir;

    // If nothing found, return a default location in user's documents
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    return documentsDir.getChildFile("MainStage Sampler").getChildFile("Samples");
}

juce::StringArray SampleManager::findAvailableSFZFiles()
{
    juce::StringArray sfzFiles;
    auto samplesDir = getDefaultSamplesDirectory();

    if (!samplesDir.exists())
        return sfzFiles;

    // Search recursively for .sfz files
    auto files = samplesDir.findChildFiles(juce::File::findFiles, true, "*.sfz");

    for (const auto& file : files)
    {
        sfzFiles.add(file.getFullPathName());
    }

    return sfzFiles;
}

juce::String SampleManager::getLibraryNameFromFile(const juce::File& sfzFile)
{
    // Return the parent directory name if it's not "Samples", otherwise use filename
    auto parentName = sfzFile.getParentDirectory().getFileName();

    if (parentName.equalsIgnoreCase("Samples") || parentName.equalsIgnoreCase("samples"))
        return sfzFile.getFileNameWithoutExtension();
    else
        return parentName;
}