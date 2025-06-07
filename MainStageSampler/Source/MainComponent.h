#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"
#include "ProPianoInterface.h"

//==============================================================================
/*
    Main application component with UVI-style piano interface
*/
class MainComponent : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::Button::Listener,
    public juce::KeyListener,
    public juce::AudioIODeviceCallback,
    public juce::ComboBox::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Audio callbacks
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    //==============================================================================
    // File drag and drop
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    //==============================================================================
    // UI callbacks
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    // Keyboard handling
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

private:
    //==============================================================================
    // Audio components
    juce::AudioDeviceManager audioDeviceManager;
    SamplerEngine samplerEngine;
    juce::MidiKeyboardState keyboardState;

    // UI Components
    std::unique_ptr<ProPianoInterface> pianoInterface;
    juce::TabbedComponent interfaceTabs;

    // Utility controls (minimal top bar)
    juce::TextButton loadButton;
    juce::TextButton audioSettingsButton;
    juce::Label statusLabel;
    juce::ComboBox modeComboBox;
    juce::Label modeLabel;

    // File chooser
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Current state
    juce::File currentSFZFile;
    float masterVolume = 0.8f;

    //==============================================================================
    void loadSFZFile(const juce::File& file);
    void updateStatusLabel(const juce::String& message);
    void initializeAudio();
    void showAudioSettings();
    void updateAudioStatus();
    void switchToPerformanceMode();
    void switchToEngineMode();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};