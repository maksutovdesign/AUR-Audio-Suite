#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "Voice.h"
#include "ADAASaturator.h"
#include "Metering.h"

class OrganProcessor : public juce::AudioProcessor
{
public:
    OrganProcessor();
    ~OrganProcessor() override = default;

    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    aur::MeterState& getMeterState() { return meters; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

private:
    juce::AudioProcessorValueTreeState apvts;
    SynthParams params;
    juce::Synthesiser synth;
    juce::MidiKeyboardState keyboardState;
    aur::ADAASaturator drive;
    std::atomic<float>* pDrive = nullptr; std::atomic<float>* pVolume = nullptr;
    juce::SmoothedValue<float> volGain { 1.0f };
    aur::MeterState meters;
    int currentProgram = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrganProcessor)
};
