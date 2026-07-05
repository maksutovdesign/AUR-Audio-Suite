#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace aur::ui
{
/** Draws the consistent AUR brand header (suite mark + module name + subtitle)
    so every plugin in the suite is titled identically. */
inline void drawBrandHeader (juce::Graphics& g, juce::Rectangle<int> area,
                             const juce::String& moduleName,
                             const juce::String& subtitle)
{
    const auto& t = theme();
    auto x = area.getX();
    auto y = area.getY();

    g.setColour (t.accent);
    g.setFont (monoFont (t.fsCaption + 1.0f, true));
    g.drawText ("AUR", x, y, 60, 14, juce::Justification::left);

    g.setColour (t.ink);
    g.setFont (sansFont (t.fsTitle, true));
    g.drawText (moduleName, x, y + 12, area.getWidth(), 30, juce::Justification::left);

    g.setColour (t.inkDim);
    g.setFont (sansFont (t.fsCaption + 1.0f));
    g.drawText (subtitle, x, y + 42, area.getWidth(), 16, juce::Justification::left);
}
}
