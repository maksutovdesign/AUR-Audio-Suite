#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ResonanceSuppressor.h"
#include "Theme.h"

/**
    CLARITY's hero display. Draws the per-band spectrum and overlays, in the
    accent colour, exactly how much each band is being ducked right now — so the
    suppression is visible, not just audible.
*/
class SpectrumView : public juce::Component,
                     private juce::Timer
{
public:
    explicit SpectrumView (aur::ResonanceSuppressor& s) : supp (s) { startTimerHz (30); }
    ~SpectrumView() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        const auto& t = aur::ui::theme();
        auto r = getLocalBounds().toFloat();

        g.setColour (t.panel);
        g.fillRoundedRectangle (r, t.cornerRadius);

        // Faint horizontal grid.
        g.setColour (t.line.withAlpha (0.6f));
        for (int i = 1; i < 4; ++i)
        {
            const float y = r.getY() + r.getHeight() * (float) i / 4.0f;
            g.drawHorizontalLine ((int) y, r.getX() + 6.0f, r.getRight() - 6.0f);
        }

        const auto n = supp.numBands();
        if (n == 0) return;

        const float pad = 8.0f;
        const float w = (r.getWidth() - pad * 2.0f) / (float) n;
        const float baseY = r.getBottom() - pad;
        const float top = r.getY() + pad;
        const float h = baseY - top;

        for (size_t i = 0; i < n; ++i)
        {
            const float lvl = juce::jlimit (0.0f, 1.0f, juce::jmap (supp.vizLevelDb (i), -60.0f, 0.0f, 0.0f, 1.0f));
            const float red = juce::jlimit (0.0f, 1.0f, supp.vizReductionDb (i) / 18.0f);

            const float x = r.getX() + pad + (float) i * w;
            const float bw = w * 0.72f;
            const float barH = h * lvl;
            juce::Rectangle<float> bar (x, baseY - barH, bw, barH);

            // Spectrum bar.
            g.setColour (t.inkDim.withAlpha (0.85f));
            g.fillRoundedRectangle (bar, 2.0f);

            // Reduction overlay: how much is being cut, from the bar top down.
            if (red > 0.01f)
            {
                const float cutH = juce::jmin (barH, h * red);
                juce::Rectangle<float> cut (x, baseY - barH, bw, cutH);
                g.setColour (t.accent.withAlpha (0.9f));
                g.fillRoundedRectangle (cut, 2.0f);
                g.setColour (t.accentBright);
                g.fillRect (x, baseY - barH, bw, 2.0f);
            }
        }

        g.setColour (t.inkDim);
        g.setFont (aur::ui::monoFont (t.fsCaption, true));
        g.drawText ("SPECTRUM  ·  cut in copper", r.reduced (10.0f, 6.0f), juce::Justification::topLeft);
    }

private:
    void timerCallback() override { repaint(); }
    aur::ResonanceSuppressor& supp;
};
