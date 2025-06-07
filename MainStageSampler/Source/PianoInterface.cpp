/*
  ==============================================================================

    PianoInterface.cpp
    Created: MainStage-style piano interface
    Author:  Joel.Cox

  ==============================================================================
*/

#include "PianoInterface.h"

PianoInterface::PianoInterface(SamplerEngine& engine, juce::MidiKeyboardState& keyboardState)
    : samplerEngine(engine), midiKeyboardState(keyboardState)
{
    // Create large piano keyboard
    largeKeyboard = std::make_unique<juce::MidiKeyboardComponent>(
        midiKeyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
    largeKeyboard->setKeyWidth(20.0f);  // Larger keys
    largeKeyboard->setLowestVisibleKey(36);  // C2
    largeKeyboard->setAvailableRange(21, 108);  // Full 88-key range
    addAndMakeVisible(*largeKeyboard);

    // Volume control
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(0.8);
    volumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel);

    // Lid Position (tonal control)
    lidPositionSlider.setRange(0.0, 1.0);
    lidPositionSlider.setValue(0.7);
    lidPositionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lidPositionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    lidPositionSlider.addListener(this);
    addAndMakeVisible(lidPositionSlider);

    lidPositionLabel.setText("Lid Position", juce::dontSendNotification);
    lidPositionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lidPositionLabel);

    // Mic Position (close/ambient)
    micPositionSlider.setRange(0.0, 1.0);
    micPositionSlider.setValue(0.5);
    micPositionSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    micPositionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    micPositionSlider.addListener(this);
    addAndMakeVisible(micPositionSlider);

    micPositionLabel.setText("Mic Position", juce::dontSendNotification);
    micPositionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(micPositionLabel);

    // String Resonance
    stringResonanceSlider.setRange(0.0, 1.0);
    stringResonanceSlider.setValue(0.3);
    stringResonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    stringResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    stringResonanceSlider.addListener(this);
    addAndMakeVisible(stringResonanceSlider);

    stringResonanceLabel.setText("String Res", juce::dontSendNotification);
    stringResonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stringResonanceLabel);

    // Reverb Amount
    reverbAmountSlider.setRange(0.0, 1.0);
    reverbAmountSlider.setValue(0.25);
    reverbAmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverbAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    reverbAmountSlider.addListener(this);
    addAndMakeVisible(reverbAmountSlider);

    reverbAmountLabel.setText("Reverb", juce::dontSendNotification);
    reverbAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbAmountLabel);

    // Reverb Size
    reverbSizeSlider.setRange(0.0, 1.0);
    reverbSizeSlider.setValue(0.6);
    reverbSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverbSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    reverbSizeSlider.addListener(this);
    addAndMakeVisible(reverbSizeSlider);

    reverbSizeLabel.setText("Hall Size", juce::dontSendNotification);
    reverbSizeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(reverbSizeLabel);

    // Sustain pedal
    sustainPedalButton.setButtonText("Sustain Pedal");
    sustainPedalButton.setClickingTogglesState(true);
    sustainPedalButton.addListener(this);
    addAndMakeVisible(sustainPedalButton);

    // Soft pedal
    softPedalButton.setButtonText("Soft Pedal");
    softPedalButton.setClickingTogglesState(true);
    softPedalButton.addListener(this);
    addAndMakeVisible(softPedalButton);

    // Velocity curve
    velocityCurveCombo.addItem("Linear", 1);
    velocityCurveCombo.addItem("Soft", 2);
    velocityCurveCombo.addItem("Hard", 3);
    velocityCurveCombo.addItem("Extra Soft", 4);
    velocityCurveCombo.addItem("Extra Hard", 5);
    velocityCurveCombo.setSelectedId(1);
    addAndMakeVisible(velocityCurveCombo);

    velocityCurveLabel.setText("Velocity Curve", juce::dontSendNotification);
    velocityCurveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(velocityCurveLabel);

    // Display labels
    libraryNameLabel.setText("No Library Loaded", juce::dontSendNotification);
    libraryNameLabel.setJustificationType(juce::Justification::centred);
    libraryNameLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    addAndMakeVisible(libraryNameLabel);

    sustainIndicator.setText("SUSTAIN", juce::dontSendNotification);
    sustainIndicator.setJustificationType(juce::Justification::centred);
    sustainIndicator.setFont(juce::Font(16.0f, juce::Font::bold));
    sustainIndicator.setColour(juce::Label::textColourId, juce::Colours::red);
    sustainIndicator.setVisible(false);
    addAndMakeVisible(sustainIndicator);

    noteDisplayLabel.setText("", juce::dontSendNotification);
    noteDisplayLabel.setJustificationType(juce::Justification::centred);
    noteDisplayLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    addAndMakeVisible(noteDisplayLabel);

    // Start timer for updates
    startTimer(50);  // 20fps for smooth updates
}

PianoInterface::~PianoInterface()
{
    stopTimer();
}

void PianoInterface::paint(juce::Graphics& g)
{
    // MainStage-inspired dark background
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Header section
    auto headerArea = getLocalBounds().removeFromTop(80);
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(headerArea);

    // Piano controls background
    auto controlsArea = getLocalBounds().removeFromTop(200);
    g.setColour(juce::Colour(0xff252525));
    g.fillRect(controlsArea);

    // Add some subtle borders
    g.setColour(juce::Colour(0xff404040));
    g.drawHorizontalLine(80, 0.0f, (float)getWidth());
    g.drawHorizontalLine(280, 0.0f, (float)getWidth());
}

