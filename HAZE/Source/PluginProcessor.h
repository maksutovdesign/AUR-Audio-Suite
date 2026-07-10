#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "Reverb.h"
#include "Metering.h"

/** AUR HAZE — warm FDN reverb. */
class HazeProcessor : public juce::AudioProcessor
{
public:
    HazeProcessor();
    ~HazeProcessor() override = default;

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
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    aur::MeterState& getMeterState() { return meters; }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pSize  = nullptr;
    std::atomic<float>* pDecay = nullptr;
    std::atomic<float>* pDamp  = nullptr;
    std::atomic<float>* pPre   = nullptr;
    std::atomic<float>* pWidth = nullptr;
    std::atomic<float>* pMix   = nullptr;
    std::atomic<float>* pBypass = nullptr;

    aur::Reverb reverb;
    aur::MeterState meters;

    juce::AudioBuffer<float> wet;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HazeProcessor)
};
