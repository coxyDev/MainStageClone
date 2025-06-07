/*
  ==============================================================================

    ProPianoInterface.cpp
    Created: Professional piano interface for live performance
    Author:  Joel.Cox

  ==============================================================================
*/

#include "ProPianoInterface.h"

ProPianoInterface::ProPianoInterface(SamplerEngine& engine, juce::MidiKeyboardState& keyboardState)
    : samplerEngine(engine), midiKeyboardState(keyboardState)
{
    // Header section
    instrumentNameLabel.setText("Grand Piano", juce::dontSendNotification);
    instrumentNameLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    instrumentNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe6e6e6));
    instrumentNameLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(instrumentNameLabel);

    libraryInfoLabel.setText("No library loaded", juce::dontSendNotification);
    libraryInfoLabel.setFont(juce::Font(14.0f));
    libraryInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaaaaaa));
    libraryInfoLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(libraryInfoLabel);

    presetComboBox.addItem("Default", 1);
    presetComboBox.addItem("Bright", 2);
    presetComboBox.addItem("Warm", 3);
    presetComboBox.addItem("Intimate", 4);
    presetComboBox.setSelectedId(1);
    presetComboBox.addListener(this);
    addAndMakeVisible(presetComboBox);

    // Setup all control groups
    setupGroupComponent(toneGroup, "TONE");
    setupSlider(attackSlider, attackLabel, "Attack", 0.0, 1.0, 0.1);
    setupSlider(releaseSlider, releaseLabel, "Release", 0.0, 5.0, 1.0);
    setupSlider(velocitySlider, velocityLabel, "Velocity", 0.0, 1.0, 0.7);
    setupSlider(tuningSlider, tuningLabel, "Tuning", -50.0, 50.0, 0.0);

    setupGroupComponent(characterGroup, "CHARACTER");
    setupSlider(lidPositionSlider, lidPositionLabel, "Lid Position", 0.0, 1.0, 0.7);
    setupSlider(stringResonanceSlider, stringResonanceLabel, "String Res", 0.0, 1.0, 0.3);
    setupSlider(pedalNoiseSlider, pedalNoiseLabel, "Pedal Noise", 0.0, 1.0, 0.2);
    setupSlider(keyNoiseSlider, keyNoiseLabel, "Key Noise", 0.0, 1.0, 0.1);

    setupGroupComponent(microphoneGroup, "MICROPHONES");
    setupSlider(closePositionSlider, closePositionLabel, "Close Pos", 0.0, 1.0, 0.6);
    setupSlider(roomPositionSlider, roomPositionLabel, "Room Pos", 0.0, 1.0, 0.4);
    setupSlider(micBlendSlider, micBlendLabel, "Mic Blend", 0.0, 1.0, 0.5);
    setupSlider(stereoWidthSlider, stereoWidthLabel, "Stereo Width", 0.0, 1.0, 0.8);

    setupGroupComponent(effectsGroup, "EFFECTS");
    setupSlider(reverbAmountSlider, reverbAmountLabel, "Reverb", 0.0, 1.0, 0.25);
    setupSlider(reverbSizeSlider, reverbSizeLabel, "Size", 0.0, 1.0, 0.6);
    setupSlider(chorusAmountSlider, chorusAmountLabel, "Chorus", 0.0, 1.0, 0.0);

    reverbTypeCombo.addItem("Hall", 1);
    reverbTypeCombo.addItem("Room", 2);
    reverbTypeCombo.addItem("Chamber", 3);
    reverbTypeCombo.addItem("Plate", 4);
    reverbTypeCombo.setSelectedId(1);
    reverbTypeCombo.addListener(this);
    addAndMakeVisible(reverbTypeCombo);

    reverbTypeLabel.setText("Type", juce::dontSendNotification);
    reverbTypeLabel.setFont(juce::Font(12.0f));
    reverbTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    reverbTypeLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(reverbTypeLabel);

    setupGroupComponent(eqGroup, "EQ");
    setupSlider(lowGainSlider, lowGainLabel, "Low", -12.0, 12.0, 0.0);
    setupSlider(midGainSlider, midGainLabel, "Mid", -12.0, 12.0, 0.0);
    setupSlider(highGainSlider, highGainLabel, "High", -12.0, 12.0, 0.0);
    setupSlider(presenceSlider, presenceLabel, "Presence", -12.0, 12.0, 0.0);

    setupGroupComponent(masterGroup, "MASTER");
    setupSlider(volumeSlider, volumeLabel, "Volume", 0.0, 1.0, 0.8);
    setupSlider(panSlider, panLabel, "Pan", -1.0, 1.0, 0.0);

    sustainPedalButton.setButtonText("Sustain");
    sustainPedalButton.setClickingTogglesState(true);
    sustainPedalButton.addListener(this);
    addAndMakeVisible(sustainPedalButton);

    softPedalButton.setButtonText("Soft");
    softPedalButton.setClickingTogglesState(true);
    softPedalButton.addListener(this);
    addAndMakeVisible(softPedalButton);
}

