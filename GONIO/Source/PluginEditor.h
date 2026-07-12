#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"
#include "Goniometer.h"

class GonioEditor : public juce::AudioProcessorEditor
{
public:
    explicit GonioEditor (GonioProcessor&);
    ~GonioEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void applyThemeChoice (int);
    GonioProcessor& ap;
    aur::ui::AurLookAndFeel lnf;
    Goniometer scope { ap.getL(), ap.getR() };
    juce::ComboBox themeBox;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GonioEditor)
};
