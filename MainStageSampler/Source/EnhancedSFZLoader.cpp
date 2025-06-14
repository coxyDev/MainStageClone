/*
  ==============================================================================

    EnhancedSFZLoader.cpp - Complete Salamander Grand Piano Parser
    Author:  Joel.Cox

  ==============================================================================
*/

#include "EnhancedSFZLoader.h"

EnhancedSFZLoader::EnhancedSFZLoader()
{
    formatManager.registerBasicFormats();
    // Note: FLAC support is already included in registerBasicFormats()
    // Don't register FLAC again as it causes an assertion failure
}

EnhancedSFZLoader::~EnhancedSFZLoader()
{
}

juce::Array<SampleSound::Ptr> EnhancedSFZLoader::loadSFZ(const juce::File& sfzFile)
{
    DBG("=== SALAMANDER SFZ LOADER ===");
    DBG("Loading: " + sfzFile.getFullPathName());

    // Clear previous state
    variables.clear();
    masters.clear();
    groups.clear();
    regions.clear();
    currentSFZFile = sfzFile;
    currentContext = ParseContext::Global;
    currentMasterIndex = -1;
    currentGroupIndex = -1;
    defaultPath.clear();

    if (!sfzFile.exists())
    {
        DBG("ERROR: SFZ file does not exist!");
        return {};
    }

    try
    {
        // Parse the main file and all includes
        parseFile(sfzFile);

        DBG("=== PARSING RESULTS ===");
        DBG("Variables: " + juce::String(variables.size()));
        DBG("Masters: " + juce::String(masters.size()));
        DBG("Groups: " + juce::String(groups.size()));
        DBG("Regions: " + juce::String(regions.size()) + " *** KEY NUMBER ***");
        DBG("Default path: '" + defaultPath + "'");

        if (regions.size() == 0)
        {
            DBG("*** CRITICAL ERROR: NO REGIONS FOUND! ***");
            DBG("This means the include files aren't being processed correctly.");
            return {};
        }

        // Apply inheritance hierarchy
        applyInheritance();

        // Create sample sounds
        auto sounds = createSampleSounds();

        DBG("=== FINAL RESULT ===");
        DBG("Created " + juce::String(sounds.size()) + " sample sounds");

        if (sounds.size() == 0)
        {
            DBG("*** ERROR: No sounds created from " + juce::String(regions.size()) + " regions! ***");
        }

        return sounds;
    }
    catch (const std::exception& e)
    {
        DBG("Exception in SFZ loading: " + juce::String(e.what()));
        return {};
    }
}

void EnhancedSFZLoader::parseFile(const juce::File& file)
{
    DBG("Parsing file: " + file.getFileName());

    if (!file.exists())
    {
        DBG("ERROR: File does not exist: " + file.getFullPathName());
        return;
    }

    juce::String content = file.loadFileAsString();
    if (content.isEmpty())
    {
        DBG("ERROR: File is empty: " + file.getFileName());
        return;
    }

    juce::StringArray lines = juce::StringArray::fromLines(content);
    DBG("Processing " + juce::String(lines.size()) + " lines from " + file.getFileName());

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

    DBG("Parsing line: " + line);

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

    // Handle complex lines that start with section headers but have more content
    if (line.startsWith("<") && line.contains(">"))
    {
        auto closeBracket = line.indexOf(">");
        if (closeBracket > 0)
        {
            auto header = line.substring(0, closeBracket + 1);
            auto remainder = line.substring(closeBracket + 1).trim();

            DBG("Complex line: " + line);
            DBG("  Header: " + header);
            DBG("  Remainder: " + remainder);

            // Process the header first
            handleSectionHeader(header);

            // Process the remainder if it exists
            if (remainder.isNotEmpty())
            {
                parseComplexRemainder(remainder);
            }
            return;
        }
    }

    // Handle simple section headers
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

            // Substitute variables BEFORE processing
            value = substituteVariables(value);

            handleOpcode(key, value);
        }
        return;
    }

    // Log unhandled lines for debugging
    if (line.trim().isNotEmpty())
    {
        DBG("Unhandled line: '" + line + "'");
    }
}

