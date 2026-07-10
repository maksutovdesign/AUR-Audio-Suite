#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "DynamicEQ.h"
#include "AnalyzerFifo.h"

/** AUR MOTION — 3-band dynamic EQ. */
class MotionProcessor : public juce::AudioProcessor
{
public:
    MotionProcessor();
    ~MotionProcessor() override = default;

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
    aur::DynamicEQ& getEQ() { return eq; }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pf[3] { nullptr };
    std::atomic<float>* pt[3] { nullptr };
    std::atomic<float>* pr[3] { nullptr };
    std::atomic<float>* pQ = nullptr;
    std::atomic<float>* pBypass = nullptr;

    aur::DynamicEQ eq;
    aur::AnalyzerFifo<4096> analyzer;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MotionProcessor)
};
