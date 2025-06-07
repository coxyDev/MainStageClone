#pragma once

#include <JuceHeader.h>

class SamplerEngine {
public:
    SamplerEngine();
    ~SamplerEngine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void renderNextBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&, int startSample, int numSamples);

    void loadSampleSet(const juce::File& sfzFile);

private:
    juce::Synthesiser synth;
    int numVoices = 16;
};