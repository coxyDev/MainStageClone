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

    DBG("Attempting to load SFZ file: " + sfzFile.getFullPathName());

    if (!sfzFile.exists())
    {
        DBG("SFZ file does not exist!");
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "File Not Found",
            "SFZ file does not exist: " + sfzFile.getFullPathName());
        return sounds;
    }

    juce::String content = sfzFile.loadFileAsString();
    DBG("SFZ file content length: " + juce::String(content.length()));
    DBG("First 200 chars: " + content.substring(0, 200));

    juce::StringArray lines = juce::StringArray::fromLines(content);
    DBG("Number of lines: " + juce::String(lines.size()));

    juce::Array<SFZRegion> regions;
    SFZRegion currentRegion;

    for (const auto& line : lines)
    {
        parseLine(line.trim(), currentRegion, regions);
    }

    // Add the final region if it has a sample
    if (currentRegion.sample.isNotEmpty())
    {
        regions.add(currentRegion);
    }

    DBG("Found " + juce::String(regions.size()) + " regions");

    // Create SampleSound objects from regions
    for (int i = 0; i < regions.size(); ++i)
    {
        const auto& region = regions[i];
        DBG("Processing region " + juce::String(i) + ": " + region.sample);

        if (region.sample.isNotEmpty())
        {
            auto sound = createSampleSound(region, sfzFile);
            if (sound != nullptr)
            {
                sounds.add(sound);
                DBG("Successfully created sound for: " + region.sample);
            }
            else
            {
                DBG("Failed to create sound for: " + region.sample);
            }
        }
    }

    DBG("Total sounds created: " + juce::String(sounds.size()));
    return sounds;
}

void SFZLoader::parseLine(const juce::String& line, SFZRegion& currentRegion,
    juce::Array<SFZRegion>& regions)
{
    if (line.isEmpty() || line.startsWith("//"))
        return;

    DBG("Parsing line: " + line);

    if (line.startsWith("<region>"))
    {
        // Start a new region
        if (currentRegion.sample.isNotEmpty())
        {
            DBG("Adding completed region: " + currentRegion.sample);
            regions.add(currentRegion);
        }

        currentRegion = SFZRegion(); // Reset to defaults
        DBG("Starting new region");
        return;
    }

    if (line.startsWith("<group>") || line.startsWith("<global>") || line.startsWith("<control>"))
    {
        DBG("Found header: " + line);
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

            DBG("Key-value pair: " + key + " = " + value);

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
}

SampleSound::Ptr SFZLoader::createSampleSound(const SFZRegion& region, const juce::File& sfzFile)
{
    // Resolve the sample file path
    juce::File sampleFile = sfzFile.getParentDirectory().getChildFile(region.sample);

    DBG("Looking for sample file: " + sampleFile.getFullPathName());

    if (!sampleFile.exists())
    {
        // Try different common locations
        sampleFile = sfzFile.getSiblingFile(region.sample);
        DBG("Trying sibling: " + sampleFile.getFullPathName());

        if (!sampleFile.exists())
        {
            // Try samples subfolder
            sampleFile = sfzFile.getParentDirectory().getChildFile("Samples").getChildFile(region.sample);
            DBG("Trying Samples subfolder: " + sampleFile.getFullPathName());
        }

        if (!sampleFile.exists())
        {
            juce::Logger::writeToLog("Sample file not found: " + region.sample);
            DBG("Sample file not found after trying multiple locations");
            return nullptr;
        }
    }

    DBG("Found sample file: " + sampleFile.getFullPathName());

    // Load the audio file
    auto audioBuffer = loadAudioFile(sampleFile);
    if (audioBuffer == nullptr)
    {
        juce::Logger::writeToLog("Failed to load audio file: " + sampleFile.getFullPathName());
        DBG("Failed to load audio file");
        return nullptr;
    }

    DBG("Successfully loaded audio: " + juce::String(audioBuffer->getNumChannels()) + " channels, " + juce::String(audioBuffer->getNumSamples()) + " samples");

    // Create MIDI note range
    juce::BigInteger midiNotes;
    midiNotes.clear();
    for (int note = region.lokey; note <= region.hikey; ++note)
        midiNotes.setBit(note);

    // Create velocity range
    juce::Range<int> velocityRange(region.lovel, region.hivel);

    DBG("Creating sound - MIDI range: " + juce::String(region.lokey) + "-" + juce::String(region.hikey) +
        ", Velocity range: " + juce::String(region.lovel) + "-" + juce::String(region.hivel) +
        ", Root note: " + juce::String(region.pitch_keycenter));

    // Validate MIDI range
    if (region.lokey > region.hikey || region.lokey < 0 || region.hikey > 127)
    {
        DBG("Invalid MIDI range - skipping this region");
        return nullptr;
    }

    if (region.lovel > region.hivel || region.lovel < 0 || region.hivel > 127)
    {
        DBG("Invalid velocity range - skipping this region");
        return nullptr;
    }

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
    DBG("Attempting to load audio file: " + audioFile.getFullPathName());

    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(audioFile));

    if (reader == nullptr)
    {
        DBG("Failed to create reader for: " + audioFile.getFullPathName());
        return nullptr;
    }

    DBG("Audio file info - Channels: " + juce::String(reader->numChannels) +
        ", Sample rate: " + juce::String(reader->sampleRate) +
        ", Length: " + juce::String(reader->lengthInSamples));

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(
        (int)reader->numChannels,
        (int)reader->lengthInSamples
    );

    reader->read(buffer.get(), 0, (int)reader->lengthInSamples, 0, true, true);

    DBG("Successfully loaded audio buffer");
    return buffer;
}