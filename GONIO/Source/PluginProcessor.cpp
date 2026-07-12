#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorEditor* GonioProcessor::createEditor() { return new GonioEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GonioProcessor(); }
