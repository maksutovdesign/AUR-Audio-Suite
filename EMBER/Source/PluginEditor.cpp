#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

EmberEditor::EmberEditor (EmberProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    driveKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::drive,      "DRIVE");
    inputKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::inputGain,  "INPUT");
    mixKnob    = std::make_unique<LabeledKnob> (apvts, ParamID::mix,        "MIX");
    toneKnob   = std::make_unique<LabeledKnob> (apvts, ParamID::tone,       "TONE");
    outputKnob = std::make_unique<LabeledKnob> (apvts, ParamID::outputGain, "OUTPUT");
    for (auto* k : { driveKnob.get(), inputKnob.get(), mixKnob.get(), toneKnob.get(), outputKnob.get() })
        addAndMakeVisible (*k);

    flavorBox.addItemList ({ "TUBE", "TAPE", "IRON" }, 1);
    addAndMakeVisible (flavorBox);
    flavorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamID::flavor, flavorBox);

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

    // Live theme switch — proves the shared AurvedaUI design system.
    themeBox.addItemList ({ "Molten", "Obsidian", "Flux" }, 1);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (themeBox);
    themeBox.onChange = [this] { applyThemeChoice (themeBox.getSelectedId() - 1); };

    addAndMakeVisible (spectrum);
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    setSize (800, 500);
}

EmberEditor::~EmberEditor() { setLookAndFeel (nullptr); }

void EmberEditor::applyThemeChoice (int index)
{
    switch (index)
    {
        case 1:  aur::ui::setTheme (aur::ui::obsidianTheme()); break;
        case 2:  aur::ui::setTheme (aur::ui::fluxTheme());     break;
        default: aur::ui::setTheme (aur::ui::moltenTheme());   break;
    }
    lnf.applyTheme();          // re-read JUCE colour IDs from the new theme
    sendLookAndFeelChange();   // propagate to all child components
    repaint();
}

void EmberEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void EmberEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();

    // Warm ground with a molten glow behind the DRIVE knob.
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.accent.withAlpha (0.16f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),  cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 340, 60 }, "EMBER", "ADAA Saturator - analog heat, digital law");
}

void EmberEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (96).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (96).reduced (4, 14));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));
    flavorBox.setBounds    (header.removeFromRight (104).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (76);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    spectrum.setBounds (area.removeFromTop (150));
    area.removeFromTop (10);

    auto centre = area.removeFromTop (168);
    driveKnob->setBounds (centre.withSizeKeepingCentre (200, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 4;
    inputKnob ->setBounds (row.removeFromLeft (kw).reduced (6));
    mixKnob   ->setBounds (row.removeFromLeft (kw).reduced (6));
    toneKnob  ->setBounds (row.removeFromLeft (kw).reduced (6));
    outputKnob->setBounds (row.removeFromLeft (kw).reduced (6));
}
