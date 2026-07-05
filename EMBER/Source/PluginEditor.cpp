#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"

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
    // Warm ground with a molten glow behind the DRIVE knob.
    g.fillAll (MoltenLookAndFeel::ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (MoltenLookAndFeel::heat.withAlpha (0.16f),
                               cx, 150.0f,
                               MoltenLookAndFeel::ground.withAlpha (0.0f),
                               cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    // Brand
    g.setColour (MoltenLookAndFeel::heat);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("AUR", 20, 16, 60, 14, juce::Justification::left);
    g.setColour (MoltenLookAndFeel::ink);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("EMBER", 20, 28, 200, 30, juce::Justification::left);
    g.setColour (MoltenLookAndFeel::inkDim);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("ADAA Saturator · analog heat, digital law", 20, 58, 340, 16, juce::Justification::left);
}

void EmberEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    // Header row
    auto header = area.removeFromTop (56);
    header.removeFromLeft (240);
    bypassButton.setBounds (header.removeFromRight (100).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (170).reduced (4, 14));
    flavorBox.setBounds    (header.removeFromRight (120).reduced (4, 14));

    area.removeFromTop (6);

    // Meters on the right
    auto meterCol = area.removeFromRight (76);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    // Central DRIVE knob (large)
    auto centre = area.removeFromTop (200);
    const int bigW = 200;
    driveKnob->setBounds (centre.withSizeKeepingCentre (bigW, centre.getHeight()));

    // Bottom row of secondary knobs
    auto row = area;
    const int n = 4;
    const int kw = row.getWidth() / n;
    inputKnob ->setBounds (row.removeFromLeft (kw).reduced (6));
    mixKnob   ->setBounds (row.removeFromLeft (kw).reduced (6));
    toneKnob  ->setBounds (row.removeFromLeft (kw).reduced (6));
    outputKnob->setBounds (row.removeFromLeft (kw).reduced (6));
}
