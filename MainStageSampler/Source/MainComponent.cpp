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

    // Set up the keyboard component
    addAndMakeVisible(keyboardComponent);

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

    volumeSlider.setBounds(80, 140, 200, 30);

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
        juce::FileChooser chooser("Select an SFZ file to load...",
            juce::File(),
            "*.sfz");

        if (chooser.browseForFileToOpen())
        {
            loadSFZFile(chooser.getResult());
        }
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