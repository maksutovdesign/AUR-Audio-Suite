#include "MeterComponent.h"
#include "Theme.h"
#include <juce_audio_basics/juce_audio_basics.h>

namespace aur::ui
{
MeterComponent::MeterComponent (aur::MeterState& state, Which w, juce::String label)
    : meterState (state), which (w), labelText (std::move (label))
{
    startTimerHz (30);
}

MeterComponent::~MeterComponent() { stopTimer(); }

void MeterComponent::timerCallback()
{
    float target;
    if (which == Which::gainReduction)
    {
        // Fuller bar = more reduction (0..18 dB range).
        target = juce::jlimit (0.0f, 1.0f, meterState.getGainReductionDb() / 18.0f);
    }
    else
    {
        const auto peak = (which == Which::input) ? meterState.getInputPeak() : meterState.getOutputPeak();
        const auto db   = juce::Decibels::gainToDecibels (peak, -60.0f);
        target = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f));
    }

    if (target > displayValue) displayValue = target;
    else                       displayValue += (target - displayValue) * 0.2f;

    repaint();
}

void MeterComponent::paint (juce::Graphics& g)
{
    const auto& t = theme();
    auto bounds = getLocalBounds();
    auto labelArea = bounds.removeFromBottom (15);

    g.setColour (t.inkDim);
    g.setFont (monoFont (t.fsCaption, true));
    g.drawText (labelText, labelArea, juce::Justification::centred);

    auto barArea = bounds.reduced (2).toFloat();
    g.setColour (t.line);
    g.fillRoundedRectangle (barArea, 3.0f);

    const auto fillH = barArea.getHeight() * displayValue;
    auto fill = barArea.withTop (barArea.getBottom() - fillH);

    juce::Colour colour;
    if (which == Which::gainReduction) colour = t.accent2;
    else if (displayValue > 0.92f)     colour = t.warning;
    else                               colour = (which == Which::input ? t.precision : t.accent);
    g.setColour (colour);
    g.fillRoundedRectangle (fill, 3.0f);
}
}
