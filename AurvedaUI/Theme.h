#pragma once

#include <juce_graphics/juce_graphics.h>

/**
    ============================================================================
    AUR DESIGN SYSTEM — single source of truth
    ============================================================================
    This is THE place to change the look of the whole suite. Every plugin, every
    knob, meter and panel reads its colours, spacing and fonts from here.

    - To tweak the palette or metrics: edit `moltenTheme()` below.
    - To switch the whole suite to another identity: call `aur::ui::setTheme(...)`
      once at startup (e.g. `setTheme (obsidianTheme())`). Obsidian/Flux are
      provided as ready alternative directions.

    Design language MOLTEN: molten-copper accent = analog heat; cool teal =
    precision / metrology; warm near-black neutrals (never pure grey/black).
*/
namespace aur::ui
{
    struct Theme
    {
        // ---- Neutrals (warm-biased, not pure grey) ----
        juce::Colour ground   { 0xff141210 };
        juce::Colour panel    { 0xff1c1815 };
        juce::Colour panelHi  { 0xff231e1a };
        juce::Colour line     { 0xff332a24 };
        juce::Colour line2    { 0xff443830 };

        // ---- HEAT ramp (analog character / primary accent) ----
        juce::Colour accent       { 0xffff8a3d }; // molten copper
        juce::Colour accentBright { 0xffffc98a }; // highlight
        juce::Colour accent2      { 0xffe85f2c }; // forge red
        juce::Colour accentDeep   { 0xff8a2f1c }; // deep ember

        // ---- LAW ramp (precision / metering) ----
        juce::Colour precision       { 0xff37b9a8 };
        juce::Colour precisionBright { 0xff7fe6d6 };

        // ---- Ink & semantic ----
        juce::Colour ink     { 0xfff3ece2 };
        juce::Colour inkMute { 0xffa6988a };
        juce::Colour inkDim  { 0xff6f6357 };
        juce::Colour warning { 0xffff5c4d };
        juce::Colour ok      { 0xff8fcb6e };

        // ---- Metrics ----
        float cornerRadius   = 12.0f;
        float knobArcRatio   = 0.16f;  // arc thickness as a fraction of knob radius
        float controlPadding = 6.0f;

        // ---- Type scale (pt) ----
        float fsTitle   = 22.0f;
        float fsSection = 15.0f;
        float fsLabel   = 12.0f;
        float fsValue   = 11.0f;
        float fsCaption = 10.0f;
    };

    // ---- Ready alternative directions (see design/aur-suite-directions) ----
    inline Theme moltenTheme() { return {}; }

    inline Theme obsidianTheme()
    {
        Theme t;
        t.ground = juce::Colour (0xff0c0c0e);
        t.panel  = juce::Colour (0xff141417);
        t.panelHi= juce::Colour (0xff1a1a1e);
        t.line   = juce::Colour (0xff26262b);
        t.line2  = juce::Colour (0xff34343b);
        t.accent = juce::Colour (0xffe8b24c);       // refined gold
        t.accentBright = juce::Colour (0xfff3d38a);
        t.accent2 = juce::Colour (0xffc9922f);
        t.ink    = juce::Colour (0xffeceae6);
        return t;
    }

    inline Theme fluxTheme()
    {
        Theme t;
        t.ground = juce::Colour (0xff17140f);
        t.panel  = juce::Colour (0xff1e1a24);
        t.accent = juce::Colour (0xffff8a3d);       // warm→cool handled per-component
        t.precision = juce::Colour (0xff37b9a8);
        return t;
    }

    // ---- Current theme (mutable global; UI thread only) ----
    inline Theme& mutableTheme()
    {
        static Theme t = obsidianTheme();   // fixed suite look (no in-plugin selector)
        return t;
    }
    inline const Theme& theme()          { return mutableTheme(); }
    inline void setTheme (const Theme& t) { mutableTheme() = t; }

    // ---- Font helpers (system stacks; mono for data = technical feel) ----
    inline juce::Font sansFont (float height, bool bold = false)
    {
        auto o = juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain);
        return juce::Font (o);
    }
    inline juce::Font monoFont (float height, bool bold = false)
    {
        auto o = juce::FontOptions()
                    .withName (juce::Font::getDefaultMonospacedFontName())
                    .withHeight (height)
                    .withStyle (bold ? "Bold" : "Regular");
        return juce::Font (o);
    }
}
