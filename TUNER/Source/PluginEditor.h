#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "AurLookAndFeel.h"

class TunerEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TunerEditor (TunerProcessor&);
    ~TunerEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    void applyThemeChoice (int);
    TunerProcessor& ap;
    aur::ui::AurLookAndFeel lnf;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TunerEditor)
};