ProPianoInterface::~ProPianoInterface()
{
}

void ProPianoInterface::paint(juce::Graphics& g)
{
    // Professional gradient background
    auto bounds = getLocalBounds();

    // Main background gradient
    juce::ColourGradient mainGradient(
        juce::Colour(0xff2a2a2a), 0.0f, 0.0f,
        juce::Colour(0xff1a1a1a), 0.0f, (float)getHeight(),
        false
    );
    g.setGradientFill(mainGradient);
    g.fillAll();

    // Header section
    headerArea = bounds.removeFromTop(80);
    juce::ColourGradient headerGradient(
        juce::Colour(0xff3a3a3a), 0.0f, (float)headerArea.getY(),
        juce::Colour(0xff2a2a2a), 0.0f, (float)headerArea.getBottom(),
        false
    );
    g.setGradientFill(headerGradient);
    g.fillRect(headerArea);

    // Header border
    g.setColour(juce::Colour(0xff4a4a4a));
    g.drawHorizontalLine(headerArea.getBottom(), 0.0f, (float)getWidth());

    // Subtle highlight at top
    g.setColour(juce::Colour(0xff505050));
    g.drawHorizontalLine(0, 0.0f, (float)getWidth());

    // Draw section backgrounds
    controlsArea = bounds;

    // Calculate section positions (we'll do this properly in resized())
    int sectionWidth = getWidth() / 3;
    int sectionHeight = (getHeight() - 80) / 2;

    // Draw subtle section separators
    g.setColour(juce::Colour(0xff353535));
    g.drawVerticalLine(sectionWidth, 80.0f, (float)getHeight());
    g.drawVerticalLine(sectionWidth * 2, 80.0f, (float)getHeight());
    g.drawHorizontalLine(80 + sectionHeight, 0.0f, (float)getWidth());
}

