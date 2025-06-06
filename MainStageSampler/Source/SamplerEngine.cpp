#include "SamplerEngine.h"
#include "SampleVoice.h"
#include "SampleSound.h"
#include "SFZLoader.h"

SamplerEngine::SamplerEngine() {
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice(new SampleVoice());
}

SamplerEngine::~SamplerEngine() {}

void SamplerEngine::prepareToPlay(double sampleRate, int samplesPerBlock) {
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SamplerEngine::renderNextBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi, int startSample, int numSamples) {
    synth.renderNextBlock(buffer, midi, startSample, numSamples);
}

void SamplerEngine::loadSampleSet(const juce::File& sfzFile) {
    auto sounds = SFZLoader::parse(sfzFile); // We’ll build this next
    for (auto* sound : sounds)
        synth.addSound(sound);
}
