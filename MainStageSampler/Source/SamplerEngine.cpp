/*
  ==============================================================================

    SamplerEngine.cpp
    Created: 6 Jun 2025 12:11:00pm
    Author:  Joel.Cox

  ==============================================================================
*/

#include "SamplerEngine.h"
#include "SampleVoice.h"
#include "SFZLoader.h"

SamplerEngine::SamplerEngine()
{
    // Add voices to the synthesiser
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice(new SampleVoice());
}

SamplerEngine::~SamplerEngine()
{
}

void SamplerEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SamplerEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages,
    int startSample,
    int numSamples)
{
    synth.renderNextBlock(buffer, midiMessages, startSample, numSamples);
}

void SamplerEngine::loadSampleSet(const juce::File& sfzFile)
{
    // Clear existing sounds
    synth.clearSounds();

    // Load the SFZ file
    SFZLoader loader;
    auto sounds = loader.loadSFZ(sfzFile);

    // Add sounds to the synthesiser
    for (auto sound : sounds)
    {
        synth.addSound(sound);
    }

    juce::Logger::writeToLog("Loaded " + juce::String(sounds.size()) + " samples from " + sfzFile.getFileName());
}