void PianoInterface::resized()
{
    auto bounds = getLocalBounds();

    // Header section (80px)
    auto headerSection = bounds.removeFromTop(80);
    libraryNameLabel.setBounds(headerSection.reduced(20, 15));

    // Controls section (200px)
    auto controlsSection = bounds.removeFromTop(200);

    // Split controls into rows
    auto topControlsRow = controlsSection.removeFromTop(100);
    auto bottomControlsRow = controlsSection.removeFromTop(100);

    // Top row - main piano controls
    int controlWidth = 80;
    int controlSpacing = 100;
    int startX = 50;

    auto volumeArea = topControlsRow.removeFromLeft(controlSpacing);
    volumeLabel.setBounds(startX, 10, controlWidth, 20);
    volumeSlider.setBounds(startX, 30, controlWidth, controlWidth);

    auto lidArea = topControlsRow.removeFromLeft(controlSpacing);
    lidPositionLabel.setBounds(startX + controlSpacing, 10, controlWidth, 20);
    lidPositionSlider.setBounds(startX + controlSpacing, 30, controlWidth, controlWidth);

    auto micArea = topControlsRow.removeFromLeft(controlSpacing);
    micPositionLabel.setBounds(startX + controlSpacing * 2, 10, controlWidth, 20);
    micPositionSlider.setBounds(startX + controlSpacing * 2, 30, controlWidth, controlWidth);

    auto stringArea = topControlsRow.removeFromLeft(controlSpacing);
    stringResonanceLabel.setBounds(startX + controlSpacing * 3, 10, controlWidth, 20);
    stringResonanceSlider.setBounds(startX + controlSpacing * 3, 30, controlWidth, controlWidth);

    auto reverbArea = topControlsRow.removeFromLeft(controlSpacing);
    reverbAmountLabel.setBounds(startX + controlSpacing * 4, 10, controlWidth, 20);
    reverbAmountSlider.setBounds(startX + controlSpacing * 4, 30, controlWidth, controlWidth);

    auto reverbSizeArea = topControlsRow.removeFromLeft(controlSpacing);
    reverbSizeLabel.setBounds(startX + controlSpacing * 5, 10, controlWidth, 20);
    reverbSizeSlider.setBounds(startX + controlSpacing * 5, 30, controlWidth, controlWidth);

    // Bottom row - performance controls
    sustainPedalButton.setBounds(50, 110, 100, 30);
    softPedalButton.setBounds(170, 110, 100, 30);

    velocityCurveLabel.setBounds(300, 110, 100, 20);
    velocityCurveCombo.setBounds(300, 135, 100, 25);

    // Right side - indicators
    sustainIndicator.setBounds(getWidth() - 120, 120, 100, 30);
    noteDisplayLabel.setBounds(getWidth() - 120, 150, 100, 30);

    // Large keyboard takes remaining space
    auto keyboardArea = bounds;
    keyboardArea = keyboardArea.reduced(20, 10);
    largeKeyboard->setBounds(keyboardArea);
}

void PianoInterface::sliderValueChanged(juce::Slider* slider)
{
    // These would eventually control the sampler engine parameters
    if (slider == &volumeSlider)
    {
        // Control master volume
    }
    else if (slider == &lidPositionSlider)
    {
        // Control tonal brightness (EQ)
    }
    else if (slider == &micPositionSlider)
    {
        // Control close/ambient mix
    }
    else if (slider == &stringResonanceSlider)
    {
        // Control string resonance amount
    }
    else if (slider == &reverbAmountSlider)
    {
        // Control reverb mix
    }
    else if (slider == &reverbSizeSlider)
    {
        // Control reverb size/decay
    }
}

void PianoInterface::buttonClicked(juce::Button* button)
{
    if (button == &sustainPedalButton)
    {
        sustainPedalDown = sustainPedalButton.getToggleState();
        updateSustainIndicator();

        // Send CC64 message for sustain pedal
        // This would be implemented when we have MIDI CC support
    }
    else if (button == &softPedalButton)
    {
        // Send CC67 message for soft pedal
        // This would control velocity scaling or EQ
    }
}

void PianoInterface::timerCallback()
{
    updateNoteDisplay();
    updateSustainIndicator();
}

void PianoInterface::setCurrentLibrary(const juce::String& libraryName)
{
    currentLibraryName = libraryName;
    libraryNameLabel.setText(libraryName, juce::dontSendNotification);
}

void PianoInterface::updateNoteDisplay()
{
    // Check for currently pressed notes
    bool foundNote = false;
    for (int note = 0; note <= 127; ++note)
    {
        if (midiKeyboardState.isNoteOn(1, note))
        {
            if (lastNotePressed != note)
            {
                lastNotePressed = note;
                noteDisplayLabel.setText(getNoteNameFromMidiNumber(note), juce::dontSendNotification);
            }
            foundNote = true;
            break;
        }
    }

    if (!foundNote && lastNotePressed != -1)
    {
        lastNotePressed = -1;
        noteDisplayLabel.setText("", juce::dontSendNotification);
    }
}

void PianoInterface::updateSustainIndicator()
{
    sustainIndicator.setVisible(sustainPedalDown);
}

juce::String PianoInterface::getNoteNameFromMidiNumber(int midiNote)
{
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (midiNote / 12) - 1;
    int noteIndex = midiNote % 12;

    return juce::String(noteNames[noteIndex]) + juce::String(octave);
}