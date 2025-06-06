/*
  ==============================================================================

    SFZLoader.cpp
    Created: 6 Jun 2025 12:09:51pm
    Author:  Joel.Cox

  ==============================================================================
*/

#include "SFZLoader.h"

SFZLoader::SFZLoader()
{
    formatManager.registerBasicFormats();
}

SFZLoader::~SFZLoader()
{
}

juce::Array<SampleSound::Ptr> SFZLoader::loadSFZ(const juce::File& sfzFile)
{
    juce::Array<SampleSound::Ptr> sounds;

    if (!sfzFile.exists())
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
            "File Not Found",
            "SFZ file does not exist: " + sfzFile.getFullPathName());
        return sounds;
    }

    juce::String content = sfzFile.loadFileAsString();
    juce::StringArray lines = juce::StringArray::fromLines(content);

    juce::Array<SFZRegion> regions;
    SFZRegion currentRegion;

    for (const auto& line : lines)
    {
        parseLine(line.trim(), currentRegion, regions);
    }

    // Create SampleSound objects from regions
    for (const auto& region : regions)
    {
        if (region.sample.isNotEmpty())
        {
            auto sound = createSampleSound(region, sfzFile);
            if (sound != nullptr)
                sounds.add(sound);
        }
    }

    return sounds;
}

void SFZLoader::parseLine(const juce::String& line, SFZRegion& currentRegion,
    juce::Array<SFZRegion>& regions)
{
    if (line.isEmpty() || line.startsWith("//"))
        return;

    if (line.startsWith("<region>"))
    {
        // Start a new region
        if (currentRegion.sample.isNotEmpty())
            regions.add(currentRegion);

        currentRegion = SFZRegion(); // Reset to defaults
        return;
    }

    if (line.startsWith("<group>"))
    {
        // For now, we'll treat groups similar to regions
        return;
    }

    // Parse key=value pairs
    if (line.contains("="))
    {
        auto tokens = juce::StringArray::fromTokens(line, "=", "");
        if (tokens.size() >= 2)
        {
            auto key = tokens[0].trim();
            auto value = tokens[1].trim();

            if (key == "sample")
                currentRegion.sample = value;
            else if (key == "lokey")
                currentRegion.lokey = value.getIntValue();
            else if (key == "hikey")
                currentRegion.hikey = value.getIntValue();
            else if (key == "lovel")
                currentRegion.lovel = value.getIntValue();
            else if (key == "hivel")
                currentRegion.hivel = value.getIntValue();
            else if (key == "pitch_keycenter")
                currentRegion.pitch_keycenter = value.getIntValue();
            else if (key == "key")
            {
                currentRegion.key = value.getIntValue();
                currentRegion.lokey = currentRegion.hikey = currentRegion.key;
                currentRegion.pitch_keycenter = currentRegion.key;
            }
            else if (key == "ampeg_attack")
                currentRegion.ampeg_attack = value.getDoubleValue();
            else if (key == "ampeg_release")
                currentRegion.ampeg_release = value.getDoubleValue();
        }
    }

    // Add the last region if we reach the end
    if (currentRegion.sample.isNotEmpty())
    {
        bool found = false;
        for (const auto& region : regions)
        {
            if (region.sample == currentRegion.sample &&
                region.lokey == currentRegion.lokey &&
                region.hikey == currentRegion.hikey)
            {
                found = true;
                break;
            }
        }
        if (!found)
            regions.add(currentRegion);
    }
}

SampleSound::Ptr SFZLoader::createSampleSound(const SFZRegion& region, const juce::File& sfzFile)
{
    // Resolve the sample file path
    juce::File sampleFile = sfzFile.getParentDirectory().getChildFile(region.sample);

    if (!sampleFile.exists())
    {
        juce::Logger::writeToLog("Sample file not found: " + sampleFile.getFullPathName());
        return nullptr;
    }

    // Load the audio file
    auto audioBuffer = loadAudioFile(sampleFile);
    if (audioBuffer == nullptr)
    {
        juce::Logger::writeToLog("Failed to load audio file: " + sampleFile.getFullPathName());
        return nullptr;
    }

    // Create MIDI note range
    juce::BigInteger midiNotes;
    midiNotes.clear();
    for (int note = region.lokey; note <= region.hikey; ++note)
        midiNotes.setBit(note);

    // Create velocity range
    juce::Range<int> velocityRange(region.lovel, region.hivel);

    // Create the sample sound
    auto sound = new SampleSound(
        sampleFile.getFileNameWithoutExtension(),
        *audioBuffer,
        midiNotes,
        region.pitch_keycenter,
        region.ampeg_attack,
        region.ampeg_release,
        10.0, // max sample length in seconds
        velocityRange
    );

    return sound;
}

std::unique_ptr<juce::AudioBuffer<float>> SFZLoader::loadAudioFile(const juce::File& audioFile)
{
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(audioFile));

    if (reader == nullptr)
        return nullptr;

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(
        (int)reader->numChannels,
        (int)reader->lengthInSamples
    );

    reader->read(buffer.get(), 0, (int)reader->lengthInSamples, 0, true, true);

    return buffer;
}