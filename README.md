# AUR — audio plugin suite

An original suite of audio-effect plugins with one signature sound:
**warm but powerful, dynamic, and strictly correct by all DSP laws.**

The warmth and the rigor are not a compromise — they are two layers:

```
[ clean ZDF/TPT core ]  →  [ dosable analog-character layer ]
  correct filters,           ADAA anti-aliasing, controlled harmonics,
  correct phase, LUFS,       tube / tape / iron flavours
  true-peak
```

Design language: **MOLTEN** — molten-copper accent (analog heat) on warm
near-black, with cool teal reserved for precision readouts (LUFS, true-peak).

## Repository layout

```
AUR/
  AurvedaDSP/          # shared SOUND engine (header-only) — reused by every module
    ADAASaturator.h    #   1st-order antiderivative anti-aliased saturator
    GainStage.h  ToneTilt.h  Metering.h
  AurvedaUI/           # shared DESIGN SYSTEM — reused by every module
    Theme.h            #   ← single source of truth: colours, metrics, fonts
    AurLookAndFeel.*   #   suite look-and-feel (reads Theme)
    Knob.h  MeterComponent.*  Branding.h
  EMBER/               # module 1 — ADAA saturator (Tube / Tape / Iron)
    Source/  Presets/  CMakeLists.txt
```

Two shared foundations make the whole line feel like one product:
- **`AurvedaDSP`** — every module draws its *sound* from the same engine.
- **`AurvedaUI`** — every module draws its *look* from the same design system.
  Edit [`AurvedaUI/Theme.h`](AurvedaUI/Theme.h) (or call `aur::ui::setTheme(...)`)
  and the entire suite restyles at once. Alternative directions
  `obsidianTheme()` / `fluxTheme()` ship ready to switch to.

## Modules (roadmap)

| Module   | Category | Status |
|----------|----------|--------|
| **EMBER**   | Saturator (Tube/Tape/Iron) | ✅ v0.1 — building |
| FORGE    | Channel strip | design concept |
| PRISM    | EQ (dynamic + natural phase) | planned |
| GRIP     | Compressor | planned |
| CEIL     | Limiter (true-peak / LUFS) | planned |
| HAZE     | Reverb / space | planned |
| **CLARITY** | Perceptual de-mask (critical-band) | planned — the revolutionary one |
| SCOPE    | Metering | planned |

## Why ADAA (the "warm but correct" proof)

Cheap saturators fold high harmonics back as inharmonic **aliasing** — the
digital "grit" that makes them sound bad. EMBER uses **1st-order antiderivative
anti-aliasing (ADAA)**: it integrates the waveshaper's antiderivative
(`F1(x)=log cosh x` for `tanh`) across each sample interval instead of sampling
the curve pointwise.

Measured on a loud 7 kHz sine (all legitimate harmonics land *above* the
fundamental, so any energy *below* it is pure aliasing):

```
Trivial tanh : −22.5 dB alias energy
ADAA1        : −43.4 dB alias energy
             → ~21 dB less aliasing, at zero added latency
```

## Build

Needs only CMake ≥ 3.22 and a C++17 compiler — **JUCE is fetched automatically**.

```bash
cd EMBER
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build --config Release
```

Produces a **universal (x86_64 + arm64)** `AUR EMBER.vst3` + Standalone, copied
to `~/Library/Audio/Plug-Ins/VST3/` automatically. Universal matters: an
arm64-only build is invisible to a DAW running under Rosetta.

## EMBER controls

`Flavor` (Tube / Tape / Iron) · `Drive` · `Mix` (parallel) · `Tone` (tilt) ·
`Input` · `Output` · `Bypass`. Five factory presets. Realtime-safe, zero latency.

## License

MIT for AUR's own code. JUCE is licensed separately (GPLv3 or commercial) —
comply with its terms. VST3 uses Steinberg's free proprietary license.
