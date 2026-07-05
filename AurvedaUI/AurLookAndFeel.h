#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace aur::ui
{
/**
    Suite-wide look and feel. Derives every colour from the current Theme, so
    restyling the whole suite is a matter of editing Theme.h or calling
    setTheme(). Draws the signature glowing rotary knob.
*/
class AurLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AurLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int, int, int, int,
                           float, float, float, juce::Slider&) override;

    /** Re-reads colours from the current Theme (call after setTheme). */
    void applyTheme();
};
}
