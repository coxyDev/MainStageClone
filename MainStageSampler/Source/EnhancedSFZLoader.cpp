/*
  ==============================================================================

    EnhancedSFZLoader.cpp
    Created: Professional SFZ parser for advanced sample libraries
    Author:  Joel.Cox

  ==============================================================================
*/

#include "EnhancedSFZLoader.h"

EnhancedSFZLoader::EnhancedSFZLoader()
{
    formatManager.registerBasicFormats();
}

EnhancedSFZLoader::~EnhancedSFZLoader()
{
}

juce::Array<SampleSound::Ptr> EnhancedSFZLoader::loadSFZ(const juce::File& sfzFile)
{
    DBG("Enhanced SFZ Loader: Starting to load " + sfzFile.getFullPathName());

    // Clear previous state
    variables.clear();
    masters.clear();
    groups.clear();
    regions.clear();
    currentSFZFile = sfzFile;
    currentContext = ParseContext::Global;
    currentMasterIndex = -1;
    currentGroupIndex = -1;

    if (!sfzFile.exists())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "File Not Found",
            "SFZ file does not exist: " + sfzFile.getFullPathName());
        return {};
    }

    try
    {
        // Parse the main file and all includes
        parseFile(sfzFile);

        DBG("Parsed " + juce::String(variables.size()) + " variables");
        DBG("Parsed " + juce::String(masters.size()) + " masters");
        DBG("Parsed " + juce::String(groups.size()) + " groups");
        DBG("Parsed " + juce::String(regions.size()) + " regions");

        // Apply inheritance hierarchy
        applyInheritance();

        // Create sample sounds
        auto sounds = createSampleSounds();

        DBG("Created " + juce::String(sounds.size()) + " sample sounds");
        return sounds;
    }
    catch (const std::exception& e)
    {
        DBG("Error loading SFZ: " + juce::String(e.what()));
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "SFZ Parse Error",
            "Error parsing SFZ file: " + juce::String(e.what()));
        return {};
    }
}

void EnhancedSFZLoader::parseFile(const juce::File& file)
{
    DBG("Parsing file: " + file.getFullPathName());

    if (!file.exists())
    {
        DBG("File does not exist: " + file.getFullPathName());
        return;
    }

    juce::String content = file.loadFileAsString();
    juce::StringArray lines = juce::StringArray::fromLines(content);

    for (const auto& line : lines)
    {
        parseLine(line.trim());
    }
}

void EnhancedSFZLoader::parseLine(const juce::String& line)
{
    // Skip empty lines and comments
    if (line.isEmpty() || line.startsWith("//"))
        return;

    // Handle preprocessor directives
    if (line.startsWith("#define"))
    {
        handleDefine(line);
        return;
    }

    if (line.startsWith("#include"))
    {
        handleInclude(line);
        return;
    }

    // Handle section headers
    if (line.startsWith("<") && line.endsWith(">"))
    {
        handleSectionHeader(line);
        return;
    }

    // Handle opcodes (key=value pairs)
    if (line.contains("="))
    {
        auto tokens = juce::StringArray::fromTokens(line, "=", "");
        if (tokens.size() >= 2)
        {
            auto key = tokens[0].trim();
            auto value = tokens[1].trim();

            // Substitute variables
            value = substituteVariables(value);

            handleOpcode(key, value);
        }
    }
}

void EnhancedSFZLoader::handleDefine(const juce::String& line)
{
    // Parse #define $VARIABLE value
    auto tokens = juce::StringArray::fromTokens(line, " \t", "");
    if (tokens.size() >= 3)
    {
        auto varName = tokens[1]; // Should start with $
        auto varValue = tokens[2];

        DBG("Defining variable: " + varName + " = " + varValue);

        // Store variable
        SFZVariable var;
        var.name = varName;
        var.value = varValue;
        variables.add(var);
    }
}

void EnhancedSFZLoader::handleInclude(const juce::String& line)
{
    // Parse #include "filename"
    auto startQuote = line.indexOf("\"");
    auto endQuote = line.lastIndexOf("\"");

    if (startQuote >= 0 && endQuote > startQuote)
    {
        auto filename = line.substring(startQuote + 1, endQuote);
        auto includeFile = currentSFZFile.getParentDirectory().getChildFile(filename);

        DBG("Including file: " + filename + " -> " + includeFile.getFullPathName());

        // Recursively parse the included file
        parseFile(includeFile);
    }
}

void EnhancedSFZLoader::handleSectionHeader(const juce::String& line)
{
    auto section = line.substring(1, line.length() - 1).toLowerCase();

    DBG("Section header: " + section);

    if (section == "master")
    {
        currentContext = ParseContext::Master;
        masters.add(SFZMaster());
        currentMasterIndex = masters.size() - 1;
        currentGroupIndex = -1;
    }
    else if (section == "group")
    {
        currentContext = ParseContext::Group;
        groups.add(SFZGroup());
        currentGroupIndex = groups.size() - 1;
    }
    else if (section == "region")
    {
        currentContext = ParseContext::Region;
        regions.add(SFZRegion());
    }
    else if (section == "global" || section == "control")
    {
        currentContext = ParseContext::Global;
    }
    // Ignore other sections like <curve> for now
}

