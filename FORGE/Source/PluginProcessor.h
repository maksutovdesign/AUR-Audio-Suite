#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

#include "Parameters.h"
#include "GainStage.h"
#include "Biquad.h"
#include "ADAASaturator.h"
#include "Compressor.h"
#include "ToneTilt.h"
#include "Metering.h"
#include "AnalyzerFifo.h"

/**
    AUR FORGE — the flagship channel strip. One CHARACTER macro drives the whole
    chain from clean to molten: Input → HPF → ADAA Saturation → Compressor →
    Tone → Output → safety clip. Built entirely on the shared AurvedaDSP engine.
*/
class ForgeProcessor : public juce::AudioProcessor
{
public:
    ForgeProcessor();
    ~ForgeProcessor() override = default;

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
    aur::AnalyzerFifo<4096>& getAnalyzer() { return analyzer; }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pChar   = nullptr;
    std::atomic<float>* pFlavor = nullptr;
    std::atomic<float>* pInput  = nullptr;
    std::atomic<float>* pHpf    = nullptr;
    std::atomic<float>* pTone   = nullptr;
    std::atomic<float>* pOutput = nullptr;
    std::atomic<float>* pBypass = nullptr;

    double fs = 48000.0;

    aur::GainStage     inGain, outGain;
    std::array<aur::Biquad, 2> hpf;
    aur::ADAASaturator sat;
    aur::Compressor    comp;
    aur::ToneTilt      tone;
    aur::MeterState    meters;
    aur::AnalyzerFifo<4096> analyzer;

    juce::AudioBuffer<float> dryBuffer;
    juce::SmoothedValue<float> bypassMix { 0.0f };

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForgeProcessor)
};
