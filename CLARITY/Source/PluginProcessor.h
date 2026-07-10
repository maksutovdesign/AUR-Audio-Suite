#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "ResonanceSuppressor.h"
#include "Metering.h"

/**
    AUR CLARITY — perceptual dynamic resonance suppressor. Ducks protruding
    spectral resonances (harshness/mud) relative to the local spectral trend,
    without touching flat/broadband content. Built on the shared AurvedaDSP
    engine and AurvedaUI design system.
*/
class ClarityProcessor : public juce::AudioProcessor
{
public:
    ClarityProcessor();
    ~ClarityProcessor() override = default;

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
    aur::MeterState& getMeterState() { return meters; }
    aur::ResonanceSuppressor& getSuppressor() { return suppressor; }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pDepth  = nullptr;
    std::atomic<float>* pSens   = nullptr;
    std::atomic<float>* pSharp  = nullptr;
    std::atomic<float>* pMix    = nullptr;
    std::atomic<float>* pDelta  = nullptr;
    std::atomic<float>* pBypass = nullptr;

    aur::ResonanceSuppressor suppressor;
    aur::MeterState meters;

    juce::SmoothedValue<float> bypassMix { 0.0f };
    juce::AudioBuffer<float>   dryBuffer;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClarityProcessor)
};
