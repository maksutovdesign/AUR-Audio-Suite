#include "PluginProcessor.h"
#include "PluginEditor.h"

AssistProcessor::AssistProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", Params::createLayout())
{
    pTarget = apvts.getRawParameterValue (ParamID::target);
    pTone   = apvts.getRawParameterValue (ParamID::tone);
    pInt    = apvts.getRawParameterValue (ParamID::intensity);
    pCeil   = apvts.getRawParameterValue (ParamID::ceiling);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
}

void AssistProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;
    const int ch = getTotalNumOutputChannels();

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) 512, (juce::uint32) ch };
    comp.prepare (spec);
    limiter.prepare (sampleRate, ch, 2.0);
    loudnessIn.prepare (sampleRate, ch);
    loudnessOut.prepare (sampleRate, ch);

    for (auto& f : lpDet) f.setLowpass  (sampleRate, 300.0, 0.707);
    for (auto& f : hpDet) f.setHighpass (sampleRate, 3000.0, 0.707);
    for (auto& f : tiltLow)  f.setLowShelf  (sampleRate, 250.0, 0.707, 0.0);
    for (auto& f : tiltHigh) f.setHighShelf (sampleRate, 4000.0, 0.707, 0.0);

    gainSm.reset (sampleRate, 0.05);
    analyzeLen = (int) (1.5 * sampleRate);
    setLatencySamples (limiter.getLatencySamples());
}

bool AssistProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

void AssistProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn  = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    for (int ch = totalIn; ch < totalOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();
    const float intensity = pInt->load() / 100.0f;

    // ---- Analysis pass (measures the INPUT before processing) ----
    if (analyzing.load())
    {
        if (analyzeRemaining.load() == analyzeLen) { lpAcc = hpAcc = 0.0; accN = 0; loudnessIn.reset(); }

        for (int s = 0; s < n; ++s)
        {
            const float mono = 0.5f * (buffer.getReadPointer (0)[s] + buffer.getReadPointer (totalOut > 1 ? 1 : 0)[s]);
            const float lo = lpDet[0].process (mono);
            const float hi = hpDet[0].process (mono);
            lpAcc += (double) lo * lo;
            hpAcc += (double) hi * hi;
            ++accN;
            const float* fr[2] = { &buffer.getReadPointer (0)[s], &buffer.getReadPointer (totalOut > 1 ? 1 : 0)[s] };
            loudnessIn.push (fr, (size_t) totalOut);
        }

        int rem = analyzeRemaining.load() - n;
        if (rem <= 0)
        {
            const float inLufs = loudnessIn.shortTermLufs();
            const float g = juce::jlimit (-18.0f, 18.0f, Params::targetLufs ((int) *pTarget) - inLufs);
            computedGainDb.store (g);

            const double lpMean = lpAcc / (double) (accN < 1 ? 1 : accN);
            const double hpMean = hpAcc / (double) (accN < 1 ? 1 : accN);
            const float curTilt = (float) (10.0 * std::log10 ((hpMean + 1e-12) / (lpMean + 1e-12)));
            const float t = juce::jlimit (-8.0f, 8.0f, Params::targetTilt ((int) *pTone) - curTilt);
            computedTiltDb.store (t);

            analyzing.store (false);
        }
        else analyzeRemaining.store (rem);
    }

    if (*pBypass > 0.5f)
    {
        for (int s = 0; s < n; ++s)
        {
            const float* fr[2] = { &buffer.getReadPointer (0)[s], &buffer.getReadPointer (totalOut > 1 ? 1 : 0)[s] };
            loudnessOut.push (fr, (size_t) totalOut);
        }
        outLufs.store (loudnessOut.shortTermLufs());
        return;
    }

    // ---- Apply the (intensity-scaled) learned settings ----
    const float tiltDb = computedTiltDb.load() * intensity;
    for (auto& f : tiltLow)  f.setLowShelf  (fs, 250.0,  0.707, -0.5 * tiltDb);
    for (auto& f : tiltHigh) f.setHighShelf (fs, 4000.0, 0.707,  0.5 * tiltDb);
    gainSm.setTargetValue (juce::Decibels::decibelsToGain (computedGainDb.load() * intensity));

    for (int ch = 0; ch < totalOut; ++ch)
    {
        auto* d = buffer.getWritePointer (ch);
        const size_t c = (size_t) juce::jmin (ch, 1);
        for (int s = 0; s < n; ++s)
            d[s] = tiltHigh[c].process (tiltLow[c].process (d[s]));
    }

    // Makeup gain toward target.
    for (int s = 0; s < n; ++s)
    {
        const float g = gainSm.getNextValue();
        for (int ch = 0; ch < totalOut; ++ch)
            buffer.getWritePointer (ch)[s] *= g;
    }

    // Glue compression (fixed gentle, scaled by intensity) + limiter.
    comp.setParameters (-16.0f, 1.0f + 1.5f * intensity, 20.0f, 200.0f, 0.0f, 100.0f);
    juce::dsp::AudioBlock<float> block (buffer);
    comp.process (block);

    limiter.setParameters (pCeil->load(), 100.0f, 0.0f);
    limiter.process (buffer.getArrayOfWritePointers(), totalOut, n);

    // Output loudness + spectrum.
    for (int s = 0; s < n; ++s)
    {
        const float* fr[2] = { &buffer.getReadPointer (0)[s], &buffer.getReadPointer (totalOut > 1 ? 1 : 0)[s] };
        loudnessOut.push (fr, (size_t) totalOut);
        analyzer.push (0.5f * (buffer.getReadPointer (0)[s] + buffer.getReadPointer (totalOut > 1 ? 1 : 0)[s]));
    }
    outLufs.store (loudnessOut.shortTermLufs());
}

juce::AudioProcessorEditor* AssistProcessor::createEditor() { return new AssistEditor (*this); }

void AssistProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        // Persist the learned amounts alongside the parameters.
        state.setProperty ("computedGainDb", computedGainDb.load(), nullptr);
        state.setProperty ("computedTiltDb", computedTiltDb.load(), nullptr);
        juce::MemoryOutputStream mos (dest, false);
        state.writeToStream (mos);
    }
}

void AssistProcessor::setStateInformation (const void* data, int size)
{
    auto tree = juce::ValueTree::readFromData (data, (size_t) size);
    if (tree.isValid())
    {
        computedGainDb.store ((float) tree.getProperty ("computedGainDb", 0.0));
        computedTiltDb.store ((float) tree.getProperty ("computedTiltDb", 0.0));
        apvts.replaceState (tree);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AssistProcessor(); }
