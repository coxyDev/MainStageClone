#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Set up the load button
    loadButton.setButtonText("Load SFZ File");
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);

    // Set up the status label
    statusLabel.setText("Ready to load SFZ file...", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    // Set up the volume slider
    volumeSlider.setRange(0.0, 1.0);
    volumeSlider.setValue(0.7);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent(&volumeSlider, true);
    addAndMakeVisible(volumeLabel);

    // Set up the library combo box
    libraryComboBox.addListener(this);
    addAndMakeVisible(libraryComboBox);

    libraryLabel.setText("Library", juce::dontSendNotification);
    libraryLabel.attachToComponent(&libraryComboBox, true);
    addAndMakeVisible(libraryLabel);

    // Populate the library list
    refreshLibraryList();

    // Set up the keyboard component
    addAndMakeVisible(keyboardComponent);

    // Enable keyboard focus for computer keyboard input
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    // Set the window size
    setSize(800, 600);

    // Set up audio
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        setAudioChannels(2, 2);
    }
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    samplerEngine.prepareToPlay(sampleRate, samplesPerBlockExpected);
    keyboardState.reset();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
        bufferToFill.numSamples, true);

    samplerEngine.renderNextBlock(*bufferToFill.buffer, incomingMidi,
        bufferToFill.startSample, bufferToFill.numSamples);

    // Apply volume control
    float volume = (float)volumeSlider.getValue();
    bufferToFill.buffer->applyGain(bufferToFill.startSample, bufferToFill.numSamples, volume);
}

void MainComponent::releaseResources()
{
    keyboardState.reset();
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
    auto topSection = bounds.removeFromTop(200);

    loadButton.setBounds(20, 100, 120, 30);
    statusLabel.setBounds(160, 100, 400, 30);

    libraryComboBox.setBounds(80, 140, 200, 25);
    volumeSlider.setBounds(80, 170, 200, 30);

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
            break; // Only load the first SFZ file
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
}

void MainComponent::loadSFZFile(const juce::File& file)
{
    if (file.exists() && file.hasFileExtension(".sfz"))
    {
        updateStatusLabel("Loading " + file.getFileName() + "...");

        samplerEngine.loadSampleSet(file);
        currentSFZFile = file;

        updateStatusLabel("Loaded: " + file.getFileNameWithoutExtension());
        repaint();
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
    if (comboBoxThatHasChanged == &libraryComboBox)
    {
        auto selectedId = libraryComboBox.getSelectedId();
        if (selectedId > 0)
        {
            auto sfzFiles = SampleManager::findAvailableSFZFiles();
            if (selectedId <= sfzFiles.size())
            {
                loadSFZFile(juce::File(sfzFiles[selectedId - 1]));
            }
        }
    }
}

void MainComponent::refreshLibraryList()
{
    libraryComboBox.clear();
    libraryComboBox.addItem("Select a library...", -1);

    auto sfzFiles = SampleManager::findAvailableSFZFiles();

    for (int i = 0; i < sfzFiles.size(); ++i)
    {
        auto file = juce::File(sfzFiles[i]);
        auto libraryName = SampleManager::getLibraryNameFromFile(file);
        libraryComboBox.addItem(libraryName, i + 1);
    }

    if (sfzFiles.isEmpty())
    {
        libraryComboBox.addItem("No libraries found", -2);
        updateStatusLabel("Place SFZ files in the Samples folder");
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    // Map computer keyboard to piano keys
    // Using a piano-like layout: AWSEDFTGYHUJK... (white keys on QWERTY, black keys on top row)

    int midiNote = -1;
    int baseNote = 60; // Middle C

    // White keys (lower row)
    if (key.getKeyCode() == 'A') midiNote = baseNote;      // C
    else if (key.getKeyCode() == 'S') midiNote = baseNote + 2;  // D
    else if (key.getKeyCode() == 'D') midiNote = baseNote + 4;  // E
    else if (key.getKeyCode() == 'F') midiNote = baseNote + 5;  // F
    else if (key.getKeyCode() == 'G') midiNote = baseNote + 7;  // G
    else if (key.getKeyCode() == 'H') midiNote = baseNote + 9;  // A
    else if (key.getKeyCode() == 'J') midiNote = baseNote + 11; // B
    else if (key.getKeyCode() == 'K') midiNote = baseNote + 12; // C (octave up)

    // Black keys (upper row)
    else if (key.getKeyCode() == 'W') midiNote = baseNote + 1;  // C#
    else if (key.getKeyCode() == 'E') midiNote = baseNote + 3;  // D#
    else if (key.getKeyCode() == 'T') midiNote = baseNote + 6;  // F#
    else if (key.getKeyCode() == 'Y') midiNote = baseNote + 8;  // G#
    else if (key.getKeyCode() == 'U') midiNote = baseNote + 10; // A#

    // Lower octave
    else if (key.getKeyCode() == 'Z') midiNote = baseNote - 12; // C (octave down)
    else if (key.getKeyCode() == 'X') midiNote = baseNote - 10; // D
    else if (key.getKeyCode() == 'C') midiNote = baseNote - 8;  // E
    else if (key.getKeyCode() == 'V') midiNote = baseNote - 7;  // F
    else if (key.getKeyCode() == 'B') midiNote = baseNote - 5;  // G
    else if (key.getKeyCode() == 'N') midiNote = baseNote - 3;  // A
    else if (key.getKeyCode() == 'M') midiNote = baseNote - 1;  // B

    if (midiNote >= 0 && midiNote <= 127)
    {
        keyboardState.noteOn(1, midiNote, 0.8f);
        keyboardComponent.grabKeyboardFocus(); // Update visual keyboard
        return true;
    }

    return false;
}

bool MainComponent::keyStateChanged(bool /*isKeyDown*/, juce::Component* /*originatingComponent*/)
{
    // Handle key releases - turn off all notes when any key is released
    // This is a simple approach; you could make it more sophisticated
    keyboardState.allNotesOff(1);
    return true;
}