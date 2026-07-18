#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

#include "Parameters.h"
#include "Metering.h"

/** AUR CONVO — convolution reverb.

    Two IR sources:
      • SYNTHETIC (default) — an exponentially-decaying, tone-shaped,
        stereo-decorrelated noise IR built from the Decay / Tone / Width knobs.
      • FILE — any WAV/AIFF impulse response the user loads.

    Pre-Delay is a real delay line on the wet path, so it works for both
    sources. The synthetic IR is rebuilt on the message thread (AsyncUpdater)
    whenever a shaping parameter changes, so the audio thread never allocates. */
class ConvoProcessor : public juce::AudioProcessor,
                       private juce::AsyncUpdater
{
public:
    ConvoProcessor();
    ~ConvoProcessor() override = default;

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

    // --- IR source control (called from the editor / message thread) ---
    void loadImpulseFile (const juce::File&);
    void useSyntheticIR();
    bool isUsingFile() const { return usingFile.load(); }
    juce::String getIRSourceName() const;

private:
    void handleAsyncUpdate() override;   // rebuilds the synthetic IR off the audio thread
    void rebuildSyntheticIR();

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* pChar     = nullptr;
    std::atomic<float>* pDecay    = nullptr;
    std::atomic<float>* pTone     = nullptr;
    std::atomic<float>* pPredelay = nullptr;
    std::atomic<float>* pWidth    = nullptr;
    std::atomic<float>* pMix      = nullptr;
    std::atomic<float>* pBypass   = nullptr;

    juce::dsp::Convolution conv { juce::dsp::Convolution::Latency { 0 } };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> predelay { 8192 };
    double currentSampleRate = 44100.0;
    int    numChannels       = 2;

    std::atomic<bool> usingFile { false };

    // Cached shaping values so processBlock only asks for a rebuild on change.
    float lastDecay = -1.f, lastTone = -1.f, lastWidth = -1.f, lastChar = -1.f;

    juce::AudioBuffer<float> wetBuffer, dryBuffer;
    juce::SmoothedValue<float> mixSmooth { 0.35f };
    aur::MeterState meters;

    int currentProgram = 0;

    static constexpr const char* kIRPathProp = "irFilePath";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvoProcessor)
};
