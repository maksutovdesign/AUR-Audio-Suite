#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

#include "LoudnessMeter.h"
#include "AnalyzerFifo.h"
#include "Metering.h"

/** AUR SCOPE — analysis-only metering: spectrum, LUFS, peaks, correlation.
    Passes audio through unchanged. */
class ScopeProcessor : public juce::AudioProcessor
{
public:
    ScopeProcessor();
    ~ScopeProcessor() override = default;

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

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Analyzer"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    aur::MeterState& getMeterState() { return meters; }
    aur::AnalyzerFifo<4096>& getAnalyzer() { return analyzer; }
    float getMomentaryLufs() const { return momLufs.load(); }
    float getShortTermLufs() const { return stLufs.load(); }
    float getCorrelation()   const { return corr.load(); }

private:
    aur::LoudnessMeter loudness;
    aur::AnalyzerFifo<4096> analyzer;
    aur::MeterState meters;

    std::atomic<float> momLufs { -100.0f };
    std::atomic<float> stLufs  { -100.0f };
    std::atomic<float> corr    { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeProcessor)
};
