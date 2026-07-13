#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

DehumEditor::DehumEditor (DehumProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    freqK  = std::make_unique<LabeledKnob> (apvts, ParamID::freq,      "FREQ");
    harmK  = std::make_unique<LabeledKnob> (apvts, ParamID::harmonics, "HARM");
    depthK = std::make_unique<LabeledKnob> (apvts, ParamID::depth,     "DEPTH");
    qK     = std::make_unique<LabeledKnob> (apvts, ParamID::q,         "WIDTH");
    for (auto* k : { freqK.get(), harmK.get(), depthK.get(), qK.get() })
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
    addAndMakeVisible (outMeter);

    setSize (680, 400);
}

DehumEditor::~DehumEditor() { setLookAndFeel (nullptr); }

void DehumEditor::applyThemeChoice (int index)
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

void DehumEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void DehumEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 320.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "DEHUM", "Mains hum remover");
}

void DehumEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (180);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (140).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (76);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    auto row = area;
    const int kw = row.getWidth() / 4;
    freqK ->setBounds (row.removeFromLeft (kw).reduced (8));
    harmK ->setBounds (row.removeFromLeft (kw).reduced (8));
    depthK->setBounds (row.removeFromLeft (kw).reduced (8));
    qK    ->setBounds (row.removeFromLeft (kw).reduced (8));
}