void EnhancedSFZLoader::parseComplexRemainder(const juce::String& remainder)
{
    DBG("Parsing complex remainder: " + remainder);

    // Split by spaces but keep quoted strings together
    juce::StringArray parts;
    bool inQuotes = false;
    juce::String currentPart;

    for (int i = 0; i < remainder.length(); ++i)
    {
        juce::juce_wchar c = remainder[i];

        if (c == '"')
        {
            inQuotes = !inQuotes;
            currentPart += c;
        }
        else if (c == ' ' && !inQuotes)
        {
            if (currentPart.isNotEmpty())
            {
                parts.add(currentPart.trim());
                currentPart.clear();
            }
        }
        else
        {
            currentPart += c;
        }
    }

    // Add the last part
    if (currentPart.isNotEmpty())
    {
        parts.add(currentPart.trim());
    }

    DBG("  Found " + juce::String(parts.size()) + " parts:");
    for (int i = 0; i < parts.size(); ++i)
    {
        DBG("    Part " + juce::String(i) + ": '" + parts[i] + "'");
    }

    // Process each part
    for (const auto& part : parts)
    {
        if (part.startsWith("#include"))
        {
            DBG("  Processing include: " + part);
            handleInclude(part);
        }
        else if (part.contains("="))
        {
            auto tokens = juce::StringArray::fromTokens(part, "=", "");
            if (tokens.size() >= 2)
            {
                auto key = tokens[0].trim();
                auto value = tokens[1].trim();
                value = substituteVariables(value);

                DBG("  Processing opcode: " + key + " = " + value);
                handleOpcode(key, value);
            }
        }
        else if (part.isNotEmpty())
        {
            DBG("  Unhandled part: " + part);
        }
    }
}

void EnhancedSFZLoader::handleDefine(const juce::String& line)
{
    auto tokens = juce::StringArray::fromTokens(line, " \t", "");
    if (tokens.size() >= 3)
    {
        auto varName = tokens[1];
        auto varValue = tokens[2];

        // Store variable
        SFZVariable var;
        var.name = varName;
        var.value = varValue;
        variables.add(var);

        DBG("Variable: " + varName + " = " + varValue);
    }
}

void EnhancedSFZLoader::handleInclude(const juce::String& line)
{
    auto startQuote = line.indexOf("\"");
    auto endQuote = line.lastIndexOf("\"");

    if (startQuote >= 0 && endQuote > startQuote)
    {
        auto filename = line.substring(startQuote + 1, endQuote);
        auto includeFile = currentSFZFile.getParentDirectory().getChildFile(filename);

        DBG("Including: " + filename);
        DBG("Full path: " + includeFile.getFullPathName());
        DBG("File exists: " + juce::String(includeFile.exists() ? "YES" : "NO"));

        if (includeFile.exists())
        {
            // Recursively parse the included file
            parseFile(includeFile);
        }
        else
        {
            DBG("ERROR: Include file not found: " + filename);
        }
    }
}

void EnhancedSFZLoader::handleSectionHeader(const juce::String& line)
{
    auto section = line.substring(1, line.length() - 1).toLowerCase();

    DBG("Section: " + section);

    if (section == "master")
    {
        currentContext = ParseContext::Master;
        masters.add(SFZMaster());
        currentMasterIndex = masters.size() - 1;
        currentGroupIndex = -1;
        DBG("Created master " + juce::String(currentMasterIndex));
    }
    else if (section == "group")
    {
        currentContext = ParseContext::Group;
        groups.add(SFZGroup());
        currentGroupIndex = groups.size() - 1;

        // Associate with current master
        if (currentMasterIndex >= 0)
        {
            groups.getReference(currentGroupIndex).masterIndex = currentMasterIndex;
        }
        DBG("Created group " + juce::String(currentGroupIndex) + " (master=" + juce::String(currentMasterIndex) + ")");
    }
    else if (section == "region")
    {
        currentContext = ParseContext::Region;
        SFZRegion newRegion;
        newRegion.masterIndex = currentMasterIndex;
        newRegion.groupIndex = currentGroupIndex;
        regions.add(newRegion);
        DBG("Created region " + juce::String(regions.size() - 1) + " (master=" + juce::String(currentMasterIndex) + " group=" + juce::String(currentGroupIndex) + ")");
    }
    else if (section == "global" || section == "control")
    {
        currentContext = ParseContext::Global;
    }
    else if (section == "curve")
    {
        currentContext = ParseContext::Global; // Ignore curves for now
    }
}

