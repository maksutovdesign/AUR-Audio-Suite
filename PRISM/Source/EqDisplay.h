#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SpectrumAnalyzer.h"
#include "ParametricEQ.h"
#include "Parameters.h"
#include "Theme.h"

/**
    PRISM's hero: a live spectrum (reused SpectrumAnalyzer) with the EQ response
    curve drawn on top. The curve is computed UI-side from a private ParametricEQ
    reading the APVTS, so it never races the audio thread.
*/
class EqDisplay : public juce::Component,
                  private juce::Timer
{
public:
    EqDisplay (aur::AnalyzerFifo<4096>& fifo, juce::AudioProcessorValueTreeState& s, double sampleRate)
        : spectrum (fifo), apvts (s)
    {
        eq.prepare (sampleRate, 1);
        addAndMakeVisible (spectrum);
        startTimerHz (30);
    }
    ~EqDisplay() override { stopTimer(); }

    void resized() override { spectrum.setBounds (getLocalBounds()); }

    void paintOverChildren (juce::Graphics& g) override
    {
        const auto& t = aur::ui::theme();
        auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
        eq.setParameters (get (ParamID::hpFreq), get (ParamID::lsFreq), get (ParamID::lsGain),
                          get (ParamID::bellFreq), get (ParamID::bellGain), get (ParamID::bellQ),
                          get (ParamID::hsFreq), get (ParamID::hsGain), get (ParamID::lpFreq));

        auto r = getLocalBounds().toFloat().reduced (8.0f);
        const float logMin = std::log10 (20.0f), logMax = std::log10 (20000.0f);
        const float dbRange = 18.0f; // ±18 dB shown

        juce::Path curve;
        const int steps = 220;
        for (int i = 0; i <= steps; ++i)
        {
            const float fx = (float) i / (float) steps;
            const float freq = std::pow (10.0f, logMin + fx * (logMax - logMin));
            const float db = eq.responseDb (freq);
            const float x = r.getX() + fx * r.getWidth();
            const float y = r.getCentreY() - (db / dbRange) * (r.getHeight() * 0.5f);
            if (i == 0) curve.startNewSubPath (x, y);
            else        curve.lineTo (x, y);
        }

        // 0 dB line.
        g.setColour (t.line);
        g.drawHorizontalLine ((int) r.getCentreY(), r.getX(), r.getRight());

        g.setColour (t.accentBright);
        g.strokePath (curve, juce::PathStrokeType (2.0f));
    }

private:
    void timerCallback() override { repaint(); }

    aur::ui::SpectrumAnalyzer spectrum;
    juce::AudioProcessorValueTreeState& apvts;
    aur::ParametricEQ eq;
};
