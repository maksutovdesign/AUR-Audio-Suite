#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

AuroraEditor::AuroraEditor (AuroraProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    sections = {
        { "OSC 1", { { ParamID::osc1shape, "SHAPE", true }, { ParamID::osc1pw, "PW", false }, { ParamID::osc1level, "LEVEL", false } }, 0 },
        { "OSC 2", { { ParamID::osc2shape, "SHAPE", true }, { ParamID::osc2level, "LEVEL", false }, { ParamID::osc2coarse, "COARSE", false }, { ParamID::osc2fine, "FINE", false }, { ParamID::osc2pw, "PW", false } }, 0 },
        { "SUB / NOISE", { { ParamID::sublevel, "SUB", false }, { ParamID::noiselevel, "NOISE", false } }, 0 },

        { "FILTER", { { ParamID::cutoff, "CUTOFF", false }, { ParamID::resonance, "RESO", false }, { ParamID::fdrive, "DRIVE", false }, { ParamID::envamt, "ENV", false }, { ParamID::keytrack, "KEYTRK", false } }, 1 },
        { "FILTER ENV", { { ParamID::fatk, "ATK", false }, { ParamID::fdec, "DEC", false }, { ParamID::fsus, "SUS", false }, { ParamID::frel, "REL", false } }, 1 },
        { "GLOBAL", { { ParamID::drive, "DRIVE", false }, { ParamID::glide, "GLIDE", false }, { ParamID::volume, "VOLUME", false } }, 1 },

        { "AMP ENV", { { ParamID::aatk, "ATK", false }, { ParamID::adec, "DEC", false }, { ParamID::asus, "SUS", false }, { ParamID::arel, "REL", false } }, 2 },
        { "UNISON", { { ParamID::unison, "VOICES", false }, { ParamID::detune, "DETUNE", false }, { ParamID::spread, "SPREAD", false } }, 2 },
        { "LFO", { { ParamID::lforate, "RATE", false }, { ParamID::lfoshape, "SHAPE", true }, { ParamID::lfo2cut, ">CUT", false }, { ParamID::lfo2pitch, ">PITCH", false } }, 2 },
        { "ARP", { { ParamID::arpon, "ON", false }, { ParamID::arpmode, "MODE", true }, { ParamID::arprate, "RATE", true }, { ParamID::arpoct, "OCT", false }, { ParamID::arpgate, "GATE", false } }, 0 },
    };

    buildControls();

    addAndMakeVisible (presetBox);
    refreshPresetBox();
    presetBox.onChange = [this]
    {
        const auto idx = presetBox.getSelectedId() - 1;
        if (idx >= 0) audioProcessor.setCurrentProgram (idx);
    };

    addAndMakeVisible (outMeter);

    keyboard.setKeyWidth (22.0f);
    keyboard.setLowestVisibleKey (36);
    addAndMakeVisible (keyboard);

    startTimerHz (15);
    setSize (960, 820);
}

AuroraEditor::~AuroraEditor() { stopTimer(); setLookAndFeel (nullptr); }

void AuroraEditor::buildControls()
{
    auto& apvts = audioProcessor.getAPVTS();
    for (const auto& sec : sections)
        for (const auto& c : sec.cells)
        {
            std::unique_ptr<juce::Component> comp;
            if (c.combo) comp = std::make_unique<LabeledCombo> (apvts, c.id, c.caption);
            else         comp = std::make_unique<LabeledKnob>  (apvts, c.id, c.caption);
            addAndMakeVisible (*comp);
            cellByOrder.push_back ({ sec.title + "/" + c.id, comp.get() });
            controls.push_back (std::move (comp));
        }
}

void AuroraEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void AuroraEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 120.0f,
                               t.ground.withAlpha (0.0f), cx, 420.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 420, 60 }, "AURORA", "Hybrid virtual-analog polysynth");

    g.setColour (t.precision);
    g.setFont (monoFont (t.fsCaption, true));
    for (const auto& [title, r] : titleRects)
        g.drawText (title, r, juce::Justification::centredLeft);
}

void AuroraEditor::resized()
{
    titleRects.clear();
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (300);
    presetBox.setBounds (header.removeFromRight (170).reduced (4, 14));
    outMeter.setBounds  (header.removeFromRight (70).reduced (4, 6));

    keyboard.setBounds (area.removeFromBottom (84).reduced (0, 8));
    area.removeFromTop (6);

    const int nCols   = 3;
    const int gutter  = 14;
    const int colW    = (area.getWidth() - gutter * (nCols - 1)) / nCols;
    const int cellW   = 88, cellH = 90, titleH = 22;
    const int perRow  = juce::jmax (1, colW / cellW);

    // Index cells by their column via cellByOrder (built in section order).
    size_t cellIdx = 0;
    int colY[3] = { area.getY(), area.getY(), area.getY() };

    for (const auto& sec : sections)
    {
        const int col  = sec.column;
        const int colX = area.getX() + col * (colW + gutter);
        titleRects.push_back ({ sec.title, { colX + 2, colY[col], colW, titleH } });
        const int rowsTop = colY[col] + titleH;

        for (size_t i = 0; i < sec.cells.size(); ++i, ++cellIdx)
        {
            const int row = (int) i / perRow;
            const int pos = (int) i % perRow;
            auto* comp = cellByOrder[cellIdx].second;
            comp->setBounds (colX + pos * cellW, rowsTop + row * cellH, cellW, cellH);
        }
        const int rowsUsed = (int) std::ceil ((double) sec.cells.size() / perRow);
        colY[col] = rowsTop + rowsUsed * cellH + 10;
    }
}
