#include "SamplerEngine.h"
#include "SampleVoice.h"
#include "SampleSound.h"
#include "EnhancedSFZLoader.h"

SamplerEngine::SamplerEngine()
{
    // Add voices to the synthesiser
    for (int i = 0; i < numVoices; ++i)
        synth.addVoice(new SampleVoice());
}

SamplerEngine::~SamplerEngine()
{
}

void SamplerEngine::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void SamplerEngine::renderNextBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages,
    int startSample,
    int numSamples)
{
    // Debug MIDI messages (only when they occur)
    if (!midiMessages.isEmpty())
    {
        DBG("=== MIDI EVENT ===");
        DBG("Events: " + juce::String(midiMessages.getNumEvents()));
        DBG("Sounds loaded: " + juce::String(synth.getNumSounds()));

        for (const auto metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            if (message.isNoteOn())
            {
                int note = message.getNoteNumber();
                int velocity = message.getVelocity();

                DBG("Note ON: " + juce::String(note) + " velocity " + juce::String(velocity));

                // Check which sounds should respond
                bool foundResponder = false;
                for (int i = 0; i < synth.getNumSounds(); ++i)
                {
                    if (auto* sound = dynamic_cast<SampleSound*>(synth.getSound(i).get()))
                    {
                        bool appliesToNote = sound->appliesToNote(note);
                        bool appliesToVel = sound->appliesToVelocity(velocity);

                        if (appliesToNote && appliesToVel)
                        {
                            DBG("  -> Sound " + juce::String(i) + " (" + sound->getName() + ") should respond");
                            foundResponder = true;
                        }
                    }
                }

                if (!foundResponder)
                {
                    DBG("  -> *** NO SOUNDS RESPOND TO THIS NOTE! ***");

                    // Show what notes ARE covered
                    DBG("  -> Available note ranges:");
                    for (int i = 0; i < juce::jmin(5, synth.getNumSounds()); ++i)
                    {
                        if (auto* sound = dynamic_cast<SampleSound*>(synth.getSound(i).get()))
                        {
                            // Find the note range for this sound
                            int lowestNote = -1, highestNote = -1;
                            for (int n = 0; n <= 127; ++n)
                            {
                                if (sound->appliesToNote(n))
                                {
                                    if (lowestNote == -1) lowestNote = n;
                                    highestNote = n;
                                }
                            }

                            if (lowestNote != -1)
                            {
                                DBG("    Sound " + juce::String(i) + ": notes " +
                                    juce::String(lowestNote) + "-" + juce::String(highestNote) +
                                    " vel " + juce::String(sound->getVelocityRange().getStart()) +
                                    "-" + juce::String(sound->getVelocityRange().getEnd()));
                            }
                        }
                    }
                }
            }
        }
    }

    synth.renderNextBlock(buffer, midiMessages, startSample, numSamples);

    // Apply master volume
    buffer.applyGain(0, numSamples, masterVolume);
}

void SamplerEngine::loadSampleSet(const juce::File& sfzFile)
{
    DBG("=== SAMPLER ENGINE LOADING ===");
    DBG("Loading SFZ: " + sfzFile.getFileName());

    // Clear existing sounds
    synth.clearSounds();
    DBG("Cleared existing sounds");

    // Load the SFZ file with enhanced parser
    EnhancedSFZLoader loader;
    auto sounds = loader.loadSFZ(sfzFile);

    DBG("Loader returned " + juce::String(sounds.size()) + " sounds");

    // Add sounds to the synthesiser
    for (auto sound : sounds)
    {
        synth.addSound(sound);
    }

    DBG("Added sounds to synthesizer");
    debugLoadedSounds();

    juce::Logger::writeToLog("Enhanced SFZ Loader: Loaded " + juce::String(sounds.size()) + " samples from " + sfzFile.getFileName());
}

void SamplerEngine::debugLoadedSounds()
{
    DBG("=== SYNTHESIZER SOUNDS DEBUG ===");
    DBG("Total sounds: " + juce::String(synth.getNumSounds()));

    if (synth.getNumSounds() == 0)
    {
        DBG("*** NO SOUNDS LOADED - This is the problem! ***");
        return;
    }

    // Show first 5 sounds in detail
    for (int i = 0; i < juce::jmin(5, synth.getNumSounds()); ++i)
    {
        if (auto* sound = dynamic_cast<SampleSound*>(synth.getSound(i).get()))
        {
            DBG("Sound " + juce::String(i) + ":");
            DBG("  Name: " + sound->getName());
            DBG("  Root note: " + juce::String(sound->getRootMidiNote()));

            // Find actual note range
            int lowestNote = -1, highestNote = -1;
            for (int note = 0; note <= 127; ++note)
            {
                if (sound->appliesToNote(note))
                {
                    if (lowestNote == -1) lowestNote = note;
                    highestNote = note;
                }
            }

            if (lowestNote != -1)
            {
                DBG("  Note range: " + juce::String(lowestNote) + "-" + juce::String(highestNote));
            }
            else
            {
                DBG("  Note range: NONE! (This is a problem)");
            }

            auto velRange = sound->getVelocityRange();
            DBG("  Velocity range: " + juce::String(velRange.getStart()) + "-" + juce::String(velRange.getEnd()));

            // Test common notes
            DBG("  Responds to C4(60): " + juce::String(sound->appliesToNote(60) ? "YES" : "NO"));
            DBG("  Responds to A0(21): " + juce::String(sound->appliesToNote(21) ? "YES" : "NO"));
            DBG("  Responds to vel 100: " + juce::String(sound->appliesToVelocity(100) ? "YES" : "NO"));
        }
    }

    if (synth.getNumSounds() > 5)
    {
        DBG("... and " + juce::String(synth.getNumSounds() - 5) + " more sounds");
    }

    DBG("=== END SYNTHESIZER DEBUG ===");
}