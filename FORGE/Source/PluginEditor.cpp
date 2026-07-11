#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

ForgeEditor::ForgeEditor (ForgeProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    charK   = std::make_unique<LabeledKnob> (apvts, ParamID::character, "CHARACTER");
    inputK  = std::make_unique<LabeledKnob> (apvts, ParamID::input,     "INPUT");
    hpfK    = std::make_unique<LabeledKnob> (apvts, ParamID::hpf,       "HPF");
    toneK   = std::make_unique<LabeledKnob> (apvts, ParamID::tone,      "TONE");
    outputK = std::make_unique<LabeledKnob> (apvts, ParamID::output,    "OUTPUT");
    for (auto* k : { charK.get(), inputK.get(), hpfK.get(), toneK.get(), outputK.get() })
        addAndMakeVisible (*k);

    addAndMakeVisible (spectrum);

    flavorBox.addItemList ({ "TUBE", "TAPE", "IRON" }, 1);
    addAndMakeVisible (flavorBox);
    flavorAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, ParamID::flavor, flavorBox);

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

    themeBox.addItemList ({ "Molten", "Obsidian", "Flux" }, 1);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (themeBox);
    themeBox.onChange = [this] { applyThemeChoice (themeBox.getSelectedId() - 1); };

    addAndMakeVisible (inMeter);
    addAndMakeVisible (grMeter);
    addAndMakeVisible (outMeter);

    setSize (860, 520);
}

ForgeEditor::~ForgeEditor() { setLookAndFeel (nullptr); }

void ForgeEditor::applyThemeChoice (int index)
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

void ForgeEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void ForgeEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.accent.withAlpha (0.16f), cx, 300.0f,
                               t.ground.withAlpha (0.0f),  cx, 520.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "FORGE", "Channel strip · analog heat, digital law");
}

void ForgeEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (92).reduced (4, 14));
    presetBox.setBounds    (header.removeFromRight (160).reduced (4, 14));
    flavorBox.setBounds    (header.removeFromRight (100).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (100);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 3;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    grMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    spectrum.setBounds (area.removeFromTop (150));
    area.removeFromTop (10);

    auto centre = area.removeFromTop (168);
    charK->setBounds (centre.withSizeKeepingCentre (190, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 4;
    inputK ->setBounds (row.removeFromLeft (kw).reduced (6));
    hpfK   ->setBounds (row.removeFromLeft (kw).reduced (6));
    toneK  ->setBounds (row.removeFromLeft (kw).reduced (6));
    outputK->setBounds (row.removeFromLeft (kw).reduced (6));
}
