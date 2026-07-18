#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

#include "Parameters.h"
#include "DrumSynth.h"
#include "ADAASaturator.h"
#include "Metering.h"

/** AUR BEAT — analog drum machine (synthesised, no samples). */
class BeatProcessor : public juce::AudioProcessor
{
public:
    BeatProcessor();
    ~BeatProcessor() override = default;

    void prepareToPlay (double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    aur::MeterState& getMeterState() { return meters; }

    /** Trigger a drum from the UI pads (lock-free). */
    void triggerPad (int voice) { if (juce::isPositiveAndBelow (voice, Drum::Count)) uiTrigger[(size_t) voice].store (100); }

private:
    void triggerVoice (int voice, float velocity);
    float renderVoiceSample (int voice);

    juce::AudioProcessorValueTreeState apvts;

    aur::syn::KickVoice  kick;
    aur::syn::SnareVoice snare;
    aur::syn::ClapVoice  clap;
    aur::syn::HatVoice   chat, ohat;
    aur::syn::TomVoice   tom;
    aur::syn::RimVoice   rim;

    std::array<std::atomic<int>, Drum::Count> uiTrigger { };

    aur::ADAASaturator drive;
    std::atomic<float>* pDrive  = nullptr;
    std::atomic<float>* pVolume = nullptr;
    std::atomic<float>* pLevel[Drum::Count] { };
    std::atomic<float>* pStep[Drum::Count][16] { };
    std::atomic<float>* pSeqOn = nullptr;
    int lastStep = -1;
    std::atomic<float>* pTune [Drum::Count] { };
    std::atomic<float>* pDecay[Drum::Count] { };

    juce::SmoothedValue<float> volGain { 1.0f };
    aur::MeterState meters;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatProcessor)
};
