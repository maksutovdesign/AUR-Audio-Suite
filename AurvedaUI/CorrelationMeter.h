#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Metering.h"
#include "Theme.h"

namespace aur::ui
{
/** Horizontal phase-correlation meter (-1 … +1), styled from Theme. */
class CorrelationMeter : public juce::Component,
                         private juce::Timer
{
public:
    explicit CorrelationMeter (aur::MeterState& s) : state (s) { startTimerHz (20); }
    ~CorrelationMeter() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        const auto& t = theme();
        auto r = getLocalBounds().toFloat();

        g.setColour (t.inkDim);
        g.setFont (monoFont (t.fsCaption, true));
        g.drawText ("CORRELATION", r.removeFromTop (15).toNearestInt(), juce::Justification::centred);

        auto track = r.reduced (2.0f, 4.0f);
        g.setColour (t.line);
        g.fillRoundedRectangle (track, 4.0f);

        const float c = juce::jlimit (-1.0f, 1.0f, state.getCorrelation());
        const float cx = track.getCentreX();
        const float half = track.getWidth() * 0.5f;
        juce::Rectangle<float> fill = c >= 0.0f
            ? juce::Rectangle<float> (cx, track.getY(), c * half, track.getHeight())
            : juce::Rectangle<float> (cx + c * half, track.getY(), -c * half, track.getHeight());
        g.setColour (c < 0.0f ? t.warning : t.precision);
        g.fillRoundedRectangle (fill, 4.0f);

        g.setColour (t.inkDim);
        g.drawVerticalLine ((int) cx, track.getY(), track.getBottom());
    }

private:
    void timerCallback() override { repaint(); }
    aur::MeterState& state;
};
}
