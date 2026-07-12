#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorEditor* TunerProcessor::createEditor() { return new TunerEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new TunerProcessor(); }