void EnhancedSFZLoader::handleOpcode(const juce::String& key, const juce::String& value)
{
    // Handle global opcodes first
    if (key == "default_path")
    {
        defaultPath = value;
        DBG("Set default_path: " + defaultPath);
        return;
    }

    switch (currentContext)
    {
    case ParseContext::Master:
        if (currentMasterIndex >= 0)
        {
            masters.getReference(currentMasterIndex).opcodes.add(SFZOpcode(key, value));
        }
        break;

    case ParseContext::Group:
        if (currentGroupIndex >= 0)
        {
            groups.getReference(currentGroupIndex).opcodes.add(SFZOpcode(key, value));
        }
        break;

    case ParseContext::Region:
        applyOpcodeToRegion(key, value);
        break;

    case ParseContext::Global:
        // Global opcodes - store for later application
        break;
    }
}

void EnhancedSFZLoader::applyOpcodeToRegion(const juce::String& key, const juce::String& value)
{
    if (regions.isEmpty())
    {
        DBG("ERROR: Trying to apply opcode to region but no regions exist!");
        return;
    }

    auto& region = regions.getReference(regions.size() - 1);

    // Store the opcode
    region.opcodes.add(SFZOpcode(key, value));

    // Parse common opcodes
    if (key == "sample")
    {
        region.sample = value;
        DBG("  sample: " + value);
    }
    else if (key == "lokey")
    {
        region.lokey = parseNoteValue(value);
        DBG("  lokey: " + juce::String(region.lokey));
    }
    else if (key == "hikey")
    {
        region.hikey = parseNoteValue(value);
        DBG("  hikey: " + juce::String(region.hikey));
    }
    else if (key == "key")
    {
        int keyNum = parseNoteValue(value);
        region.key = keyNum;
        region.lokey = region.hikey = keyNum;
        region.pitch_keycenter = keyNum;
        DBG("  key: " + juce::String(keyNum));
    }
    else if (key == "lovel")
    {
        region.lovel = juce::jlimit(0, 127, value.getIntValue());
        DBG("  lovel: " + juce::String(region.lovel));
    }
    else if (key == "hivel")
    {
        region.hivel = juce::jlimit(0, 127, value.getIntValue());
        DBG("  hivel: " + juce::String(region.hivel));
    }
    else if (key == "pitch_keycenter")
    {
        region.pitch_keycenter = parseNoteValue(value);
        DBG("  pitch_keycenter: " + juce::String(region.pitch_keycenter));
    }
    else if (key == "volume")
    {
        region.volume = juce::jlimit(-144.0, 6.0, value.getDoubleValue());
    }
    else if (key == "tune")
    {
        region.tune = juce::jlimit(-100, 100, value.getIntValue());
    }
    else if (key == "transpose")
    {
        region.transpose = juce::jlimit(-127, 127, value.getIntValue());
    }
    else if (key == "ampeg_attack")
    {
        region.ampeg_attack = juce::jmax(0.0, value.getDoubleValue());
    }
    else if (key == "ampeg_release")
    {
        region.ampeg_release = juce::jmax(0.0, value.getDoubleValue());
    }
    // Add other opcodes as needed...
}

