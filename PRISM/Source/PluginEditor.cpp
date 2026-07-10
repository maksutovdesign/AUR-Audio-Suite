#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

PrismEditor::PrismEditor (PrismProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = audioProcessor.getAPVTS();

    auto mk = [&] (const char* id, const char* cap) { return std::make_unique<LabeledKnob> (apvts, id, cap); };
    hpK  = mk (ParamID::hpFreq,   "HP");
    lsFK = mk (ParamID::lsFreq,   "LOW F");
    lsGK = mk (ParamID::lsGain,   "LOW G");
    bFK  = mk (ParamID::bellFreq, "MID F");
    bGK  = mk (ParamID::bellGain, "MID G");
    bQK  = mk (ParamID::bellQ,    "MID Q");
    hsFK = mk (ParamID::hsFreq,   "HIGH F");
    hsGK = mk (ParamID::hsGain,   "HIGH G");
    lpK  = mk (ParamID::lpFreq,   "LP");
    for (auto* k : { hpK.get(), lsFK.get(), lsGK.get(), bFK.get(), bGK.get(), bQK.get(), hsFK.get(), hsGK.get(), lpK.get() })
        addAndMakeVisible (*k);

    display = std::make_unique<EqDisplay> (audioProcessor.getAnalyzer(), apvts, audioProcessor.getSampleRateHz());
    addAndMakeVisible (*display);

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

    themeBox.addItemList ({ "Molten", "Obsidian", "Flux" }, 1);
    themeBox.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (themeBox);
    themeBox.onChange = [this] { applyThemeChoice (themeBox.getSelectedId() - 1); };

    setSize (860, 520);
}

PrismEditor::~PrismEditor() { setLookAndFeel (nullptr); }

void PrismEditor::applyThemeChoice (int index)
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

void PrismEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void PrismEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "PRISM", "Parametric equalizer");
}

void PrismEditor::resized()
{
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (230);
    bypassButton.setBounds (header.removeFromRight (92).reduced (4, 12));
    themeBox.setBounds     (header.removeFromRight (92).reduced (4, 14));
    presetBox.setBounds    (header.removeFromRight (150).reduced (4, 14));

    area.removeFromTop (6);
    display->setBounds (area.removeFromTop (210));
    area.removeFromTop (12);

    // Nine knobs in a single row, grouped LOW · MID · HIGH.
    auto row = area.removeFromTop (juce::jmin (150, area.getHeight()));
    LabeledKnob* order[9] = { hpK.get(), lsFK.get(), lsGK.get(), bFK.get(), bGK.get(), bQK.get(), hsFK.get(), hsGK.get(), lpK.get() };
    const int kw = row.getWidth() / 9;
    for (auto* k : order) k->setBounds (row.removeFromLeft (kw).reduced (5));
}
