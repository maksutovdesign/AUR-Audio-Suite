#include "PluginEditor.h"
#include "Theme.h"
#include "Branding.h"
using namespace aur::ui;

GonioEditor::GonioEditor (GonioProcessor& p) : AudioProcessorEditor (p), ap (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (scope);
    setSize (460, 520);
}
GonioEditor::~GonioEditor() { setLookAndFeel (nullptr); }

void GonioEditor::applyThemeChoice (int i)
{
    switch (i) { case 1: setTheme (obsidianTheme()); break; case 2: setTheme (fluxTheme()); break; default: setTheme (moltenTheme()); }
    lnf.applyTheme(); sendLookAndFeelChange(); repaint();
}

void GonioEditor::paint (juce::Graphics& g)
{
    const auto& t = theme();
    g.fillAll (t.ground);
    drawBrandHeader (g, { 20, 16, 360, 60 }, "GONIO", "Stereo vectorscope");
}

void GonioEditor::resized()
{
    auto area = getLocalBounds().reduced (18);
    auto header = area.removeFromTop (56);
    area.removeFromTop (6);
    scope.setBounds (area);
}
