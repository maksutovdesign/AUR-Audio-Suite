#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "AnalyzerFifo.h"

namespace aur::ui
{
/**
    Reusable real-time FFT spectrum display, styled from the current Theme.
    Reads a mono AnalyzerFifo (fed by the audio thread), windows + transforms
    on a UI timer, and draws a smoothed log-frequency curve with a filled body.
    Shared across the suite (EMBER shows saturation harmonics; SCOPE will use it).
*/
class SpectrumAnalyzer : public juce::Component,
                         private juce::Timer
{
public:
    static constexpr int fftOrder = 11;              // 2048
    static constexpr int fftSize  = 1 << fftOrder;

    explicit SpectrumAnalyzer (aur::AnalyzerFifo<4096>& feed);
    ~SpectrumAnalyzer() override;

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    aur::AnalyzerFifo<4096>& fifo;
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, fftSize * 2> fftData {};
    std::array<float, fftSize / 2> smoothed {};
};
}
