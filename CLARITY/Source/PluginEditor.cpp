#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

ClarityEditor::ClarityEditor (ClarityProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    depthKnob = std::make_unique<LabeledKnob> (apvts, ParamID::depth,     "DEPTH");
    sensKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::sens,      "SENS");
    sharpKnob = std::make_unique<LabeledKnob> (apvts, ParamID::sharpness, "SHARP");
    mixKnob   = std::make_unique<LabeledKnob> (apvts, ParamID::mix,       "MIX");
    for (auto* k : { depthKnob.get(), sensKnob.get(), sharpKnob.get(), mixKnob.get() })
        addAndMakeVisible (*k);

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this]
    {
        const auto idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) audioProcessor.setCurrentProgram (idx);
    };

    deltaButton.setClickingTogglesState (true);
    addAndMakeVisible (deltaButton);
    deltaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamID::delta, deltaButton);

    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamID::bypass, bypassButton);


    addAndMakeVisible (spectrum);
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    setSize (800, 500);
}

ClarityEditor::~ClarityEditor() { setLookAndFeel (nullptr); }

void ClarityEditor::applyThemeChoice (int index)
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

void ClarityEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void ClarityEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.14f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "CLARITY", "Perceptual resonance suppressor");
}

void ClarityEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    deltaButton.setBounds  (header.removeFromRight (78).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

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
    depthKnob->setBounds (centre.withSizeKeepingCentre (200, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 3;
    sensKnob ->setBounds (row.removeFromLeft (kw).reduced (6));
    sharpKnob->setBounds (row.removeFromLeft (kw).reduced (6));
    mixKnob  ->setBounds (row.removeFromLeft (kw).reduced (6));
}
