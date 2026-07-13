#include "PluginEditor.h"
#include "Theme.h"
#include "Branding.h"
#include <cmath>
using namespace aur::ui;

namespace { const char* kNotes[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }; }

TunerEditor::TunerEditor (TunerProcessor& p) : AudioProcessorEditor (p), ap (p)
{
    setLookAndFeel (&lnf);
    startTimerHz (20);
    setSize (460, 340);
}
TunerEditor::~TunerEditor() { stopTimer(); setLookAndFeel (nullptr); }

void TunerEditor::applyThemeChoice (int i)
{
    switch (i) { case 1: setTheme (obsidianTheme()); break; case 2: setTheme (fluxTheme()); break; default: setTheme (moltenTheme()); }
    lnf.applyTheme(); sendLookAndFeelChange(); repaint();
}

void TunerEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "TUNER", "Chromatic tuner");

    const float f = ap.getFrequency();
    juce::String note = "--"; int cents = 0; bool have = f > 0.0f;
    if (have)
    {
        const double midi = 69.0 + 12.0 * std::log2 ((double) f / 440.0);
        const int nearest = (int) std::lround (midi);
        cents = (int) std::lround ((midi - nearest) * 100.0);
        const int idx = ((nearest % 12) + 12) % 12;
        const int octave = nearest / 12 - 1;
        note = juce::String (kNotes[idx]) + juce::String (octave);
    }

    auto area = getLocalBounds().reduced (18);
    area.removeFromTop (64);

    // Note name.
    g.setColour (have && std::abs (cents) <= 5 ? t.precision : t.ink);
    g.setFont (aur::ui::sansFont (72.0f, true));
    g.drawText (note, area.removeFromTop (100), juce::Justification::centred);

    // Cents needle bar (-50..+50).
    auto bar = area.removeFromTop (60).reduced (30, 20).toFloat();
    g.setColour (t.line); g.fillRoundedRectangle (bar, 4.0f);
    const float cxp = bar.getCentreX();
    g.setColour (t.inkDim); g.drawVerticalLine ((int) cxp, bar.getY() - 6, bar.getBottom() + 6);
    if (have)
    {
        const float px = cxp + juce::jlimit (-1.0f, 1.0f, cents / 50.0f) * bar.getWidth() * 0.5f;
        const auto col = std::abs (cents) <= 5 ? t.precision : (std::abs (cents) <= 20 ? t.accentBright : t.warning);
        g.setColour (col);
        g.fillRoundedRectangle (juce::Rectangle<float> (px - 4, bar.getY() - 4, 8, bar.getHeight() + 8), 3.0f);
    }

    // Frequency + cents readout.
    g.setColour (t.inkMute); g.setFont (aur::ui::monoFont (t.fsLabel, true));
    g.drawText (have ? juce::String (f, 1) + " Hz   " + (cents >= 0 ? "+" : "") + juce::String (cents) + " cents"
                     : juce::String ("play a note…"),
                area.removeFromTop (24), juce::Justification::centred);
}

void TunerEditor::resized()
{
    auto header = getLocalBounds().reduced (18).removeFromTop (56);
}