void EnhancedSFZLoader::handleOpcode(const juce::String& key, const juce::String& value)
{
    DBG("Opcode: " + key + " = " + value + " (context: " + juce::String((int)currentContext) + ")");

    switch (currentContext)
    {
    case ParseContext::Master:
        applyOpcodeToMaster(key, value);
        break;

    case ParseContext::Group:
        applyOpcodeToGroup(key, value);
        break;

    case ParseContext::Region:
        applyOpcodeToRegion(key, value);
        break;

    case ParseContext::Global:
        // Global opcodes affect all subsequent regions
        break;
    }
}

juce::String EnhancedSFZLoader::substituteVariables(const juce::String& input)
{
    juce::String result = input;

    for (const auto& var : variables)
    {
        result = result.replace(var.name, var.value);
    }

    return result;
}

void EnhancedSFZLoader::applyOpcodeToMaster(const juce::String& key, const juce::String& value)
{
    if (currentMasterIndex >= 0)
    {
        masters.getReference(currentMasterIndex).opcodes.add(SFZOpcode(key, value));
    }
}

void EnhancedSFZLoader::applyOpcodeToGroup(const juce::String& key, const juce::String& value)
{
    if (currentGroupIndex >= 0)
    {
        groups.getReference(currentGroupIndex).opcodes.add(SFZOpcode(key, value));
    }
}

void EnhancedSFZLoader::applyOpcodeToRegion(const juce::String& key, const juce::String& value)
{
    if (regions.isEmpty())
        return;

    auto& region = regions.getReference(regions.size() - 1);

    // Store all opcodes for advanced processing
    region.opcodes.add(SFZOpcode(key, value));

    // Handle common opcodes directly
    if (key == "sample")
        region.sample = value;
    else if (key == "lokey")
        region.lokey = clampMidiNote(parseIntOpcode(value));
    else if (key == "hikey")
        region.hikey = clampMidiNote(parseIntOpcode(value));
    else if (key == "key")
    {
        int keyNum = clampMidiNote(parseIntOpcode(value));
        region.key = keyNum;
        region.lokey = region.hikey = keyNum;
        region.pitch_keycenter = keyNum;
    }
    else if (key == "lovel")
        region.lovel = clampVelocity(parseIntOpcode(value));
    else if (key == "hivel")
        region.hivel = clampVelocity(parseIntOpcode(value));
    else if (key == "pitch_keycenter")
        region.pitch_keycenter = clampMidiNote(parseIntOpcode(value));
    else if (key == "volume")
        region.volume = clampGain(parseFloatOpcode(value));
    else if (key == "pan")
        region.pan = juce::jlimit(-100.0, 100.0, parseFloatOpcode(value));
    else if (key == "tune")
        region.tune = juce::jlimit(-100, 100, parseIntOpcode(value));
    else if (key == "transpose")
        region.transpose = juce::jlimit(-127, 127, parseIntOpcode(value));

    // ADSR
    else if (key == "ampeg_attack")
        region.ampeg_attack = juce::jmax(0.0, parseFloatOpcode(value));
    else if (key == "ampeg_decay")
        region.ampeg_decay = juce::jmax(0.0, parseFloatOpcode(value));
    else if (key == "ampeg_sustain")
        region.ampeg_sustain = juce::jlimit(0.0, 100.0, parseFloatOpcode(value));
    else if (key == "ampeg_release")
        region.ampeg_release = juce::jmax(0.0, parseFloatOpcode(value));

    // Trigger and sequencing
    else if (key == "trigger")
        region.trigger = value;
    else if (key == "seq_length")
        region.seq_length = juce::jmax(1, parseIntOpcode(value));
    else if (key == "seq_position")
        region.seq_position = juce::jmax(1, parseIntOpcode(value));

    // Controllers (CC)
    else if (key == "locc64")
        region.locc64 = clampVelocity(parseIntOpcode(value));
    else if (key == "hicc64")
        region.hicc64 = clampVelocity(parseIntOpcode(value));

    // Switches
    else if (key == "sw_lokey")
        region.sw_lokey = clampMidiNote(parseIntOpcode(value));
    else if (key == "sw_hikey")
        region.sw_hikey = clampMidiNote(parseIntOpcode(value));
    else if (key == "sw_last")
        region.sw_last = clampMidiNote(parseIntOpcode(value));
    else if (key == "sw_label")
        region.sw_label = value;

    // Group and exclusivity
    else if (key == "group")
        region.group = parseIntOpcode(value);
    else if (key == "off_by")
        region.off_by = parseIntOpcode(value);
}

