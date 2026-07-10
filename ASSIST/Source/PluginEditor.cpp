#include "PluginEditor.h"
#include "Parameters.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

AssistEditor::AssistEditor (AssistProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    addAndMakeVisible (spectrum);

    intensityK = std::make_unique<LabeledKnob> (apvts, ParamID::intensity, "INTENSITY");
    ceilingK   = std::make_unique<LabeledKnob> (apvts, ParamID::ceiling,   "CEILING");
    addAndMakeVisible (*intensityK);
    addAndMakeVisible (*ceilingK);

    assistButton.onClick = [this] { audioProcessor.startAnalysis(); };
    addAndMakeVisible (assistButton);

    targetBox.addItemList ({ "-14 Streaming", "-12 Balanced", "-9 Loud", "-7 Club" }, 1);
    toneBox.addItemList   ({ "Warm", "Neutral", "Bright" }, 1);
    addAndMakeVisible (targetBox);
    addAndMakeVisible (toneBox);
    targetAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, ParamID::target, targetBox);
    toneAtt   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, ParamID::tone, toneBox);

    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ParamID::bypass, bypassButton);

    themeBox.addItemList ({ "Molten", "Obsidian", "Flux" }, 1);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (themeBox);
    themeBox.onChange = [this] { applyThemeChoice (themeBox.getSelectedId() - 1); };

    startTimerHz (15);
    setSize (800, 520);
}

AssistEditor::~AssistEditor() { stopTimer(); setLookAndFeel (nullptr); }

void AssistEditor::applyThemeChoice (int index)
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

void AssistEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "ASSIST", "Master Assistant");

    // Readout panel: status + learned amounts + output LUFS.
    g.setColour (t.panel);
    g.fillRoundedRectangle (readout.toFloat(), t.cornerRadius);
    auto r = readout.reduced (12);

    const bool analyzing = audioProcessor.isAnalyzing();
    g.setColour (analyzing ? t.accentBright : t.inkDim);
    g.setFont (monoFont (t.fsCaption, true));
    g.drawText (analyzing ? "ANALYSING…" : "LEARNED", r.removeFromTop (16), juce::Justification::centredLeft);

    auto stat = [&] (juce::Rectangle<int> a, const juce::String& lbl, const juce::String& val, juce::Colour col)
    {
        g.setColour (t.inkDim); g.setFont (monoFont (t.fsCaption, false));
        g.drawText (lbl, a.removeFromTop (14), juce::Justification::centredLeft);
        g.setColour (col); g.setFont (monoFont (18.0f, true));
        g.drawText (val, a, juce::Justification::centredLeft);
    };
    const int w = r.getWidth() / 3;
    auto gcol = r.removeFromLeft (w);
    auto tcol = r.removeFromLeft (w);
    stat (gcol, "GAIN", juce::String (audioProcessor.getComputedGainDb(), 1) + " dB", t.accent);
    stat (tcol, "TILT", juce::String (audioProcessor.getComputedTiltDb(), 1) + " dB", t.accentBright);
    const auto lu = audioProcessor.getOutputLufs();
    stat (r, "OUT LUFS", lu <= -99.f ? juce::String ("-inf") : juce::String (lu, 1), t.precisionBright);
}

void AssistEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (200);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (92).reduced (4, 14));

    area.removeFromTop (6);
    spectrum.setBounds (area.removeFromTop (210));
    area.removeFromTop (12);

    readout = area.removeFromBottom (70);
    area.removeFromBottom (12);

    // Control row: ASSIST button, target/tone stacked, intensity, ceiling.
    auto row = area;
    assistButton.setBounds (row.removeFromLeft (150).reduced (6, 24));

    auto combos = row.removeFromLeft (200).reduced (6, 18);
    targetBox.setBounds (combos.removeFromTop (combos.getHeight() / 2 - 4));
    combos.removeFromTop (8);
    toneBox.setBounds (combos);

    const int kw = row.getWidth() / 2;
    intensityK->setBounds (row.removeFromLeft (kw).reduced (8));
    ceilingK  ->setBounds (row.removeFromLeft (kw).reduced (8));
}
