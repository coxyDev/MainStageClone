/*
  ==============================================================================

    SampleVoice.cpp
    Created: 6 Jun 2025 12:10:02pm
    Author:  Joel.Cox

  ==============================================================================
*/

#include "SampleVoice.h"

SampleVoice::SampleVoice()
{
    // Set up default ADSR parameters
    adsrParams.attack = 0.1f;
    adsrParams.decay = 1.0f;
    adsrParams.sustain = 1.0f;
    adsrParams.release = 0.1f;
}

SampleVoice::~SampleVoice()
{
}

bool SampleVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<const SampleSound*> (sound) != nullptr;
}

void SampleVoice::startNote(int midiNoteNumber, float velocity,
    juce::SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<const SampleSound*> (s))
    {
        DBG("SampleVoice: Starting note " + juce::String(midiNoteNumber) +
            " with sound: " + sound->getName());

        pitchRatio = std::pow(2.0, (midiNoteNumber - sound->getRootMidiNote()) / 12.0);
        sourceSamplePosition = 0.0;

        lgain = velocity;
        rgain = velocity;

        // Update ADSR parameters from the sound
        adsrParams.attack = (float)sound->getAttackTime();
        adsrParams.release = (float)sound->getReleaseTime();
        adsr.setParameters(adsrParams);

        adsr.setSampleRate(getSampleRate());
        adsr.noteOn();

        DBG("SampleVoice: Note started successfully, pitch ratio: " + juce::String(pitchRatio));
    }
    else
    {
        DBG("SampleVoice: ERROR - Cannot cast sound to SampleSound!");
        jassertfalse; // this object can only play SampleSounds!
    }
}

void SampleVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void SampleVoice::pitchWheelMoved(int /*newValue*/)
{
    // Handle pitch bend here if needed
}

void SampleVoice::controllerMoved(int /*controllerNumber*/, int /*newValue*/)
{
    // Handle CC messages here if needed (e.g., sustain pedal)
}

void SampleVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (auto* playingSound = static_cast<SampleSound*> (getCurrentlyPlayingSound().get()))
    {
        auto& data = *playingSound->getAudioData();

        if (data.getNumSamples() == 0)
        {
            DBG("SampleVoice: WARNING - Audio data is empty!");
            clearCurrentNote();
            return;
        }

        const float* const inL = data.getReadPointer(0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;

        float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            auto pos = (int)sourceSamplePosition;
            auto alpha = (float)(sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // Bounds checking
            if (pos >= data.getNumSamples() - 1)
            {
                stopNote(0.0f, false);
                break;
            }

            // simple linear interpolation..
            float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

            auto envelopeValue = adsr.getNextSample();

            l *= lgain * envelopeValue;
            r *= rgain * envelopeValue;

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;

            if (sourceSamplePosition >= data.getNumSamples())
            {
                stopNote(0.0f, false);
                break;
            }
        }

        if (!adsr.isActive())
            clearCurrentNote();
    }
}