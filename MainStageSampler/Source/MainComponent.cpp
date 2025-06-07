#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Set up the load button
    loadButton.setButtonText("Load SFZ File");
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);

    // Set up the audio settings button
    audioSettingsButton.setButtonText("Audio Settings");
    audioSettingsButton.addListener(this);
    addAndMakeVisible(audioSettingsButton);

    // Set up the status label
    statusLabel.setText("Ready to load SFZ file...", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // Set up the audio status label
    audioStatusLabel.setText("Initializing audio...", juce::dontSendNotification);
    audioStatusLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(audioStatusLabel);

    // Set up the volume slider
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(0.7);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, true);
    addAndMakeVisible(volumeLabel);

    // Set up the keyboard component
    addAndMakeVisible(keyboardComponent);

    // Enable keyboard focus for computer keyboard input
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    // Set the window size
    setSize(1000, 600);

    // Initialize audio
    initializeAudio();
    updateAudioStatus();
}

MainComponent::~MainComponent()
{
    audioDeviceManager.removeAudioCallback(this);
    audioDeviceManager.closeAudioDevice();
}

//==============================================================================
void MainComponent::initializeAudio()
{
    // Initialize the audio device manager with multiple driver types
    audioDeviceManager.initialiseWithDefaultDevices(0, 2);

    // Add this component as an audio callback
    audioDeviceManager.addAudioCallback(this);

    DBG("Audio device manager initialized");

    auto& deviceTypes = audioDeviceManager.getAvailableDeviceTypes();
    for (auto* type : deviceTypes)
    {
        DBG("Available audio device type: " + type->getTypeName());
    }

    if (auto* currentDevice = audioDeviceManager.getCurrentAudioDevice())
    {
        DBG("Current audio device: " + currentDevice->getName());
        DBG("Sample rate: " + juce::String(currentDevice->getCurrentSampleRate()));
        DBG("Buffer size: " + juce::String(currentDevice->getCurrentBufferSizeSamples()));
    }
    else
    {
        DBG("No audio device found!");
    }
}

void MainComponent::audioDeviceIOCallbackWithContext(const float* const* /*inputChannelData*/,
    int /*numInputChannels*/,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& /*context*/)
{
    // Clear output buffers first
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (outputChannelData[channel] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[channel], numSamples);
    }

    // Create audio buffer from output data
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);

    // Create MIDI buffer from keyboard state
    juce::MidiBuffer midiBuffer;
    keyboardState.processNextMidiBuffer(midiBuffer, 0, numSamples, true);

    // Debug MIDI messages
    if (!midiBuffer.isEmpty())
    {
        DBG("MIDI buffer has " + juce::String(midiBuffer.getNumEvents()) + " events");
        for (const auto metadata : midiBuffer)
        {
            auto message = metadata.getMessage();
            if (message.isNoteOn())
                DBG("MIDI Note ON: " + juce::String(message.getNoteNumber()) + " velocity: " + juce::String(message.getVelocity()));
            else if (message.isNoteOff())
                DBG("MIDI Note OFF: " + juce::String(message.getNoteNumber()));
        }
    }

    // Process audio through sampler engine
    samplerEngine.renderNextBlock(buffer, midiBuffer, 0, numSamples);

    // Check if we're producing any audio
    bool hasAudio = false;
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (outputChannelData[channel] != nullptr)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                if (std::abs(outputChannelData[channel][sample]) > 0.001f)
                {
                    hasAudio = true;
                    break;
                }
            }
        }
        if (hasAudio) break;
    }

    if (hasAudio)
        DBG("Audio detected in buffer!");

    // Apply volume control
    float volume = (float)volumeSlider.getValue();
    buffer.applyGain(0, numSamples, volume);
}

void MainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    DBG("Audio device about to start: " + device->getName());

    auto sampleRate = device->getCurrentSampleRate();
    auto bufferSize = device->getCurrentBufferSizeSamples();

    DBG("Sample rate: " + juce::String(sampleRate));
    DBG("Buffer size: " + juce::String(bufferSize));

    // Prepare the sampler engine
    samplerEngine.prepareToPlay(sampleRate, bufferSize);
    keyboardState.reset();

    updateAudioStatus();
}

void MainComponent::audioDeviceStopped()
{
    DBG("Audio device stopped");
    keyboardState.reset();
    updateAudioStatus();
}

