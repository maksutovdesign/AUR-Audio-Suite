#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Parameters.h"
#include "SpectralDenoiser.h"
#include "AnalyzerFifo.h"
#include "Metering.h"

/** AUR DENOISE — spectral noise reduction with a learned noise profile. */
class DenoiseProcessor : public juce::AudioProcessor
{
public:
    DenoiseProcessor();
    ~DenoiseProcessor() override = default;

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
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    aur::AnalyzerFifo<4096>& getAnalyzer() { return analyzer; }
    void learn() { denoiser.startLearn (1.0f); }
    bool isLearning() const { return denoiser.isLearning(); }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pAmount = nullptr;
    std::atomic<float>* pSens   = nullptr;
    std::atomic<float>* pBypass = nullptr;

    aur::SpectralDenoiser denoiser;
    aur::AnalyzerFifo<4096> analyzer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DenoiseProcessor)
};
