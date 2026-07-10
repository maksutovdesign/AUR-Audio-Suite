#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

#include "Parameters.h"
#include "TruePeakLimiter.h"
#include "LoudnessMeter.h"
#include "Metering.h"

/** AUR CEIL — true-peak lookahead limiter with LUFS metering. */
class CeilProcessor : public juce::AudioProcessor
{
public:
    CeilProcessor();
    ~CeilProcessor() override = default;

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
    float getMomentaryLufs() const { return momLufs.load(); }
    float getShortTermLufs() const { return stLufs.load(); }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pGain    = nullptr;
    std::atomic<float>* pCeiling = nullptr;
    std::atomic<float>* pRelease = nullptr;
    std::atomic<float>* pBypass  = nullptr;

    aur::TruePeakLimiter limiter;
    aur::LoudnessMeter   loudness;
    aur::MeterState      meters;

    std::atomic<float> momLufs { -100.0f };
    std::atomic<float> stLufs  { -100.0f };

    juce::SmoothedValue<float> bypassMix { 0.0f };
    juce::AudioBuffer<float>   dryBuffer;

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CeilProcessor)
};
