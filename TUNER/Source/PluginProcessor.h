#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PitchDetector.h"

/** AUR TUNER — monophonic tuner. Analysis-only pass-through. */
class TunerProcessor : public juce::AudioProcessor
{
public:
    TunerProcessor()
        : AudioProcessor (BusesProperties()
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)) {}
    ~TunerProcessor() override = default;

    void prepareToPlay (double sr, int) override { detector.prepare (sr); }
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override
    {
        const auto& o = l.getMainOutputChannelSet();
        if (o != juce::AudioChannelSet::mono() && o != juce::AudioChannelSet::stereo()) return false;
        return l.getMainInputChannelSet() == o;
    }
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals _nd;
        const int n = buffer.getNumSamples();
        const auto tout = getTotalNumOutputChannels();
        const auto* l = buffer.getReadPointer (0);
        const auto* r = tout > 1 ? buffer.getReadPointer (1) : l;
        for (int i = 0; i < n; ++i) detector.pushMono (0.5f * (l[i] + r[i]));
    }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; } bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; } double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; } int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {} const juce::String getProgramName (int) override { return "Tuner"; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {} void setStateInformation (const void*, int) override {}

    float getFrequency() const { return detector.getFrequency(); }

private:
    aur::PitchDetector detector;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunerProcessor)
};
