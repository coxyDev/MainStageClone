#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : interfaceTabs(juce::TabbedButtonBar::TabsAtTop)
{
    // Create the Pro piano interface
    pianoInterface = std::make_unique<ProPianoInterface>(samplerEngine, keyboardState);

    // Add it to a tabbed component (ready for future instruments)
    interfaceTabs.addTab("Piano", juce::Colour(0xff2a2a2a), pianoInterface.get(), false);
    addAndMakeVisible(interfaceTabs);

    // Utility controls (minimal top bar)
    loadButton.setButtonText("Load SFZ");
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);

    audioSettingsButton.setButtonText("Audio Settings");
    audioSettingsButton.addListener(this);
    addAndMakeVisible(audioSettingsButton);

    statusLabel.setText("Ready", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::left);
    statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    addAndMakeVisible(statusLabel);

    // Mode selector (Engine vs Performance view)
    modeComboBox.addItem("Engine Mode", 1);
    modeComboBox.addItem("Performance Mode", 2);
    modeComboBox.setSelectedId(1);
    modeComboBox.addListener(this);
    addAndMakeVisible(modeComboBox);

    modeLabel.setText("Mode:", juce::dontSendNotification);
    modeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcccccc));
    modeLabel.setJustificationType(juce::Justification::right);
    addAndMakeVisible(modeLabel);

    // Enable keyboard focus
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    // Set window size - larger for UVI interface
    setSize(1400, 800);

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
    audioDeviceManager.initialiseWithDefaultDevices(0, 2);
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
    }
}

void MainComponent::audioDeviceIOCallbackWithContext(const float* const* /*inputChannelData*/,
    int /*numInputChannels*/,
    float* const* outputChannelData,
    int numOutputChannels,
    int numSamples,
    const juce::AudioIODeviceCallbackContext& /*context*/)
{
    // Clear output buffers
    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (outputChannelData[channel] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[channel], numSamples);
    }

    // Create audio buffer
    juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);

    // Create MIDI buffer
    juce::MidiBuffer midiBuffer;
    keyboardState.processNextMidiBuffer(midiBuffer, 0, numSamples, true);

    // Process through sampler
    samplerEngine.renderNextBlock(buffer, midiBuffer, 0, numSamples);

    // Apply master volume
    buffer.applyGain(0, numSamples, masterVolume);
}

void MainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    auto sampleRate = device->getCurrentSampleRate();
    auto bufferSize = device->getCurrentBufferSizeSamples();

    samplerEngine.prepareToPlay(sampleRate, bufferSize);
    keyboardState.reset();
    updateAudioStatus();
}

void MainComponent::audioDeviceStopped()
{
    keyboardState.reset();
    updateAudioStatus();
}

void MainComponent::updateAudioStatus()
{
    auto* currentDevice = audioDeviceManager.getCurrentAudioDevice();

    if (currentDevice != nullptr)
    {
        juce::String statusText = "Audio: " + currentDevice->getName();
        statusLabel.setText(statusText, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff66cc66));
    }
    else
    {
        statusLabel.setText("No audio device", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcc6666));
    }
}

void MainComponent::showAudioSettings()
{
    auto audioSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioDeviceManager, 0, 2, 2, 2, false, false, false, false);

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
    // Dark background to match UVI interface
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Top utility bar
    auto utilityArea = getLocalBounds().removeFromTop(35);
    juce::ColourGradient utilityGradient(
        juce::Colour(0xff3a3a3a), 0.0f, 0.0f,
        juce::Colour(0xff2a2a2a), 0.0f, 35.0f,
        false
    );
    g.setGradientFill(utilityGradient);
    g.fillRect(utilityArea);

    // Utility bar border
    g.setColour(juce::Colour(0xff4a4a4a));
    g.drawHorizontalLine(35, 0.0f, (float)getWidth());
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Top utility bar (35px)
    auto utilityBar = bounds.removeFromTop(35);
    auto utilityContent = utilityBar.reduced(10, 5);

    loadButton.setBounds(utilityContent.removeFromLeft(100));
    utilityContent.removeFromLeft(10); // spacing
    audioSettingsButton.setBounds(utilityContent.removeFromLeft(120));
    utilityContent.removeFromLeft(20); // spacing
    statusLabel.setBounds(utilityContent.removeFromLeft(300));

    // Mode selector on the right
    modeComboBox.setBounds(utilityContent.removeFromRight(120));
    utilityContent.removeFromRight(10); // spacing
    modeLabel.setBounds(utilityContent.removeFromRight(50));

    // Tabbed interface takes the rest
    interfaceTabs.setBounds(bounds);
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
    int baseNote = 60;

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
                        auto libraryName = file.getParentDirectory().getFileName();
                        pianoInterface->setCurrentLibrary(libraryName);
                        updateStatusLabel("Loaded: " + libraryName);
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

void MainComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modeComboBox)
    {
        auto selectedId = modeComboBox.getSelectedId();
        if (selectedId == 1)
        {
            switchToEngineMode();
        }
        else if (selectedId == 2)
        {
            switchToPerformanceMode();
        }
    }
}

void MainComponent::switchToPerformanceMode()
{
    // Future: Switch to MainStage-style performance view
    updateStatusLabel("Performance mode coming soon...");
}

void MainComponent::switchToEngineMode()
{
    // Already in engine mode with UVI interface
    updateStatusLabel("Engine mode active");
}