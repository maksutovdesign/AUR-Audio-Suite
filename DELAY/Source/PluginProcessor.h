#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "Delay.h"
#include "Metering.h"

/** AUR DELAY — stereo delay with damped feedback, ping-pong, width. */
class DelayProcessor : public juce::AudioProcessor
{
public:
    DelayProcessor();
    ~DelayProcessor() override = default;

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
    double getTailLengthSeconds() const override { return 4.0; }

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

    std::atomic<float>* pTime = nullptr;
    std::atomic<float>* pFb   = nullptr;
    std::atomic<float>* pDamp = nullptr;
    std::atomic<float>* pWidth = nullptr;
    std::atomic<float>* pPing = nullptr;
    std::atomic<float>* pMix  = nullptr;
    std::atomic<float>* pBypass = nullptr;

    aur::Delay delay;
    aur::MeterState meters;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayProcessor)
};
