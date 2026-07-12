#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AnalyzerFifo.h"
#include "Theme.h"

/** Lissajous vectorscope: plots (L,R) rotated 45° so mono = vertical line,
    wide/out-of-phase spreads horizontally. Fades a trail for readability. */
class Goniometer : public juce::Component, private juce::Timer
{
public:
    Goniometer (aur::AnalyzerFifo<4096>& l, aur::AnalyzerFifo<4096>& r) : fl (l), fr (r) { startTimerHz (30); }
    ~Goniometer() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        const auto& t = aur::ui::theme();
        auto r = getLocalBounds().toFloat();
        const float cx = r.getCentreX(), cy = r.getCentreY();
        const float rad = std::min (r.getWidth(), r.getHeight()) * 0.46f;

        g.setColour (t.panel); g.fillRoundedRectangle (r, t.cornerRadius);
        // guides: L/R diagonals + M vertical + S horizontal
        g.setColour (t.line);
        g.drawLine (cx - rad, cy - rad, cx + rad, cy + rad);
        g.drawLine (cx - rad, cy + rad, cx + rad, cy - rad);
        g.setColour (t.line.withAlpha (0.6f));
        g.drawLine (cx, cy - rad, cx, cy + rad);
        g.drawLine (cx - rad, cy, cx + rad, cy);
        g.setColour (t.inkDim); g.setFont (aur::ui::monoFont (t.fsCaption - 1.0f, true));
        g.drawText ("L", (int) (cx - rad), (int) (cy - rad - 14), 20, 12, juce::Justification::centred);
        g.drawText ("R", (int) (cx + rad - 20), (int) (cy - rad - 14), 20, 12, juce::Justification::centred);

        const int N = 1024;
        static float bl[N], br[N];
        fl.readLatest (bl, (uint32_t) N);
        fr.readLatest (br, (uint32_t) N);

        juce::Path pts;
        const float k = 0.70710678f;
        for (int i = 0; i < N; ++i)
        {
            const float x = cx + (bl[i] - br[i]) * k * rad;   // side → horizontal
            const float y = cy - (bl[i] + br[i]) * k * rad;   // mid  → vertical
            if (i == 0) pts.startNewSubPath (x, y); else pts.lineTo (x, y);
        }
        g.setColour (t.accent.withAlpha (0.75f));
        g.strokePath (pts, juce::PathStrokeType (1.0f));
    }

private:
    void timerCallback() override { repaint(); }
    aur::AnalyzerFifo<4096>& fl;
    aur::AnalyzerFifo<4096>& fr;
};
