#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

MotionEditor::MotionEditor (MotionProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    const char* ids[10] = { ParamID::f1, ParamID::t1, ParamID::r1,
                            ParamID::f2, ParamID::t2, ParamID::r2,
                            ParamID::f3, ParamID::t3, ParamID::r3, ParamID::q };
    const char* caps[10] = { "LO F", "LO THR", "LO RNG", "MID F", "MID THR", "MID RNG",
                             "HI F", "HI THR", "HI RNG", "Q" };
    for (int i = 0; i < 10; ++i)
    {
        knobs[(size_t) i] = std::make_unique<LabeledKnob> (apvts, ids[i], caps[i]);
        addAndMakeVisible (*knobs[(size_t) i]);
    }

    addAndMakeVisible (spectrum);

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

    setSize (900, 520);
}

MotionEditor::~MotionEditor() { setLookAndFeel (nullptr); }

void MotionEditor::applyThemeChoice (int index)
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

void MotionEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void MotionEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "MOTION", "Dynamic EQ · 3 bands");
}

void MotionEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (92).reduced (4, 14));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);
    spectrum.setBounds (area.removeFromTop (210));
    area.removeFromTop (12);

    auto row = area.removeFromTop (juce::jmin (150, area.getHeight()));
    const int kw = row.getWidth() / 10;
    for (auto& k : knobs) k->setBounds (row.removeFromLeft (kw).reduced (4));
}
