#include "PluginEditor.h"
#include "Parameters.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

DenoiseEditor::DenoiseEditor (DenoiseProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    addAndMakeVisible (spectrum);

    amountK = std::make_unique<LabeledKnob> (apvts, ParamID::amount,      "AMOUNT");
    sensK   = std::make_unique<LabeledKnob> (apvts, ParamID::sensitivity, "SENS");
    addAndMakeVisible (*amountK);
    addAndMakeVisible (*sensK);

    learnButton.onClick = [this] { audioProcessor.learn(); };
    addAndMakeVisible (learnButton);

    bypassButton.setClickingTogglesState (true);
    addAndMakeVisible (bypassButton);
    bypassAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ParamID::bypass, bypassButton);


    startTimerHz (12);
    setSize (720, 460);
}

DenoiseEditor::~DenoiseEditor() { stopTimer(); setLookAndFeel (nullptr); }

void DenoiseEditor::applyThemeChoice (int index)
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

void DenoiseEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "DENOISE", "Spectral noise reduction");

    g.setColour (t.panel);
    g.fillRoundedRectangle (statusArea.toFloat(), t.cornerRadius);
    const bool learning = audioProcessor.isLearning();
    g.setColour (learning ? t.accentBright : t.inkDim);
    g.setFont (monoFont (t.fsLabel, true));
    g.drawText (learning ? "LEARNING NOISE PROFILE…" : "Play a noise-only section, then press LEARN NOISE",
                statusArea.reduced (12, 0), juce::Justification::centredLeft);
}

void DenoiseEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (220);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));

    area.removeFromTop (6);
    spectrum.setBounds (area.removeFromTop (210));
    area.removeFromTop (10);

    statusArea = area.removeFromBottom (40);
    area.removeFromBottom (10);

    auto row = area;
    learnButton.setBounds (row.removeFromLeft (170).reduced (8, 22));
    const int kw = row.getWidth() / 2;
    amountK->setBounds (row.removeFromLeft (kw).reduced (10));
    sensK  ->setBounds (row.removeFromLeft (kw).reduced (10));
}