void MainComponent::updateAudioStatus()
{
    auto* currentDevice = audioDeviceManager.getCurrentAudioDevice();

    if (currentDevice != nullptr)
    {
        juce::String statusText = "Audio: " + currentDevice->getName() +
            " (" + currentDevice->getTypeName() + ") - " +
            juce::String(currentDevice->getCurrentSampleRate(), 0) + "Hz, " +
            juce::String(currentDevice->getCurrentBufferSizeSamples()) + " samples";

        audioStatusLabel.setText(statusText, juce::dontSendNotification);
        audioStatusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    }
    else
    {
        audioStatusLabel.setText("No audio device", juce::dontSendNotification);
        audioStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}

void MainComponent::showAudioSettings()
{
    auto audioSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioDeviceManager,
        0, 2,  // input channels
        2, 2,  // output channels
        false, false,  // show MIDI
        false, false   // stereo pairs, advanced options
    );

    audioSelector->setSize(500, 400);

    juce::CallOutBox::launchAsynchronously(
        std::move(audioSelector),
        audioSettingsButton.getScreenBounds(),
        nullptr
    );
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2e2e2e));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("MainStage Sampler", 20, 20, 400, 40, juce::Justification::left);

    if (currentSFZFile.exists())
    {
        g.setFont(16.0f);
        g.drawText("Loaded: " + currentSFZFile.getFileNameWithoutExtension(),
            20, 60, 600, 30, juce::Justification::left);
    }
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Top section for controls
    auto topSection = bounds.removeFromTop(250);

    // Button row
    loadButton.setBounds(20, 100, 120, 30);
    audioSettingsButton.setBounds(160, 100, 120, 30);

    // Status labels
    statusLabel.setBounds(20, 140, 600, 30);
    audioStatusLabel.setBounds(20, 170, 600, 30);

    // Volume slider
    volumeSlider.setBounds(80, 200, 200, 30);

    // Bottom section for keyboard
    keyboardComponent.setBounds(bounds.removeFromBottom(100));
}

//==============================================================================
bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& filename : files)
    {
        if (filename.endsWithIgnoreCase(".sfz"))
            return true;
    }
    return false;
}

void MainComponent::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    for (const auto& filename : files)
    {
        if (filename.endsWithIgnoreCase(".sfz"))
        {
            loadSFZFile(juce::File(filename));
            break;
        }
    }
}

//==============================================================================
void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        auto chooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles;

        fileChooser = std::make_unique<juce::FileChooser>("Select an SFZ file to load...",
            juce::File(),
            "*.sfz");

        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.exists())
                {
                    loadSFZFile(file);
                }
            });
    }
    else if (button == &audioSettingsButton)
    {
        showAudioSettings();
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    int midiNote = -1;
    int baseNote = 60; // Middle C

    // White keys
    if (key.getKeyCode() == 'A') midiNote = baseNote;
    else if (key.getKeyCode() == 'S') midiNote = baseNote + 2;
    else if (key.getKeyCode() == 'D') midiNote = baseNote + 4;
    else if (key.getKeyCode() == 'F') midiNote = baseNote + 5;
    else if (key.getKeyCode() == 'G') midiNote = baseNote + 7;
    else if (key.getKeyCode() == 'H') midiNote = baseNote + 9;
    else if (key.getKeyCode() == 'J') midiNote = baseNote + 11;
    else if (key.getKeyCode() == 'K') midiNote = baseNote + 12;

    // Black keys
    else if (key.getKeyCode() == 'W') midiNote = baseNote + 1;
    else if (key.getKeyCode() == 'E') midiNote = baseNote + 3;
    else if (key.getKeyCode() == 'T') midiNote = baseNote + 6;
    else if (key.getKeyCode() == 'Y') midiNote = baseNote + 8;
    else if (key.getKeyCode() == 'U') midiNote = baseNote + 10;

    if (midiNote >= 0 && midiNote <= 127)
    {
        DBG("Triggering MIDI note: " + juce::String(midiNote));
        keyboardState.noteOn(1, midiNote, 0.8f);
        return true;
    }

    return false;
}

bool MainComponent::keyStateChanged(bool /*isKeyDown*/, juce::Component* /*originatingComponent*/)
{
    keyboardState.allNotesOff(1);
    return true;
}

void MainComponent::loadSFZFile(const juce::File& file)
{
    if (file.exists() && file.hasFileExtension(".sfz"))
    {
        updateStatusLabel("Loading " + file.getFileName() + "...");

        juce::Thread::launch([this, file]()
            {
                samplerEngine.loadSampleSet(file);

                juce::MessageManager::callAsync([this, file]()
                    {
                        currentSFZFile = file;
                        updateStatusLabel("Loaded: " + file.getFileNameWithoutExtension());
                        repaint();
                    });
            });
    }
    else
    {
        updateStatusLabel("Invalid SFZ file!");
    }
}

void MainComponent::updateStatusLabel(const juce::String& message)
{
    statusLabel.setText(message, juce::dontSendNotification);
}