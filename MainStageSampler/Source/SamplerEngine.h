#pragma once

#include <JuceHeader.h>

class SamplerEngine {
public:
    SamplerEngine();
    ~SamplerEngine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void renderNextBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&, int startSample, int numSamples);

    void loadSampleSet(const juce::File& sfzFile);

    // Debug method
    void debugLoadedSounds();

private:
    juce::Synthesiser synth;
    int numVoices = 16;
    float masterVolume = 0.8f;
};