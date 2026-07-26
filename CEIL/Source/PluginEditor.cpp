#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

CeilEditor::CeilEditor (CeilProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    gainKnob    = std::make_unique<LabeledKnob> (apvts, ParamID::gain,    "GAIN");
    ceilingKnob = std::make_unique<LabeledKnob> (apvts, ParamID::ceiling, "CEILING");
    releaseKnob = std::make_unique<LabeledKnob> (apvts, ParamID::release, "RELEASE");
    for (auto* k : { gainKnob.get(), ceilingKnob.get(), releaseKnob.get() })
        addAndMakeVisible (*k);

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this]
    {
        const auto idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) audioProcessor.setCurrentProgram (idx);
    };

    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamID::bypass, bypassButton);


    addAndMakeVisible (inMeter);
    addAndMakeVisible (grMeter);
    addAndMakeVisible (outMeter);

    startTimerHz (15);
    setSize (720, 440);
}

CeilEditor::~CeilEditor() { stopTimer(); setLookAndFeel (nullptr); }

void CeilEditor::applyThemeChoice (int index)
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

void CeilEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void CeilEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "CEIL", "True-peak limiter - LUFS");

    // LUFS readout panel.
    g.setColour (t.panel);
    g.fillRoundedRectangle (lufsArea.toFloat(), t.cornerRadius);

    auto drawStat = [&] (juce::Rectangle<int> r, const juce::String& label, float value, juce::Colour col)
    {
        g.setColour (t.inkDim);
        g.setFont (monoFont (t.fsCaption, true));
        g.drawText (label, r.removeFromTop (16), juce::Justification::centred);
        g.setColour (col);
        g.setFont (monoFont (22.0f, true));
        const auto txt = value <= -99.0f ? juce::String ("-inf") : juce::String (value, 1);
        g.drawText (txt, r, juce::Justification::centred);
    };

    auto la = lufsArea.reduced (10);
    const int half = la.getWidth() / 2;
    drawStat (la.removeFromLeft (half), "MOMENTARY LUFS", audioProcessor.getMomentaryLufs(), theme().precisionBright);
    drawStat (la, "SHORT-TERM LUFS", audioProcessor.getShortTermLufs(), theme().precision);
}

void CeilEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (110);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 3;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    grMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    lufsArea = area.removeFromBottom (64);
    area.removeFromBottom (10);

    auto centre = area.removeFromTop (juce::jmin (190, area.getHeight() - 120));
    gainKnob->setBounds (centre.withSizeKeepingCentre (180, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 2;
    ceilingKnob->setBounds (row.removeFromLeft (kw).reduced (10));
    releaseKnob->setBounds (row.removeFromLeft (kw).reduced (10));
}
