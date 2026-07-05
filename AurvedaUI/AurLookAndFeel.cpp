#include "AurLookAndFeel.h"

namespace aur::ui
{
AurLookAndFeel::AurLookAndFeel() { applyTheme(); }

void AurLookAndFeel::applyTheme()
{
    const auto& t = theme();
    setColour (juce::ResizableWindow::backgroundColourId, t.ground);
    setColour (juce::Slider::textBoxTextColourId, t.ink);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, t.ink);
    setColour (juce::ComboBox::backgroundColourId, t.ground);
    setColour (juce::ComboBox::textColourId, t.ink);
    setColour (juce::ComboBox::outlineColourId, t.line2);
    setColour (juce::ComboBox::arrowColourId, t.accent);
    setColour (juce::PopupMenu::backgroundColourId, t.panel);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, t.accent.withAlpha (0.25f));
    setColour (juce::TextButton::buttonColourId, t.panel);
    setColour (juce::TextButton::buttonOnColourId, t.accent);
    setColour (juce::TextButton::textColourOffId, t.inkMute);
    setColour (juce::TextButton::textColourOnId, t.ground);
}

void AurLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                       float pos, float startAngle, float endAngle,
                                       juce::Slider&)
{
    const auto& t = theme();
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (6.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle  = startAngle + pos * (endAngle - startAngle);
    const auto lineW  = radius * t.knobArcRatio;
    const auto arcR   = radius - lineW * 0.5f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (t.line);
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.f, startAngle, angle, true);
    g.setColour (t.accent.withAlpha (0.25f)); // glow
    g.strokePath (value, juce::PathStrokeType (lineW * 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (t.accent);
    g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto knobR = radius - lineW * 1.7f;
    g.setColour (t.panelHi.brighter (0.10f));
    g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.f, knobR * 2.f);
    g.setColour (t.line2);
    g.drawEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.f, knobR * 2.f, 1.0f);

    juce::Path ptr;
    const auto pl = knobR * 0.88f;
    ptr.addRoundedRectangle (-lineW * 0.32f, -pl, lineW * 0.64f, pl * 0.55f, lineW * 0.32f);
    ptr.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (t.accentBright);
    g.fillPath (ptr);
}
}
