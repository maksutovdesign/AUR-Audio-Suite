#include "PluginEditor.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

ScopeEditor::ScopeEditor (ScopeProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (spectrum);
    addAndMakeVisible (lMeter);
    addAndMakeVisible (rMeter);


    startTimerHz (20);
    setSize (760, 470);
}

ScopeEditor::~ScopeEditor() { stopTimer(); setLookAndFeel (nullptr); }

void ScopeEditor::applyThemeChoice (int index)
{
    switch (index)
    {
        case 1:  aur::ui::setTheme (aur::ui::obsidianTheme()); break;
        case 2:  aur::ui::setTheme (aur::ui::fluxTheme());     break;
        default: aur::ui::setTheme (aur::ui::moltenTheme());   break;
    }
    lnf.applyTheme();
    sendLookAndFeelChange();
    repaint();
}

void ScopeEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "SCOPE", "Spectrum · LUFS · correlation");

    // LUFS panel.
    g.setColour (t.panel);
    g.fillRoundedRectangle (lufsArea.toFloat(), t.cornerRadius);
    auto drawStat = [&] (juce::Rectangle<int> r, const juce::String& label, float value, juce::Colour col)
    {
        g.setColour (t.inkDim);
        g.setFont (monoFont (t.fsCaption, true));
        g.drawText (label, r.removeFromTop (16), juce::Justification::centred);
        g.setColour (col);
        g.setFont (monoFont (20.0f, true));
        g.drawText (value <= -99.0f ? juce::String ("-inf") : juce::String (value, 1), r, juce::Justification::centred);
    };
    auto la = lufsArea.reduced (10);
    const int half = la.getWidth() / 2;
    drawStat (la.removeFromLeft (half), "MOMENTARY LUFS", audioProcessor.getMomentaryLufs(), t.precisionBright);
    drawStat (la, "SHORT-TERM LUFS", audioProcessor.getShortTermLufs(), t.precision);

    // Correlation meter (-1 .. +1).
    g.setColour (t.panel);
    g.fillRoundedRectangle (corrArea.toFloat(), t.cornerRadius);
    auto ca = corrArea.reduced (12);
    g.setColour (t.inkDim);
    g.setFont (monoFont (t.fsCaption, true));
    g.drawText ("CORRELATION", ca.removeFromTop (16), juce::Justification::centred);
    auto track = ca.reduced (0, 8).toFloat();
    g.setColour (t.line);
    g.fillRoundedRectangle (track, 4.0f);
    const float corr = juce::jlimit (-1.0f, 1.0f, audioProcessor.getCorrelation());
    const float cxPix = track.getCentreX();
    const float half2 = track.getWidth() * 0.5f;
    juce::Rectangle<float> fill;
    if (corr >= 0.0f) fill = { cxPix, track.getY(), corr * half2, track.getHeight() };
    else              fill = { cxPix + corr * half2, track.getY(), -corr * half2, track.getHeight() };
    g.setColour (corr < 0.0f ? t.warning : t.precision);
    g.fillRoundedRectangle (fill, 4.0f);
    g.setColour (t.inkDim);
    g.drawVerticalLine ((int) cxPix, track.getY(), track.getBottom());
    g.setFont (monoFont (t.fsCaption, false));
    g.drawText ("-1", corrArea.getX() + 12, corrArea.getBottom() - 16, 30, 14, juce::Justification::left);
    g.drawText ("+1", corrArea.getRight() - 42, corrArea.getBottom() - 16, 30, 14, juce::Justification::right);
}

void ScopeEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (360);

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (70);
    auto mi = meterCol.reduced (4);
    const auto mw = mi.getWidth() / 2;
    lMeter.setBounds (mi.removeFromLeft (mw).reduced (3));
    rMeter.setBounds (mi.reduced (3));

    area.removeFromRight (8);

    spectrum.setBounds (area.removeFromTop (230));
    area.removeFromTop (12);

    auto bottom = area;
    lufsArea = bottom.removeFromLeft (bottom.getWidth() / 2).reduced (0, 0);
    bottom.removeFromLeft (10);
    corrArea = bottom;
}
