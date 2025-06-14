/*
  ==============================================================================

    EnhancedSFZLoader.h
    Created: Professional SFZ parser for advanced sample libraries
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SampleSound.h"

//==============================================================================
/**
    Enhanced SFZ parser that handles professional sample libraries with
    variables, includes, hierarchies, and complex opcodes
*/
class EnhancedSFZLoader
{
public:
    //==============================================================================
    EnhancedSFZLoader();
    ~EnhancedSFZLoader();

    //==============================================================================
    /** Loads an SFZ file with full support for advanced features */
    juce::Array<SampleSound::Ptr> loadSFZ(const juce::File& sfzFile);

private:
    //==============================================================================
    struct SFZVariable
    {
        juce::String name;
        juce::String value;
    };

    struct SFZOpcode
    {
        juce::String key;
        juce::String value;

        SFZOpcode() = default;
        SFZOpcode(const juce::String& k, const juce::String& v) : key(k), value(v) {}
    };

    struct SFZRegion
    {
        // Hierarchy tracking
        int masterIndex = -1;
        int groupIndex = -1;

        // Basic sample info
        juce::String sample;
        int lokey = 0;
        int hikey = 127;
        int lovel = 0;
        int hivel = 127;
        int pitch_keycenter = 60;
        int key = -1;

        // ADSR
        double ampeg_attack = 0.0;
        double ampeg_decay = 0.0;
        double ampeg_sustain = 1.0;
        double ampeg_release = 0.1;

        // Filters
        double cutoff = 20000.0;
        double resonance = 0.0;
        int fil_type = 0; // 0=lpf, 1=hpf, 2=bpf

        // Volume and pan
        double volume = 0.0; // dB
        double pan = 0.0;
        double amplitude = 100.0;

        // Pitch
        int transpose = 0;
        int tune = 0;

        // Triggers and conditions
        juce::String trigger = "attack"; // attack, release, first, legato
        int seq_length = 1;
        int seq_position = 1;

        // Round robin
        double lorand = 0.0;
        double hirand = 1.0;

        // Controllers
        int locc1 = 0, hicc1 = 127;
        int locc64 = 0, hicc64 = 127; // sustain pedal

        // Switches
        int sw_lokey = -1, sw_hikey = -1;
        int sw_last = -1;
        juce::String sw_label;

        // Group and exclusivity
        int group = 0;
        int off_by = 0;

        // Timing
        double offset = 0.0;
        double delay = 0.0;

        // All opcodes for advanced processing
        juce::Array<SFZOpcode> opcodes;
    };

    struct SFZGroup
    {
        int masterIndex = -1;
        juce::Array<SFZOpcode> opcodes;
    };

    struct SFZMaster
    {
        juce::Array<SFZOpcode> opcodes;
    };

    //==============================================================================
    // Parsing state
    juce::Array<SFZVariable> variables;
    juce::Array<SFZMaster> masters;
    juce::Array<SFZGroup> groups;
    juce::Array<SFZRegion> regions;

    juce::File currentSFZFile;
    juce::AudioFormatManager formatManager;
    juce::String defaultPath; // Store default_path from <control> section

    //==============================================================================
    /** Process the main SFZ file and all includes */
    void parseFile(const juce::File& file);

    /** Parse a single line of SFZ content */
    void parseLine(const juce::String& line);

    /** Handle #define variables */
    void handleDefine(const juce::String& line);

    /** Handle #include statements */
    void handleInclude(const juce::String& line);

    /** Handle section headers like <master>, <group>, <region> */
    void handleSectionHeader(const juce::String& line);

    /** Parse key=value opcodes */
    void handleOpcode(const juce::String& key, const juce::String& value);

    /** Apply opcode to region */
    void applyOpcodeToRegion(const juce::String& key, const juce::String& value);

    /** Apply opcode directly to region (used during inheritance) */
    void applyOpcodeToRegionDirect(SFZRegion& region, const juce::String& key, const juce::String& value);

    /** Parse note values (handles note names like C4, A0) */
    int parseNoteValue(const juce::String& value);

    /** Substitute variables in a string */
    juce::String substituteVariables(const juce::String& input);

    /** Apply inheritance: master -> group -> region */
    void applyInheritance();

    /** Check if region already has an opcode */
    bool hasOpcode(const SFZRegion& region, const juce::String& key);

    /** Convert parsed regions to SampleSound objects */
    juce::Array<SampleSound::Ptr> createSampleSounds();

    /** Create a single SampleSound from a region */
    SampleSound::Ptr createSampleSound(const SFZRegion& region);

    /** Load audio file with proper error handling */
    std::unique_ptr<juce::AudioBuffer<float>> loadAudioFile(const juce::File& audioFile);

    /** Current parsing context */
    enum class ParseContext
    {
        Global,
        Master,
        Group,
        Region
    } currentContext = ParseContext::Global;

    int currentMasterIndex = -1;
    int currentGroupIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnhancedSFZLoader)
};