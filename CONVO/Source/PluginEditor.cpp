#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

ConvoEditor::ConvoEditor (ConvoProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    decayKnob    = std::make_unique<LabeledKnob> (apvts, ParamID::decay,    "DECAY");
    toneKnob     = std::make_unique<LabeledKnob> (apvts, ParamID::tone,     "TONE");
    predelayKnob = std::make_unique<LabeledKnob> (apvts, ParamID::predelay, "PRE-DELAY");
    widthKnob    = std::make_unique<LabeledKnob> (apvts, ParamID::width,    "WIDTH");
    mixKnob      = std::make_unique<LabeledKnob> (apvts, ParamID::mix,      "MIX");
    for (auto* k : { decayKnob.get(), toneKnob.get(), predelayKnob.get(), widthKnob.get(), mixKnob.get() })
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

    addAndMakeVisible (loadButton);
    loadButton.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> (
            "Load an impulse response", juce::File{}, "*.wav;*.aif;*.aiff;*.flac");
        const auto flags = juce::FileBrowserComponent::openMode
                         | juce::FileBrowserComponent::canSelectFiles;
        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
        {
            const auto f = fc.getResult();
            if (f.existsAsFile()) audioProcessor.loadImpulseFile (f);
        });
    };

    addAndMakeVisible (synthButton);
    synthButton.onClick = [this] { audioProcessor.useSyntheticIR(); };

    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    startTimerHz (15);
    setSize (720, 470);
}

ConvoEditor::~ConvoEditor() { stopTimer(); setLookAndFeel (nullptr); }

void ConvoEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void ConvoEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.12f), cx, 150.0f,
                               t.ground.withAlpha (0.0f),     cx, 360.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 360, 60 }, "CONVO", "Convolution reverb · IR");

    // IR source readout panel.
    g.setColour (t.panel);
    g.fillRoundedRectangle (irStrip.toFloat(), t.cornerRadius);
    auto r = irStrip.reduced (12, 0);
    g.setColour (t.inkDim);
    g.setFont (monoFont (t.fsCaption, true));
    g.drawText ("IR SOURCE", r.removeFromTop (irStrip.getHeight() / 2), juce::Justification::centredLeft);
    g.setColour (audioProcessor.isUsingFile() ? t.precisionBright : t.ink);
    g.setFont (monoFont (15.0f, true));
    g.drawText (audioProcessor.getIRSourceName(), r, juce::Justification::centredLeft);
}

void ConvoEditor::resized()
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

    // IR-source strip along the bottom: readout panel + Load / Synth buttons.
    auto strip = area.removeFromBottom (44);
    synthButton.setBounds (strip.removeFromRight (90).reduced (4, 6));
    loadButton.setBounds  (strip.removeFromRight (110).reduced (4, 6));
    strip.removeFromRight (8);
    irStrip = strip;
    area.removeFromBottom (10);

    // Hero knob (DECAY) on top, the four shaping knobs in a row below.
    auto top = area.removeFromTop (juce::jmin (190, area.getHeight() - 140));
    decayKnob->setBounds (top.withSizeKeepingCentre (180, top.getHeight()));

    auto row = area;
    const int kw = row.getWidth() / 4;
    toneKnob->setBounds     (row.removeFromLeft (kw).reduced (8));
    predelayKnob->setBounds (row.removeFromLeft (kw).reduced (8));
    widthKnob->setBounds    (row.removeFromLeft (kw).reduced (8));
    mixKnob->setBounds      (row.reduced (8));
}
