#include "SpectrumAnalyzer.h"
#include "Theme.h"

namespace aur::ui
{
SpectrumAnalyzer::SpectrumAnalyzer (aur::AnalyzerFifo<4096>& feed) : fifo (feed)
{
    startTimerHz (30);
}

SpectrumAnalyzer::~SpectrumAnalyzer() { stopTimer(); }

void SpectrumAnalyzer::timerCallback()
{
    fifo.readLatest (fftData.data(), (uint32_t) fftSize);
    window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    // Normalise to dB and smooth over time for a fluid display.
    for (int i = 0; i < fftSize / 2; ++i)
    {
        const float mag = fftData[(size_t) i] / (float) fftSize;
        const float db  = juce::Decibels::gainToDecibels (mag, -100.0f);
        const float norm = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -90.0f, 0.0f, 0.0f, 1.0f));
        smoothed[(size_t) i] += 0.35f * (norm - smoothed[(size_t) i]);
    }
    repaint();
}

void SpectrumAnalyzer::paint (juce::Graphics& g)
{
    const auto& t = theme();
    auto r = getLocalBounds().toFloat();

    g.setColour (t.panel);
    g.fillRoundedRectangle (r, t.cornerRadius);

    g.setColour (t.line.withAlpha (0.6f));
    for (int i = 1; i < 4; ++i)
    {
        const float y = r.getY() + r.getHeight() * (float) i / 4.0f;
        g.drawHorizontalLine ((int) y, r.getX() + 6.0f, r.getRight() - 6.0f);
    }

    const float pad = 8.0f;
    const auto left = r.getX() + pad, right = r.getRight() - pad;
    const auto baseY = r.getBottom() - pad, topY = r.getY() + pad;
    const float sr = 48000.0f;
    const float minF = 20.0f, maxF = 20000.0f;
    const float logMin = std::log10 (minF), logMax = std::log10 (maxF);

    auto xForBin = [&] (int bin)
    {
        const float freq = juce::jmax (minF, (float) bin * sr / (float) fftSize);
        const float t01 = (std::log10 (freq) - logMin) / (logMax - logMin);
        return left + juce::jlimit (0.0f, 1.0f, t01) * (right - left);
    };

    juce::Path curve;
    bool started = false;
    for (int bin = 1; bin < fftSize / 2; ++bin)
    {
        const float x = xForBin (bin);
        const float y = baseY - (baseY - topY) * smoothed[(size_t) bin];
        if (! started) { curve.startNewSubPath (x, y); started = true; }
        else            curve.lineTo (x, y);
    }

    // Filled body under the curve (heat gradient).
    juce::Path fill = curve;
    fill.lineTo (right, baseY);
    fill.lineTo (left, baseY);
    fill.closeSubPath();
    g.setGradientFill (juce::ColourGradient (t.accent.withAlpha (0.35f), 0, topY,
                                             t.accentDeep.withAlpha (0.05f), 0, baseY, false));
    g.fillPath (fill);

    g.setColour (t.accentBright);
    g.strokePath (curve, juce::PathStrokeType (1.5f));

    g.setColour (t.inkDim);
    g.setFont (monoFont (t.fsCaption, true));
    g.drawText ("SPECTRUM", r.reduced (10.0f, 6.0f), juce::Justification::topLeft);
}
}
