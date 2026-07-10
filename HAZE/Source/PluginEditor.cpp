#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

HazeEditor::HazeEditor (HazeProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    decayK = std::make_unique<LabeledKnob> (apvts, ParamID::decay,    "DECAY");
    sizeK  = std::make_unique<LabeledKnob> (apvts, ParamID::size,     "SIZE");
    dampK  = std::make_unique<LabeledKnob> (apvts, ParamID::damp,     "DAMP");
    preK   = std::make_unique<LabeledKnob> (apvts, ParamID::predelay, "PRE");
    widthK = std::make_unique<LabeledKnob> (apvts, ParamID::width,    "WIDTH");
    mixK   = std::make_unique<LabeledKnob> (apvts, ParamID::mix,      "MIX");
    for (auto* k : { decayK.get(), sizeK.get(), dampK.get(), preK.get(), widthK.get(), mixK.get() })
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
    addAndMakeVisible (outMeter);

    setSize (760, 430);
}

HazeEditor::~HazeEditor() { setLookAndFeel (nullptr); }

void HazeEditor::applyThemeChoice (int index)
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

void HazeEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void HazeEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "HAZE", "Warm FDN reverb");
}

void HazeEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (92).reduced (4, 14));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (76);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    auto centre = area.removeFromTop (190);
    decayK->setBounds (centre.withSizeKeepingCentre (180, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 5;
    sizeK ->setBounds (row.removeFromLeft (kw).reduced (6));
    dampK ->setBounds (row.removeFromLeft (kw).reduced (6));
    preK  ->setBounds (row.removeFromLeft (kw).reduced (6));
    widthK->setBounds (row.removeFromLeft (kw).reduced (6));
    mixK  ->setBounds (row.removeFromLeft (kw).reduced (6));
}
