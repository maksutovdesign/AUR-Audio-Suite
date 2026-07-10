#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

#include "Parameters.h"
#include "StereoImager.h"
#include "Metering.h"

/** AUR IMAGER — stereo width, mono-maker, balance, correlation. */
class ImagerProcessor : public juce::AudioProcessor
{
public:
    ImagerProcessor();
    ~ImagerProcessor() override = default;

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

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pWidth  = nullptr;
    std::atomic<float>* pMono   = nullptr;
    std::atomic<float>* pBal    = nullptr;
    std::atomic<float>* pBypass = nullptr;

    aur::StereoImager imager;
    aur::MeterState   meters;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagerProcessor)
};
