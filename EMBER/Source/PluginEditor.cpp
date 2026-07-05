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

    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    setSize (720, 420);
}

EmberEditor::~EmberEditor() { setLookAndFeel (nullptr); }

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
    header.removeFromLeft (240);
    bypassButton.setBounds (header.removeFromRight (100).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (170).reduced (4, 14));
    flavorBox.setBounds    (header.removeFromRight (120).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (76);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    auto centre = area.removeFromTop (200);
    driveKnob->setBounds (centre.withSizeKeepingCentre (200, centre.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 4;
    inputKnob ->setBounds (row.removeFromLeft (kw).reduced (6));
    mixKnob   ->setBounds (row.removeFromLeft (kw).reduced (6));
    toneKnob  ->setBounds (row.removeFromLeft (kw).reduced (6));
    outputKnob->setBounds (row.removeFromLeft (kw).reduced (6));
}
