#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

#include "Parameters.h"
#include "Biquad.h"
#include "Compressor.h"
#include "TruePeakLimiter.h"
#include "LoudnessMeter.h"
#include "AnalyzerFifo.h"

/**
    AUR ASSIST — Master Assistant. Analyses the incoming audio ("Assist") and
    auto-configures a mastering chain (tilt EQ → glue compressor → makeup →
    true-peak limiter) toward a target loudness and tonal balance. The learned
    amounts are applied through an Intensity blend the user can trim.
*/
class AssistProcessor : public juce::AudioProcessor
{
public:
    AssistProcessor();
    ~AssistProcessor() override = default;

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
    const juce::String getProgramName (int) override { return "Master"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    aur::AnalyzerFifo<4096>& getAnalyzer() { return analyzer; }

    // ---- Assistant control / readouts (UI thread) ----
    void startAnalysis() { analyzeRemaining.store (analyzeLen); analyzing.store (true); }
    bool  isAnalyzing()      const { return analyzing.load(); }
    float getComputedGainDb() const { return computedGainDb.load(); }
    float getComputedTiltDb() const { return computedTiltDb.load(); }
    float getOutputLufs()     const { return outLufs.load(); }

private:
    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pTarget = nullptr;
    std::atomic<float>* pTone   = nullptr;
    std::atomic<float>* pInt    = nullptr;
    std::atomic<float>* pCeil   = nullptr;
    std::atomic<float>* pBypass = nullptr;

    double fs = 48000.0;

    // Chain.
    std::array<aur::Biquad, 2> tiltLow, tiltHigh;   // per channel shelves
    aur::Compressor comp;
    aur::TruePeakLimiter limiter;
    juce::SmoothedValue<float> gainSm { 1.0f };

    // Analysis.
    aur::LoudnessMeter loudnessIn, loudnessOut;
    std::array<aur::Biquad, 1> lpDet, hpDet;        // mono detectors
    aur::AnalyzerFifo<4096> analyzer;

    std::atomic<bool>  analyzing { false };
    std::atomic<int>   analyzeRemaining { 0 };
    int analyzeLen = 0;
    double lpAcc = 0.0, hpAcc = 0.0; long accN = 0;

    std::atomic<float> computedGainDb { 0.0f };
    std::atomic<float> computedTiltDb { 0.0f };
    std::atomic<float> outLufs { -100.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AssistProcessor)
};
