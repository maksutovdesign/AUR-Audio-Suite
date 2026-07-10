#include "PluginProcessor.h"
#include "PluginEditor.h"

ScopeProcessor::ScopeProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void ScopeProcessor::prepareToPlay (double sampleRate, int)
{
    loudness.prepare (sampleRate, getTotalNumOutputChannels());
}

bool ScopeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void ScopeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();

    const float* l = buffer.getReadPointer (0);
    const float* r = totalOut > 1 ? buffer.getReadPointer (1) : l;

    // Peaks (L → input slot, R → output slot).
    meters.pushInputPeak  (buffer.getMagnitude (0, n));
    meters.pushOutputPeak (buffer.getMagnitude (totalOut > 1 ? 1 : 0, n));

    // Loudness + analyzer feed.
    for (int s = 0; s < n; ++s)
    {
        const float* fr[2] = { &l[s], &r[s] };
        loudness.push (fr, (size_t) totalOut);
        analyzer.push (0.5f * (l[s] + r[s]));
    }
    momLufs.store (loudness.momentaryLufs());
    stLufs.store  (loudness.shortTermLufs());

    // Phase correlation over the block, smoothed.
    double sLR = 0.0, sLL = 0.0, sRR = 0.0;
    for (int s = 0; s < n; ++s) { sLR += (double) l[s] * r[s]; sLL += (double) l[s] * l[s]; sRR += (double) r[s] * r[s]; }
    const double denom = std::sqrt (sLL * sRR);
    const float c = denom > 1.0e-9 ? (float) (sLR / denom) : 0.0f;
    corr.store (corr.load() + 0.2f * (c - corr.load()));

    // Audio is passed through unchanged.
    juce::ignoreUnused (buffer);
}

juce::AudioProcessorEditor* ScopeProcessor::createEditor() { return new ScopeEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ScopeProcessor(); }
