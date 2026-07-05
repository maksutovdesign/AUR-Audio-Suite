#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Metering.h"

namespace aur::ui
{
/** Vertical peak meter. Polls a MeterState on a timer with visual decay.
    `input` meters use the precision (teal) colour, `output` the accent (heat). */
class MeterComponent : public juce::Component,
                       private juce::Timer
{
public:
    enum class Which { input, output };

    MeterComponent (aur::MeterState& state, Which which, juce::String label);
    ~MeterComponent() override;

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;

    aur::MeterState& meterState;
    Which which;
    juce::String labelText;
    float displayValue = 0.0f;
};
}
