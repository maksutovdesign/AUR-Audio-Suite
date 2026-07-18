#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

BeatEditor::BeatEditor (BeatProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    for (int v = 0; v < Drum::Count; ++v)
    {
        auto& r = rows[(size_t) v];
        r.pad.setButtonText (Drum::shortName[(size_t) v]);
        r.pad.onClick = [this, v] { audioProcessor.triggerPad (v); };
        addAndMakeVisible (r.pad);
        r.level = std::make_unique<LabeledKnob> (apvts, ParamID::level (v), "LEVEL");
        r.tune  = std::make_unique<LabeledKnob> (apvts, ParamID::tune (v),  "TUNE");
        r.decay = std::make_unique<LabeledKnob> (apvts, ParamID::decay (v), "DECAY");
        addAndMakeVisible (*r.level); addAndMakeVisible (*r.tune); addAndMakeVisible (*r.decay);
    }

    for (int v = 0; v < Drum::Count; ++v)
        for (int st = 0; st < 16; ++st)
        {
            auto& b = stepBtn[(size_t) v][(size_t) st];
            b.setButtonText ("");
            b.setClickingTogglesState (true);
            addAndMakeVisible (b);
            stepAtt.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
                apvts, ParamID::step (v, st), b));
        }

    driveKnob  = std::make_unique<LabeledKnob> (apvts, ParamID::drive,  "DRIVE");
    volumeKnob = std::make_unique<LabeledKnob> (apvts, ParamID::volume, "VOLUME");
    addAndMakeVisible (*driveKnob); addAndMakeVisible (*volumeKnob);

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this]
    {
        const auto idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) audioProcessor.setCurrentProgram (idx);
    };

    addAndMakeVisible (outMeter);

    startTimerHz (15);
    setSize (980, 760);
}

BeatEditor::~BeatEditor() { stopTimer(); setLookAndFeel (nullptr); }

void BeatEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void BeatEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 120.0f,
                               t.ground.withAlpha (0.0f), cx, 420.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 420, 60 }, "BEAT", "Drum machine + step sequencer");
}

void BeatEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (300);
    presetBox.setBounds (header.removeFromRight (160).reduced (4, 14));
    outMeter.setBounds  (header.removeFromRight (60).reduced (4, 6));

    // Global row along the bottom.
    auto bottom = area.removeFromBottom (96);
    driveKnob->setBounds  (bottom.removeFromLeft (100).reduced (6));
    volumeKnob->setBounds (bottom.removeFromLeft (100).reduced (6));

    area.removeFromTop (6);

    const int rowH = area.getHeight() / Drum::Count;
    for (int v = 0; v < Drum::Count; ++v)
    {
        auto row = area.removeFromTop (rowH).reduced (0, 3);
        auto& r = rows[(size_t) v];
        r.pad.setBounds (row.removeFromLeft (80).reduced (3, 10));
        const int kw = 82;
        r.level->setBounds (row.removeFromLeft (kw).reduced (3, 0));
        r.tune->setBounds  (row.removeFromLeft (kw).reduced (3, 0));
        r.decay->setBounds (row.removeFromLeft (kw).reduced (3, 0));
        row.removeFromLeft (8);
        const int sw = row.getWidth() / 16;
        for (int st = 0; st < 16; ++st)
            stepBtn[(size_t) v][(size_t) st].setBounds (row.removeFromLeft (sw).reduced (2, row.getHeight() / 4));
    }
}
