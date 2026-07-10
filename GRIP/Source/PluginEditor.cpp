#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

GripEditor::GripEditor (GripProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    threshKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::threshold, "THRESH");
    ratioKnob   = std::make_unique<LabeledKnob> (apvts, ParamID::ratio,     "RATIO");
    attackKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::attack,    "ATTACK");
    releaseKnob = std::make_unique<LabeledKnob> (apvts, ParamID::release,   "RELEASE");
    makeupKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::makeup,    "MAKEUP");
    mixKnob     = std::make_unique<LabeledKnob> (apvts, ParamID::mix,       "MIX");
    for (auto* k : { threshKnob.get(), ratioKnob.get(), attackKnob.get(), releaseKnob.get(), makeupKnob.get(), mixKnob.get() })
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

    themeBox.addItemList ({ "Molten", "Obsidian", "Flux" }, 1);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (themeBox);
    themeBox.onChange = [this] { applyThemeChoice (themeBox.getSelectedId() - 1); };

    addAndMakeVisible (inMeter);
    addAndMakeVisible (grMeter);
    addAndMakeVisible (outMeter);

    setSize (760, 430);
}

GripEditor::~GripEditor() { setLookAndFeel (nullptr); }

void GripEditor::applyThemeChoice (int index)
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

void GripEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void GripEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.accent2.withAlpha (0.14f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),   cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "GRIP", "Character compressor");
}

void GripEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (92).reduced (4, 14));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (110);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 3;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    grMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    auto centre = area.removeFromTop (190);
    threshKnob->setBounds (centre.withSizeKeepingCentre (190, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 5;
    ratioKnob  ->setBounds (row.removeFromLeft (kw).reduced (6));
    attackKnob ->setBounds (row.removeFromLeft (kw).reduced (6));
    releaseKnob->setBounds (row.removeFromLeft (kw).reduced (6));
    makeupKnob ->setBounds (row.removeFromLeft (kw).reduced (6));
    mixKnob    ->setBounds (row.removeFromLeft (kw).reduced (6));
}
