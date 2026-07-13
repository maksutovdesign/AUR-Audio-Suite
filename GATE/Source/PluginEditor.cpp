#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

GateEditor::GateEditor (GateProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    thrK   = std::make_unique<LabeledKnob> (apvts, ParamID::threshold, "THRESH");
    rangeK = std::make_unique<LabeledKnob> (apvts, ParamID::range,     "RANGE");
    atkK   = std::make_unique<LabeledKnob> (apvts, ParamID::attack,    "ATTACK");
    holdK  = std::make_unique<LabeledKnob> (apvts, ParamID::hold,      "HOLD");
    relK   = std::make_unique<LabeledKnob> (apvts, ParamID::release,   "RELEASE");
    for (auto* k : { thrK.get(), rangeK.get(), atkK.get(), holdK.get(), relK.get() })
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
    bypassAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ParamID::bypass, bypassButton);


    addAndMakeVisible (inMeter);
    addAndMakeVisible (grMeter);
    addAndMakeVisible (outMeter);

    setSize (760, 400);
}

GateEditor::~GateEditor() { setLookAndFeel (nullptr); }

void GateEditor::applyThemeChoice (int index)
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

void GateEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void GateEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.accent2.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),   cx, 320.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "GATE", "Noise gate · expander");
}

void GateEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (180);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (140).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (100);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 3;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    grMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    auto row = area;
    const int kw = row.getWidth() / 5;
    thrK  ->setBounds (row.removeFromLeft (kw).reduced (6));
    rangeK->setBounds (row.removeFromLeft (kw).reduced (6));
    atkK  ->setBounds (row.removeFromLeft (kw).reduced (6));
    holdK ->setBounds (row.removeFromLeft (kw).reduced (6));
    relK  ->setBounds (row.removeFromLeft (kw).reduced (6));
}
