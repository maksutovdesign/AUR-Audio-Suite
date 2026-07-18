#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

RingEditor::RingEditor (RingProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    freqKnob   = std::make_unique<LabeledKnob> (apvts, ParamID::freq,   "FREQUENCY");
    mixKnob    = std::make_unique<LabeledKnob> (apvts, ParamID::mix,    "MIX");
    outputKnob = std::make_unique<LabeledKnob> (apvts, ParamID::output, "OUTPUT");
    for (auto* k : { freqKnob.get(), mixKnob.get(), outputKnob.get() })
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

    startTimerHz (15);
    setSize (720, 440);
}

RingEditor::~RingEditor() { stopTimer(); setLookAndFeel (nullptr); }

void RingEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void RingEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "RING", "Ring modulator");
}

void RingEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);

    auto meterCol = area.removeFromRight (100);
    auto mi = meterCol.reduced (6);
    const auto mw = mi.getWidth() / 2;
    inMeter.setBounds  (mi.removeFromLeft (mw).reduced (4));
    outMeter.setBounds (mi.reduced (4));

    area.removeFromRight (8);

    // Hero knob (FREQUENCY) on top, MIX + OUTPUT in a row below.
    auto top = area.removeFromTop (juce::jmin (200, area.getHeight() - 130));
    freqKnob->setBounds (top.withSizeKeepingCentre (190, top.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 2;
    mixKnob->setBounds    (row.removeFromLeft (kw).reduced (10));
    outputKnob->setBounds (row.reduced (10));
}
