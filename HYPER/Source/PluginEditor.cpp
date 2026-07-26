#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

HyperEditor::HyperEditor (HyperProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    auto& a = audioProcessor.getAPVTS();
    cells = {
        { ParamID::morph1, "MORPH 1", false }, { ParamID::morph2, "MORPH 2", false }, { ParamID::o2coarse, "O2 CRS", false }, { ParamID::o2level, "O2 LVL", false }, { ParamID::fm, "FM", false },
        { ParamID::unison, "UNISON", false }, { ParamID::detune, "DETUNE", false }, { ParamID::spread, "SPREAD", false },
        { ParamID::cutoff, "CUTOFF", false }, { ParamID::reso, "RESO", false }, { ParamID::fmode, "FILTER", true }, { ParamID::envamt, "ENV AMT", false },
        { ParamID::fatk, "F.ATK", false }, { ParamID::fdec, "F.DEC", false }, { ParamID::fsus, "F.SUS", false }, { ParamID::frel, "F.REL", false },
        { ParamID::aatk, "A.ATK", false }, { ParamID::adec, "A.DEC", false }, { ParamID::asus, "A.SUS", false }, { ParamID::arel, "A.REL", false },
        { ParamID::lforate, "LFO RATE", false }, { ParamID::lfo2cut, "LFO>CUT", false }, { ParamID::lfo2morph, "LFO>MRPH", false },
        { ParamID::glide, "GLIDE", false }, { ParamID::drive, "DRIVE", false }, { ParamID::volume, "VOLUME", false },
        { ParamID::arpon, "ARP", false }, { ParamID::arpmode, "A.MODE", true }, { ParamID::arprate, "A.RATE", true }, { ParamID::arpoct, "A.OCT", false }, { ParamID::arpgate, "A.GATE", false },
    };
    for (auto& c : cells)
    {
        std::unique_ptr<juce::Component> comp;
        if (c.combo) comp = std::make_unique<LabeledCombo> (a, c.id, c.caption);
        else         comp = std::make_unique<LabeledKnob>  (a, c.id, c.caption);
        addAndMakeVisible (*comp);
        controls.push_back (std::move (comp));
    }

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this] { const auto i = presetBox.getSelectedId() - 1; if (i >= 0) audioProcessor.setCurrentProgram (i); };
    addAndMakeVisible (outMeter);
    keyboard.setKeyWidth (20.0f); keyboard.setLowestVisibleKey (36);
    addAndMakeVisible (keyboard);

    startTimerHz (15);
    setSize (900, 660);
}

HyperEditor::~HyperEditor() { stopTimer(); setLookAndFeel (nullptr); }

void HyperEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets()) presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void HyperEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 110.0f, t.ground.withAlpha (0.0f), cx, 380.0f, true);
    g.setGradientFill (glow); g.fillRect (getLocalBounds());
    drawBrandHeader (g, { 20, 16, 380, 60 }, "HYPER", "Flagship hybrid synth (WT+FM+unison)");
}

void HyperEditor::resized()
{
    auto area = getLocalBounds().reduced (18);
    auto header = area.removeFromTop (56);
    header.removeFromLeft (250);
    presetBox.setBounds (header.removeFromRight (160).reduced (4, 14));
    outMeter.setBounds  (header.removeFromRight (60).reduced (4, 6));

    keyboard.setBounds (area.removeFromBottom (74).reduced (0, 6));
    area.removeFromTop (6);

    const int cellW = 86, cellH = 90;
    const int perRow = juce::jmax (1, area.getWidth() / cellW);
    for (size_t i = 0; i < controls.size(); ++i)
    {
        const int row = (int) i / perRow, col = (int) i % perRow;
        controls[i]->setBounds (area.getX() + col * cellW, area.getY() + row * cellH, cellW, cellH);
    }
}
