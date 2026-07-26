#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

ImagerEditor::ImagerEditor (ImagerProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    widthK = std::make_unique<LabeledKnob> (apvts, ParamID::width,     "WIDTH");
    monoK  = std::make_unique<LabeledKnob> (apvts, ParamID::monoBelow, "MONO <");
    balK   = std::make_unique<LabeledKnob> (apvts, ParamID::balance,   "BALANCE");
    for (auto* k : { widthK.get(), monoK.get(), balK.get() })
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
    addAndMakeVisible (outMeter);
    addAndMakeVisible (corr);

    setSize (620, 400);
}

ImagerEditor::~ImagerEditor() { setLookAndFeel (nullptr); }

void ImagerEditor::applyThemeChoice (int index)
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

void ImagerEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void ImagerEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 320.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "IMAGER", "Stereo width - mono-maker");
}

void ImagerEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (200);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (70);
    auto mi = meterCol.reduced (4);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (3));
    outMeter.setBounds (mi.reduced (3));

    area.removeFromRight (8);

    corr.setBounds (area.removeFromBottom (50));
    area.removeFromBottom (10);

    auto row = area;
    const int kw = row.getWidth() / 3;
    widthK->setBounds (row.removeFromLeft (kw).reduced (8));
    monoK ->setBounds (row.removeFromLeft (kw).reduced (8));
    balK  ->setBounds (row.removeFromLeft (kw).reduced (8));
}