int EnhancedSFZLoader::parseNoteValue(const juce::String& value)
{
    // Handle note names like C4, A0, etc.
    if (value.containsAnyOf("abcdefgABCDEFG"))
    {
        // Simple note name parsing - you might want to make this more robust
        juce::String note = value.toLowerCase();
        if (note.startsWith("c")) return 60 + (value.getTrailingIntValue() - 4) * 12;
        if (note.startsWith("a")) return 57 + (value.getTrailingIntValue() - 4) * 12;
        // Add more note parsing as needed...
    }

    // Default to integer parsing
    return juce::jlimit(0, 127, value.getIntValue());
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

void EnhancedSFZLoader::applyInheritance()
{
    DBG("=== APPLYING INHERITANCE ===");

    for (int i = 0; i < regions.size(); ++i)
    {
        auto& region = regions.getReference(i);

        // Apply master opcodes
        if (region.masterIndex >= 0 && region.masterIndex < masters.size())
        {
            const auto& master = masters.getReference(region.masterIndex);
            for (const auto& opcode : master.opcodes)
            {
                if (!hasOpcode(region, opcode.key))
                {
                    applyOpcodeToRegionDirect(region, opcode.key, opcode.value);
                }
            }
        }

        // Apply group opcodes
        if (region.groupIndex >= 0 && region.groupIndex < groups.size())
        {
            const auto& group = groups.getReference(region.groupIndex);
            for (const auto& opcode : group.opcodes)
            {
                if (!hasOpcode(region, opcode.key))
                {
                    applyOpcodeToRegionDirect(region, opcode.key, opcode.value);
                }
            }
        }
    }
}

bool EnhancedSFZLoader::hasOpcode(const SFZRegion& region, const juce::String& key)
{
    for (const auto& opcode : region.opcodes)
    {
        if (opcode.key == key)
            return true;
    }
    return false;
}

void EnhancedSFZLoader::applyOpcodeToRegionDirect(SFZRegion& region, const juce::String& key, const juce::String& value)
{
    // Apply opcode without adding to opcodes list (to avoid duplicates)
    if (key == "sample" && region.sample.isEmpty())
        region.sample = value;
    else if (key == "lokey")
        region.lokey = parseNoteValue(value);
    else if (key == "hikey")
        region.hikey = parseNoteValue(value);
    else if (key == "lovel")
        region.lovel = juce::jlimit(0, 127, value.getIntValue());
    else if (key == "hivel")
        region.hivel = juce::jlimit(0, 127, value.getIntValue());
    else if (key == "pitch_keycenter")
        region.pitch_keycenter = parseNoteValue(value);
    // Add other opcodes as needed...
}

juce::Array<SampleSound::Ptr> EnhancedSFZLoader::createSampleSounds()
{
    DBG("=== CREATING SAMPLE SOUNDS ===");
    juce::Array<SampleSound::Ptr> sounds;

    for (int i = 0; i < regions.size(); ++i)
    {
        const auto& region = regions.getReference(i);

        if (region.sample.isNotEmpty())
        {
            DBG("Processing region " + juce::String(i) + ": " + region.sample);
            auto sound = createSampleSound(region);
            if (sound != nullptr)
            {
                sounds.add(sound);
                DBG("  SUCCESS: Created sound");
            }
            else
            {
                DBG("  FAILED: Could not create sound");
            }
        }
        else
        {
            DBG("Region " + juce::String(i) + ": No sample defined");
        }
    }

    return sounds;
}

SampleSound::Ptr EnhancedSFZLoader::createSampleSound(const SFZRegion& region)
{
    // Resolve sample file path using default_path
    juce::File sampleFile;

    if (defaultPath.isNotEmpty())
    {
        sampleFile = currentSFZFile.getParentDirectory().getChildFile(defaultPath + region.sample);
        DBG("  Trying with default_path: " + sampleFile.getFullPathName());
    }

    if (!sampleFile.existsAsFile())
    {
        sampleFile = currentSFZFile.getParentDirectory().getChildFile(region.sample);
        DBG("  Trying direct path: " + sampleFile.getFullPathName());
    }

    if (!sampleFile.existsAsFile())
    {
        sampleFile = currentSFZFile.getParentDirectory().getChildFile("Samples").getChildFile(region.sample);
        DBG("  Trying Samples folder: " + sampleFile.getFullPathName());
    }

    if (!sampleFile.existsAsFile())
    {
        DBG("  ERROR: Sample file not found: " + region.sample);
        return nullptr;
    }

    DBG("  Found sample: " + sampleFile.getFullPathName());

    auto audioBuffer = loadAudioFile(sampleFile);
    if (audioBuffer == nullptr)
    {
        DBG("  ERROR: Failed to load audio file");
        return nullptr;
    }

    DBG("  Audio loaded: " + juce::String(audioBuffer->getNumChannels()) + " channels, " +
        juce::String(audioBuffer->getNumSamples()) + " samples");

    // Create MIDI note range
    juce::BigInteger midiNotes;
    for (int note = region.lokey; note <= region.hikey; ++note)
        midiNotes.setBit(note);

    // Create velocity range
    juce::Range<int> velocityRange(region.lovel, region.hivel);

    DBG("  Creating sound: keys " + juce::String(region.lokey) + "-" + juce::String(region.hikey) +
        ", vel " + juce::String(region.lovel) + "-" + juce::String(region.hivel) +
        ", root " + juce::String(region.pitch_keycenter));

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
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(audioFile));

    if (reader == nullptr)
    {
        DBG("  ERROR: Cannot create audio reader for " + audioFile.getFileName());
        return nullptr;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(
        (int)reader->numChannels,
        (int)reader->lengthInSamples
    );

    reader->read(buffer.get(), 0, (int)reader->lengthInSamples, 0, true, true);

    return buffer;
}