void EnhancedSFZLoader::applyInheritance()
{
    // Apply master -> group -> region inheritance
    for (auto& region : regions)
    {
        // Apply master opcodes
        if (currentMasterIndex >= 0)
        {
            for (const auto& opcode : masters[currentMasterIndex].opcodes)
            {
                // Apply if not already set in region
                bool hasOpcode = false;
                for (const auto& regionOpcode : region.opcodes)
                {
                    if (regionOpcode.key == opcode.key)
                    {
                        hasOpcode = true;
                        break;
                    }
                }
                if (!hasOpcode)
                {
                    applyOpcodeToRegion(opcode.key, opcode.value);
                }
            }
        }

        // Apply group opcodes
        if (currentGroupIndex >= 0)
        {
            for (const auto& opcode : groups[currentGroupIndex].opcodes)
            {
                bool hasOpcode = false;
                for (const auto& regionOpcode : region.opcodes)
                {
                    if (regionOpcode.key == opcode.key)
                    {
                        hasOpcode = true;
                        break;
                    }
                }
                if (!hasOpcode)
                {
                    applyOpcodeToRegion(opcode.key, opcode.value);
                }
            }
        }
    }
}

juce::Array<SampleSound::Ptr> EnhancedSFZLoader::createSampleSounds()
{
    juce::Array<SampleSound::Ptr> sounds;

    for (const auto& region : regions)
    {
        if (region.sample.isNotEmpty())
        {
            auto sound = createSampleSound(region);
            if (sound != nullptr)
                sounds.add(sound);
        }
    }

    return sounds;
}

SampleSound::Ptr EnhancedSFZLoader::createSampleSound(const SFZRegion& region)
{
    // Resolve sample file path
    juce::File sampleFile = currentSFZFile.getParentDirectory().getChildFile(region.sample);

    // Try multiple locations if not found
    if (!sampleFile.exists())
    {
        sampleFile = currentSFZFile.getSiblingFile(region.sample);
    }
    if (!sampleFile.exists())
    {
        sampleFile = currentSFZFile.getParentDirectory().getChildFile("Samples").getChildFile(region.sample);
    }
    if (!sampleFile.exists())
    {
        // Try with different extensions
        auto baseName = juce::File::createFileWithoutCheckingPath(region.sample).getFileNameWithoutExtension();
        juce::StringArray extensions = { ".wav", ".flac", ".ogg", ".aiff" };

        for (const auto& ext : extensions)
        {
            sampleFile = currentSFZFile.getParentDirectory().getChildFile(baseName + ext);
            if (sampleFile.exists()) break;

            sampleFile = currentSFZFile.getParentDirectory().getChildFile("Samples").getChildFile(baseName + ext);
            if (sampleFile.exists()) break;
        }
    }

    if (!sampleFile.exists())
    {
        DBG("Sample file not found: " + region.sample);
        return nullptr;
    }

    auto audioBuffer = loadAudioFile(sampleFile);
    if (audioBuffer == nullptr)
    {
        DBG("Failed to load audio: " + sampleFile.getFullPathName());
        return nullptr;
    }

    // Create MIDI note range
    juce::BigInteger midiNotes;
    for (int note = region.lokey; note <= region.hikey; ++note)
        midiNotes.setBit(note);

    // Create velocity range
    juce::Range<int> velocityRange(region.lovel, region.hivel);

    DBG("Creating sound: " + sampleFile.getFileName() +
        " MIDI:" + juce::String(region.lokey) + "-" + juce::String(region.hikey) +
        " Vel:" + juce::String(region.lovel) + "-" + juce::String(region.hivel) +
        " Root:" + juce::String(region.pitch_keycenter));

    // Create the sample sound with enhanced parameters
    auto sound = new SampleSound(
        sampleFile.getFileNameWithoutExtension(),
        *audioBuffer,
        midiNotes,
        region.pitch_keycenter,
        region.ampeg_attack,
        region.ampeg_release,
        10.0, // max sample length
        velocityRange
    );

    return sound;
}

std::unique_ptr<juce::AudioBuffer<float>> EnhancedSFZLoader::loadAudioFile(const juce::File& audioFile)
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

int EnhancedSFZLoader::parseIntOpcode(const juce::String& value)
{
    return value.getIntValue();
}

double EnhancedSFZLoader::parseFloatOpcode(const juce::String& value)
{
    return value.getDoubleValue();
}

bool EnhancedSFZLoader::parseBoolOpcode(const juce::String& value)
{
    return value.getIntValue() != 0;
}

int EnhancedSFZLoader::clampMidiNote(int note)
{
    return juce::jlimit(0, 127, note);
}

int EnhancedSFZLoader::clampVelocity(int velocity)
{
    return juce::jlimit(0, 127, velocity);
}

double EnhancedSFZLoader::clampGain(double gain)
{
    return juce::jlimit(-144.0, 6.0, gain); // Reasonable dB range
}