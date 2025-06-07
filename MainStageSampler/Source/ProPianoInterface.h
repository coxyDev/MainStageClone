/*
  ==============================================================================

    ProPianoInterface.h
    Created: Professional piano interface for live performance
    Author:  Joel.Cox

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"

//==============================================================================
/**
    Professional piano interface with comprehensive controls for live performance
*/
class ProPianoInterface : public juce::Component,
    public juce::Slider::Listener,
    public juce::Button::Listener,
    public juce::ComboBox::Listener
{
public:
    //==============================================================================
    ProPianoInterface(SamplerEngine& engine, juce::MidiKeyboardState& keyboardState);
    ~ProPianoInterface() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    //==============================================================================
    void setCurrentLibrary(const juce::String& libraryName);

private:
    //==============================================================================
    SamplerEngine& samplerEngine;
    juce::MidiKeyboardState& midiKeyboardState;

    // Header section
    juce::Label instrumentNameLabel;
    juce::Label libraryInfoLabel;
    juce::ComboBox presetComboBox;

    // Tone Section
    juce::GroupComponent toneGroup;
    juce::Slider attackSlider;
    juce::Label attackLabel;
    juce::Slider releaseSlider;
    juce::Label releaseLabel;
    juce::Slider velocitySlider;
    juce::Label velocityLabel;
    juce::Slider tuningSlider;
    juce::Label tuningLabel;

    // Character Section
    juce::GroupComponent characterGroup;
    juce::Slider lidPositionSlider;
    juce::Label lidPositionLabel;
    juce::Slider stringResonanceSlider;
    juce::Label stringResonanceLabel;
    juce::Slider pedalNoiseSlider;
    juce::Label pedalNoiseLabel;
    juce::Slider keyNoiseSlider;
    juce::Label keyNoiseLabel;

    // Microphone Section
    juce::GroupComponent microphoneGroup;
    juce::Slider closePositionSlider;
    juce::Label closePositionLabel;
    juce::Slider roomPositionSlider;
    juce::Label roomPositionLabel;
    juce::Slider micBlendSlider;
    juce::Label micBlendLabel;
    juce::Slider stereoWidthSlider;
    juce::Label stereoWidthLabel;

    // Effects Section
    juce::GroupComponent effectsGroup;
    juce::Slider reverbAmountSlider;
    juce::Label reverbAmountLabel;
    juce::Slider reverbSizeSlider;
    juce::Label reverbSizeLabel;
    juce::Slider chorusAmountSlider;
    juce::Label chorusAmountLabel;
    juce::ComboBox reverbTypeCombo;
    juce::Label reverbTypeLabel;

    // EQ Section
    juce::GroupComponent eqGroup;
    juce::Slider lowGainSlider;
    juce::Label lowGainLabel;
    juce::Slider midGainSlider;
    juce::Label midGainLabel;
    juce::Slider highGainSlider;
    juce::Label highGainLabel;
    juce::Slider presenceSlider;
    juce::Label presenceLabel;

    // Master Section
    juce::GroupComponent masterGroup;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Slider panSlider;
    juce::Label panLabel;
    juce::ToggleButton sustainPedalButton;
    juce::ToggleButton softPedalButton;

    // Visual elements
    juce::Rectangle<int> headerArea;
    juce::Rectangle<int> controlsArea;

    // Current state
    juce::String currentLibraryName;

    //==============================================================================
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText,
        double min, double max, double defaultValue);
    void setupGroupComponent(juce::GroupComponent& group, const juce::String& title);
    void drawProStyleSlider(juce::Graphics& g, juce::Slider& slider);
    void drawSectionBackground(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProPianoInterface)
};