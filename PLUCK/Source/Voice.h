#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "KarplusString.h"
#include "Parameters.h"

struct SynthParams
{
    void bind (juce::AudioProcessorValueTreeState& s)
    {
        auto g = [&s](const char* id){ return s.getRawParameterValue (id); };
        damp=g(ParamID::damp); sustain=g(ParamID::sustain); bright=g(ParamID::bright); spread=g(ParamID::spread);
    }
    std::atomic<float> *damp{},*sustain{},*bright{},*spread{};
};

struct PluckSound : juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/** Twin Karplus-Strong strings per voice for natural stereo width. */
class PluckVoice : public juce::SynthesiserVoice
{
public:
    explicit PluckVoice (const SynthParams& p) : sp (p) {}

    void prepareVoice (double sr) { setCurrentPlaybackSampleRate (sr); sL.prepare (sr); sR.prepare (sr); }
    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<PluckSound*> (s) != nullptr; }

    void startNote (int note, float vel, juce::SynthesiserSound*, int) override
    {
        const double f = 440.0 * std::pow (2.0, (note - 69) / 12.0);
        const float bright = *sp.bright;
        const float sprd = *sp.spread * 8.0f;   // cents
        sL.setParams (*sp.damp, *sp.sustain);
        sR.setParams (*sp.damp, *sp.sustain);
        sL.pluck (f * std::pow (2.0, -sprd / 1200.0), bright, vel);
        sR.pluck (f * std::pow (2.0,  sprd / 1200.0), bright, vel);
    }
    void stopNote (float, bool tail) override
    {
        if (tail) { sL.release(); sR.release(); }
        else { clearCurrentNote(); }
    }
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int n) override
    {
        if (! sL.isActive() && ! sR.isActive()) return;
        sL.setParams (*sp.damp, *sp.sustain);
        sR.setParams (*sp.damp, *sp.sustain);
        for (int s = 0; s < n; ++s)
        {
            const float l = sL.process();
            const float r = sR.process();
            out.addSample (0, start + s, l);
            if (out.getNumChannels() > 1) out.addSample (1, start + s, r);
        }
        if (! sL.isActive() && ! sR.isActive()) clearCurrentNote();
    }

private:
    const SynthParams& sp;
    aur::syn::KarplusString sL, sR;
};