void ProPianoInterface::resized()
{
    auto bounds = getLocalBounds();

    // Header section
    headerArea = bounds.removeFromTop(80);
    auto headerContent = headerArea.reduced(20, 15);

    instrumentNameLabel.setBounds(headerContent.removeFromLeft(300));
    presetComboBox.setBounds(headerContent.removeFromRight(150).removeFromTop(25));
    libraryInfoLabel.setBounds(headerContent.removeFromLeft(300).removeFromBottom(20));

    // Control sections - 3x2 grid
    int sectionWidth = bounds.getWidth() / 3;
    int sectionHeight = bounds.getHeight() / 2;

    // Top row
    auto topRow = bounds.removeFromTop(sectionHeight);
    auto toneSection = topRow.removeFromLeft(sectionWidth).reduced(10);
    auto characterSection = topRow.removeFromLeft(sectionWidth).reduced(10);
    auto microphoneSection = topRow.reduced(10);

    // Bottom row  
    auto effectsSection = bounds.removeFromLeft(sectionWidth).reduced(10);
    auto eqSection = bounds.removeFromLeft(sectionWidth).reduced(10);
    auto masterSection = bounds.reduced(10);

    // Layout Tone section
    toneGroup.setBounds(toneSection);
    auto toneContent = toneSection.reduced(15, 25);
    int controlHeight = 80;
    int controlWidth = (toneContent.getWidth() - 10) / 2;

    auto toneLeft = toneContent.removeFromLeft(controlWidth);
    auto toneRight = toneContent.removeFromRight(controlWidth);

    attackLabel.setBounds(toneLeft.removeFromTop(15));
    attackSlider.setBounds(toneLeft.removeFromTop(controlHeight - 25));
    velocityLabel.setBounds(toneLeft.removeFromTop(15));
    velocitySlider.setBounds(toneLeft);

    releaseLabel.setBounds(toneRight.removeFromTop(15));
    releaseSlider.setBounds(toneRight.removeFromTop(controlHeight - 25));
    tuningLabel.setBounds(toneRight.removeFromTop(15));
    tuningSlider.setBounds(toneRight);

    // Layout Character section
    characterGroup.setBounds(characterSection);
    auto characterContent = characterSection.reduced(15, 25);
    auto charLeft = characterContent.removeFromLeft(controlWidth);
    auto charRight = characterContent.removeFromRight(controlWidth);

    lidPositionLabel.setBounds(charLeft.removeFromTop(15));
    lidPositionSlider.setBounds(charLeft.removeFromTop(controlHeight - 25));
    pedalNoiseLabel.setBounds(charLeft.removeFromTop(15));
    pedalNoiseSlider.setBounds(charLeft);

    stringResonanceLabel.setBounds(charRight.removeFromTop(15));
    stringResonanceSlider.setBounds(charRight.removeFromTop(controlHeight - 25));
    keyNoiseLabel.setBounds(charRight.removeFromTop(15));
    keyNoiseSlider.setBounds(charRight);

    // Layout Microphone section
    microphoneGroup.setBounds(microphoneSection);
    auto micContent = microphoneSection.reduced(15, 25);
    auto micLeft = micContent.removeFromLeft(controlWidth);
    auto micRight = micContent.removeFromRight(controlWidth);

    closePositionLabel.setBounds(micLeft.removeFromTop(15));
    closePositionSlider.setBounds(micLeft.removeFromTop(controlHeight - 25));
    micBlendLabel.setBounds(micLeft.removeFromTop(15));
    micBlendSlider.setBounds(micLeft);

    roomPositionLabel.setBounds(micRight.removeFromTop(15));
    roomPositionSlider.setBounds(micRight.removeFromTop(controlHeight - 25));
    stereoWidthLabel.setBounds(micRight.removeFromTop(15));
    stereoWidthSlider.setBounds(micRight);

    // Layout Effects section
    effectsGroup.setBounds(effectsSection);
    auto effectsContent = effectsSection.reduced(15, 25);
    auto effectsLeft = effectsContent.removeFromLeft(controlWidth);
    auto effectsRight = effectsContent.removeFromRight(controlWidth);

    reverbAmountLabel.setBounds(effectsLeft.removeFromTop(15));
    reverbAmountSlider.setBounds(effectsLeft.removeFromTop(controlHeight - 25));
    chorusAmountLabel.setBounds(effectsLeft.removeFromTop(15));
    chorusAmountSlider.setBounds(effectsLeft);

    reverbSizeLabel.setBounds(effectsRight.removeFromTop(15));
    reverbSizeSlider.setBounds(effectsRight.removeFromTop(controlHeight - 25));
    reverbTypeLabel.setBounds(effectsRight.removeFromTop(15));
    reverbTypeCombo.setBounds(effectsRight.removeFromTop(25));

    // Layout EQ section
    eqGroup.setBounds(eqSection);
    auto eqContent = eqSection.reduced(15, 25);
    auto eqLeft = eqContent.removeFromLeft(controlWidth);
    auto eqRight = eqContent.removeFromRight(controlWidth);

    lowGainLabel.setBounds(eqLeft.removeFromTop(15));
    lowGainSlider.setBounds(eqLeft.removeFromTop(controlHeight - 25));
    highGainLabel.setBounds(eqLeft.removeFromTop(15));
    highGainSlider.setBounds(eqLeft);

    midGainLabel.setBounds(eqRight.removeFromTop(15));
    midGainSlider.setBounds(eqRight.removeFromTop(controlHeight - 25));
    presenceLabel.setBounds(eqRight.removeFromTop(15));
    presenceSlider.setBounds(eqRight);

    // Layout Master section
    masterGroup.setBounds(masterSection);
    auto masterContent = masterSection.reduced(15, 25);
    auto masterLeft = masterContent.removeFromLeft(controlWidth);
    auto masterRight = masterContent.removeFromRight(controlWidth);

    volumeLabel.setBounds(masterLeft.removeFromTop(15));
    volumeSlider.setBounds(masterLeft.removeFromTop(controlHeight - 25));
    sustainPedalButton.setBounds(masterLeft.removeFromTop(30));

    panLabel.setBounds(masterRight.removeFromTop(15));
    panSlider.setBounds(masterRight.removeFromTop(controlHeight - 25));
    softPedalButton.setBounds(masterRight.removeFromTop(30));
}

void ProPianoInterface::sliderValueChanged(juce::Slider* slider)
{
    // Handle parameter changes
    if (slider == &volumeSlider)
    {
        // Update master volume
    }
    else if (slider == &attackSlider)
    {
        // Update attack time
    }
    else if (slider == &releaseSlider)
    {
        // Update release time
    }
    // Add more parameter handling as needed
}

void ProPianoInterface::buttonClicked(juce::Button* button)
{
    if (button == &sustainPedalButton)
    {
        // Handle sustain pedal
    }
    else if (button == &softPedalButton)
    {
        // Handle soft pedal
    }
}

void ProPianoInterface::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &presetComboBox)
    {
        // Load preset
    }
    else if (comboBoxThatHasChanged == &reverbTypeCombo)
    {
        // Change reverb type
    }
}

void ProPianoInterface::setCurrentLibrary(const juce::String& libraryName)
{
    currentLibraryName = libraryName;
    libraryInfoLabel.setText(libraryName, juce::dontSendNotification);
}

void ProPianoInterface::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText,
    double min, double max, double defaultValue)
{
    slider.setRange(min, max);
    slider.setValue(defaultValue);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff6699cc));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff404040));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffcccccc));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202020));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff404040));
    slider.addListener(this);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::Font(12.0f));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    label.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(label);
}

void ProPianoInterface::setupGroupComponent(juce::GroupComponent& group, const juce::String& title)
{
    group.setText(title);
    group.setColour(juce::GroupComponent::outlineColourId, juce::Colour(0xff404040));
    group.setColour(juce::GroupComponent::textColourId, juce::Colour(0xffaaaaaa));
    addAndMakeVisible(group);
}