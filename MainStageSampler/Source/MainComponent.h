#pragma once

#include <JuceHeader.h>
#include "SamplerEngine.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::Button::Listener,
    public juce::KeyListener,
    public juce::AudioIODeviceCallback
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // Audio callbacks - using the correct JUCE 7+ signature
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
    // Button handling
    void buttonClicked(juce::Button* button) override;

    // Keyboard handling
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

private:
    //==============================================================================
    // Audio components
    juce::AudioDeviceManager audioDeviceManager;
    SamplerEngine samplerEngine;
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    // UI Components
    juce::TextButton loadButton;
    juce::TextButton audioSettingsButton;
    juce::Label statusLabel;
    juce::Label audioStatusLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    // File chooser
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Current loaded file
    juce::File currentSFZFile;

    //==============================================================================
    void loadSFZFile(const juce::File& file);
    void updateStatusLabel(const juce::String& message);
    void initializeAudio();
    void showAudioSettings();
    void updateAudioStatus();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};