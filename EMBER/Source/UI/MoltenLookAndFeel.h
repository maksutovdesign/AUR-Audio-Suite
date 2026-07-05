#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
    MOLTEN — the shared AUR design language. Warm molten-copper accent on warm
    near-black; cool teal reserved for precision readouts. Large rotary knobs
    with a glowing value arc.
*/
class MoltenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MoltenLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int, int, int, int,
                           float, float, float, juce::Slider&) override;

    static const juce::Colour ground, panel, line, line2;
    static const juce::Colour heat, heat1, ember;
    static const juce::Colour law, ink, inkMute, inkDim;
};
