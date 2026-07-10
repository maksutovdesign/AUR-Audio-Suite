#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "MeterComponent.h"
#include "SpectrumAnalyzer.h"

/** AUR SCOPE editor — spectrum, LUFS, peaks, correlation. */
class ScopeEditor : public juce::AudioProcessorEditor,
                    private juce::Timer
{
public:
    explicit ScopeEditor (ScopeProcessor&);
    ~ScopeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void applyThemeChoice (int index);

    ScopeProcessor& audioProcessor;
    aur::ui::AurLookAndFeel lnf;

    aur::ui::SpectrumAnalyzer spectrum { audioProcessor.getAnalyzer() };
    aur::ui::MeterComponent lMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::input,  "L" };
    aur::ui::MeterComponent rMeter { audioProcessor.getMeterState(), aur::ui::MeterComponent::Which::output, "R" };

    juce::ComboBox themeBox;

    juce::Rectangle<int> lufsArea, corrArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeEditor)
};
