#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace aur { namespace syn
{
/** Host-synced MIDI arpeggiator. Consumes held notes from the incoming buffer
    and, when enabled, replaces them with a generated pattern; non-note messages
    (pitch-bend, CC) pass through. Rate divisions lock to the host PPQ when the
    transport is playing, else free-run at the reported tempo.

    Modes: 0 Up · 1 Down · 2 Up-Down · 3 Random · 4 As-Played. */
struct Arpeggiator
{
    void prepare (double sampleRate) { fs = sampleRate; reset(); }
    void reset()
    {
        held.clear(); playingNote = -1; gateLeft = 0; freeCounter = 0.0;
        lastStep = -9223372036854775807LL; seqPos = 0; udIdx = 0; udDir = 1; rng = 0x9e3779b9u;
    }

    void setParameters (bool on, int mode, int div, int oct, float gate)
    {
        enabled = on;
        mode_ = juce::jlimit (0, 4, mode);
        div_  = juce::jlimit (0, 5, div);
        oct_  = juce::jlimit (1, 4, oct);
        gate_ = juce::jlimit (0.05f, 1.0f, gate);
    }

    bool isEnabled() const { return enabled; }
    /** For the UI: which note-slot is currently sounding (−1 = none). */
    int  currentStep() const { return uiStep; }

    void process (juce::MidiBuffer& midi, double bpm, bool playing, double ppqStart, int numSamples)
    {
        for (const auto meta : midi)
        {
            const auto m = meta.getMessage();
            if (m.isNoteOn() && m.getVelocity() > 0) addHeld (m.getNoteNumber(), m.getVelocity());
            else if (m.isNoteOff() || (m.isNoteOn() && m.getVelocity() == 0)) removeHeld (m.getNoteNumber());
        }
        if (! enabled) return;   // pass the original buffer through untouched

        juce::MidiBuffer out;
        for (const auto meta : midi)
        {
            const auto m = meta.getMessage();
            if (! m.isNoteOnOrOff()) out.addEvent (m, meta.samplePosition);
        }

        buildSeq();
        if (bpm <= 0.0) bpm = 120.0;
        static const double divBeats[6] = { 1.0, 0.5, 0.25, 0.125, 1.0 / 3.0, 1.0 / 6.0 };
        const double beats = divBeats[div_];
        const double samplesPerStep = beats * (60.0 / bpm) * fs;

        for (int s = 0; s < numSamples; ++s)
        {
            bool trig = false;
            if (playing)
            {
                const double ppq = ppqStart + (double) s / fs * (bpm / 60.0);
                const long long step = (long long) std::floor (ppq / beats);
                if (step != lastStep) { lastStep = step; trig = true; }
            }
            else
            {
                if (freeCounter <= 0.0) { trig = true; freeCounter += samplesPerStep; }
                freeCounter -= 1.0;
            }

            if (trig)
            {
                if (playingNote >= 0) { out.addEvent (juce::MidiMessage::noteOff (1, playingNote), s); playingNote = -1; }
                if (! seq.empty())
                {
                    const auto nv = nextNote();
                    out.addEvent (juce::MidiMessage::noteOn (1, nv.first, (juce::uint8) nv.second), s);
                    playingNote = nv.first;
                    gateLeft = (int) std::max (1.0, samplesPerStep * (double) gate_);
                }
            }
            if (playingNote >= 0 && --gateLeft <= 0)
            {
                out.addEvent (juce::MidiMessage::noteOff (1, playingNote), s);
                playingNote = -1;
            }
        }
        midi.swapWith (out);
    }

private:
    void addHeld (int note, int vel)
    {
        for (auto& h : held) if (h.first == note) { h.second = vel; return; }
        held.push_back ({ note, vel });
    }
    void removeHeld (int note)
    {
        held.erase (std::remove_if (held.begin(), held.end(),
                    [note] (const std::pair<int,int>& h) { return h.first == note; }), held.end());
    }
    void buildSeq()
    {
        seq.clear();
        if (held.empty()) return;
        std::vector<std::pair<int,int>> base = held;      // as-played order
        if (mode_ != 4) std::sort (base.begin(), base.end(),
                                   [] (auto& a, auto& b) { return a.first < b.first; });
        for (int o = 0; o < oct_; ++o)
            for (auto& h : base) seq.push_back ({ h.first + 12 * o, h.second });
    }
    std::pair<int,int> nextNote()
    {
        const int n = (int) seq.size();
        int i;
        switch (mode_)
        {
            case 1: i = n - 1 - (seqPos % n); ++seqPos; break;                  // down
            case 2:                                                             // up-down
                i = udIdx; udIdx += udDir;
                if (udIdx >= n - 1) { udIdx = n - 1; udDir = -1; }
                else if (udIdx <= 0) { udIdx = 0; udDir = 1; }
                break;
            case 3: rng = rng * 1664525u + 1013904223u; i = (int) ((rng >> 16) % (unsigned) n); break; // random
            default: i = seqPos % n; ++seqPos; break;                           // up / as-played
        }
        i = juce::jlimit (0, n - 1, i);
        uiStep = i;
        return seq[(size_t) i];
    }

    double fs = 44100.0;
    bool   enabled = false;
    int    mode_ = 0, div_ = 2, oct_ = 1;
    float  gate_ = 0.7f;

    std::vector<std::pair<int,int>> held, seq;
    int    playingNote = -1, gateLeft = 0, seqPos = 0, udIdx = 0, udDir = 1, uiStep = -1;
    double freeCounter = 0.0;
    long long lastStep = 0;
    unsigned rng = 0x9e3779b9u;
};
}} // namespace aur::syn
