#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "ParametricEQ.h"
#include "AnalyzerFifo.h"

/** AUR PRISM — parametric EQ (HP · low-shelf · bell · high-shelf · LP). */
class PrismProcessor : public juce::AudioProcessor
{
public:
    PrismProcessor();
    ~PrismProcessor() override = default;

    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    aur::AnalyzerFifo<4096>& getAnalyzer() { return analyzer; }
    double getSampleRateHz() const { return sr; }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* p[9] { nullptr };
    std::atomic<float>* pBypass = nullptr;

    aur::ParametricEQ eq;
    aur::AnalyzerFifo<4096> analyzer;
    double sr = 48000.0;

    juce::SmoothedValue<float> bypassMix { 0.0f };
    juce::AudioBuffer<float>   dryBuffer;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrismProcessor)
};
