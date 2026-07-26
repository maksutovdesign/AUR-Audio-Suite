#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

DelayEditor::DelayEditor (DelayProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    timeK  = std::make_unique<LabeledKnob> (apvts, ParamID::time,     "TIME");
    fbK    = std::make_unique<LabeledKnob> (apvts, ParamID::feedback, "FEEDBACK");
    dampK  = std::make_unique<LabeledKnob> (apvts, ParamID::damp,     "DAMP");
    widthK = std::make_unique<LabeledKnob> (apvts, ParamID::width,    "WIDTH");
    mixK   = std::make_unique<LabeledKnob> (apvts, ParamID::mix,      "MIX");
    for (auto* k : { timeK.get(), fbK.get(), dampK.get(), widthK.get(), mixK.get() })
        addAndMakeVisible (*k);

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this]
    {
        const auto idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) audioProcessor.setCurrentProgram (idx);
    };

    pingButton.setClickingTogglesState (true);
    addAndMakeVisible (pingButton);
    pingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamID::pingpong, pingButton);

    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamID::bypass, bypassButton);


    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    setSize (760, 400);
}

DelayEditor::~DelayEditor() { setLookAndFeel (nullptr); }

void DelayEditor::applyThemeChoice (int index)
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

void DelayEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void DelayEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.accent.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),  cx, 320.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "DELAY", "Stereo echo - damped feedback");
}

void DelayEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (200);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    pingButton.setBounds   (header.removeFromRight (104).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (140).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (76);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    auto row = area;
    const int kw = row.getWidth() / 5;
    timeK ->setBounds (row.removeFromLeft (kw).reduced (6));
    fbK   ->setBounds (row.removeFromLeft (kw).reduced (6));
    dampK ->setBounds (row.removeFromLeft (kw).reduced (6));
    widthK->setBounds (row.removeFromLeft (kw).reduced (6));
    mixK  ->setBounds (row.removeFromLeft (kw).reduced (6));
}
