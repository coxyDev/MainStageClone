/*
  ==============================================================================

    PianoInterface.h
    Created: MainStage-style piano interface
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"

//==============================================================================
/**
    MainStage-style piano interface with large keyboard and piano-specific controls
*/
class PianoInterface : public juce::Component,
    public juce::Slider::Listener,
    public juce::Button::Listener,
    public juce::Timer
{
public:
    //==============================================================================
    PianoInterface(SamplerEngine& engine, juce::MidiKeyboardState& keyboardState);
    ~PianoInterface() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    //==============================================================================
    void setCurrentLibrary(const juce::String& libraryName);

private:
    //==============================================================================
    SamplerEngine& samplerEngine;
    juce::MidiKeyboardState& midiKeyboardState;

    // Large piano keyboard
    std::unique_ptr<juce::MidiKeyboardComponent> largeKeyboard;

    // Piano controls
    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    juce::Slider lidPositionSlider;
    juce::Label lidPositionLabel;

    juce::Slider micPositionSlider;
    juce::Label micPositionLabel;

    juce::Slider stringResonanceSlider;
    juce::Label stringResonanceLabel;

    juce::Slider reverbAmountSlider;
    juce::Label reverbAmountLabel;

    juce::Slider reverbSizeSlider;
    juce::Label reverbSizeLabel;

    // Performance controls
    juce::ToggleButton sustainPedalButton;
    juce::ToggleButton softPedalButton;

    // Velocity curve
    juce::ComboBox velocityCurveCombo;
    juce::Label velocityCurveLabel;

    // Display
    juce::Label libraryNameLabel;
    juce::Label sustainIndicator;
    juce::Label noteDisplayLabel;

    // Current state
    juce::String currentLibraryName;
    int lastNotePressed = -1;
    bool sustainPedalDown = false;

    //==============================================================================
    void updateNoteDisplay();
    void updateSustainIndicator();
    juce::String getNoteNameFromMidiNumber(int midiNote);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoInterface)
};