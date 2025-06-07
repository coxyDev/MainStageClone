#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"
#include "SampleManager.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
    public juce::FileDragAndDropTarget,
    public juce::Button::Listener,
    public juce::ComboBox::Listener,
    public juce::KeyListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // File drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    //==============================================================================
    // Button handling
    void buttonClicked(juce::Button* button) override;

    // ComboBox handling
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    // Keyboard handling
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

private:
    //==============================================================================
    // Audio components
    SamplerEngine samplerEngine;
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    // UI Components
    juce::TextButton loadButton;
    juce::Label statusLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::ComboBox libraryComboBox;
    juce::Label libraryLabel;

    // File chooser
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Current loaded file
    juce::File currentSFZFile;

    //==============================================================================
    void loadSFZFile(const juce::File& file);
    void updateStatusLabel(const juce::String& message);
    void refreshLibraryList();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};