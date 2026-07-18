#include "PluginEditor.h"
#include "Parameters.h"
#include "Presets.h"
#include "Theme.h"
#include "Branding.h"

using namespace aur::ui;

NovaEditor::NovaEditor (NovaProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);

    auto opCells = [] (int i) -> std::vector<Cell>
    {
        return { { ParamID::ratio (i), "RATIO", false }, { ParamID::level (i), "LEVEL", false },
                 { ParamID::atk (i), "ATK", false }, { ParamID::dec (i), "DEC", false },
                 { ParamID::sus (i), "SUS", false }, { ParamID::rel (i), "REL", false } };
    };
    sections = {
        { "OPERATOR 1", opCells (0), 0 },
        { "OPERATOR 2", opCells (1), 0 },
        { "OPERATOR 3", opCells (2), 1 },
        { "OPERATOR 4", opCells (3), 1 },
        { "GLOBAL", { { ParamID::algo, "ALGO", true }, { ParamID::feedback, "FEEDBK", false },
                      { ParamID::glide, "GLIDE", false }, { ParamID::drive, "DRIVE", false },
                      { ParamID::volume, "VOLUME", false } }, 2 },
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
    setSize (960, 640);
}

NovaEditor::~NovaEditor() { stopTimer(); setLookAndFeel (nullptr); }

void NovaEditor::buildControls()
{
    auto& apvts = audioProcessor.getAPVTS();
    for (const auto& sec : sections)
        for (const auto& c : sec.cells)
        {
            std::unique_ptr<juce::Component> comp;
            if (c.combo) comp = std::make_unique<LabeledCombo> (apvts, c.id, c.caption);
            else         comp = std::make_unique<LabeledKnob>  (apvts, c.id, c.caption);
            addAndMakeVisible (*comp);
            cellByOrder.push_back (comp.get());
            controls.push_back (std::move (comp));
        }
}

void NovaEditor::refreshPresetBox()
{
    presetBox.clear (juce::dontSendNotification);
    int id = 1;
    for (const auto& pr : Presets::getFactoryPresets())
        presetBox.addItem (pr.name, id++);
    presetBox.setSelectedId (audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
}

void NovaEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    const auto cx = (float) getWidth() * 0.5f;
    juce::ColourGradient glow (t.precision.withAlpha (0.10f), cx, 120.0f,
                               t.ground.withAlpha (0.0f), cx, 420.0f, true);
    g.setGradientFill (glow);
    g.fillRect (getLocalBounds());

    drawBrandHeader (g, { 20, 16, 420, 60 }, "NOVA", "4-operator FM synthesizer");

    g.setColour (t.precision);
    g.setFont (monoFont (t.fsCaption, true));
    for (const auto& [title, r] : titleRects)
        g.drawText (title, r, juce::Justification::centredLeft);
}

void NovaEditor::resized()
{
    titleRects.clear();
    auto area = getLocalBounds().reduced (18);

    auto header = area.removeFromTop (56);
    header.removeFromLeft (300);
    presetBox.setBounds (header.removeFromRight (170).reduced (4, 14));
    outMeter.setBounds  (header.removeFromRight (70).reduced (4, 6));

    keyboard.setBounds (area.removeFromBottom (84).reduced (0, 8));
    area.removeFromTop (6);

    const int nCols  = 3;
    const int gutter = 14;
    const int colW   = (area.getWidth() - gutter * (nCols - 1)) / nCols;
    const int cellW  = 88, cellH = 90, titleH = 22;
    const int perRow = juce::jmax (1, colW / cellW);

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
            const int row = (int) i / perRow, pos = (int) i % perRow;
            cellByOrder[cellIdx]->setBounds (colX + pos * cellW, rowsTop + row * cellH, cellW, cellH);
        }
        const int rowsUsed = (int) std::ceil ((double) sec.cells.size() / perRow);
        colY[col] = rowsTop + rowsUsed * cellH + 10;
    }
}
