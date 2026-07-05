#include "MoltenLookAndFeel.h"

const juce::Colour MoltenLookAndFeel::ground  { 0xff141210 };
const juce::Colour MoltenLookAndFeel::panel   { 0xff1c1815 };
const juce::Colour MoltenLookAndFeel::line    { 0xff332a24 };
const juce::Colour MoltenLookAndFeel::line2   { 0xff443830 };
const juce::Colour MoltenLookAndFeel::heat    { 0xffff8a3d };
const juce::Colour MoltenLookAndFeel::heat1   { 0xffffc98a };
const juce::Colour MoltenLookAndFeel::ember   { 0xffe85f2c };
const juce::Colour MoltenLookAndFeel::law     { 0xff37b9a8 };
const juce::Colour MoltenLookAndFeel::ink     { 0xfff3ece2 };
const juce::Colour MoltenLookAndFeel::inkMute { 0xffa6988a };
const juce::Colour MoltenLookAndFeel::inkDim  { 0xff6f6357 };

MoltenLookAndFeel::MoltenLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ground);
    setColour (juce::Slider::textBoxTextColourId, ink);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, ink);
    setColour (juce::ComboBox::backgroundColourId, ground);
    setColour (juce::ComboBox::textColourId, ink);
    setColour (juce::ComboBox::outlineColourId, line2);
    setColour (juce::ComboBox::arrowColourId, heat);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, heat.withAlpha (0.25f));
    setColour (juce::TextButton::buttonColourId, panel);
    setColour (juce::TextButton::buttonOnColourId, heat);
    setColour (juce::TextButton::textColourOffId, inkMute);
    setColour (juce::TextButton::textColourOnId, ground);
}

void MoltenLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                          float pos, float startAngle, float endAngle,
                                          juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (6.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle  = startAngle + pos * (endAngle - startAngle);
    const auto lineW  = radius * 0.16f;
    const auto arcR   = radius - lineW * 0.5f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (line);
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.f, startAngle, angle, true);
    // glow
    g.setColour (heat.withAlpha (0.25f));
    g.strokePath (value, juce::PathStrokeType (lineW * 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (heat);
    g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto knobR = radius - lineW * 1.7f;
    g.setColour (panel.brighter (0.10f));
    g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.f, knobR * 2.f);
    g.setColour (line2);
    g.drawEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.f, knobR * 2.f, 1.0f);

    juce::Path ptr;
    const auto pl = knobR * 0.88f;
    ptr.addRoundedRectangle (-lineW * 0.32f, -pl, lineW * 0.64f, pl * 0.55f, lineW * 0.32f);
    ptr.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (heat1);
    g.fillPath (ptr);
}
