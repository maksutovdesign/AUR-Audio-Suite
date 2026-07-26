#!/usr/bin/env bash
# Per-plugin download artefacts: one zip per module (VST3 + AU) into site/downloads/.
# Staged from the installed copies in ~/Library/Audio/Plug-Ins.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/site/downloads"
VST3="$HOME/Library/Audio/Plug-Ins/VST3"
COMP="$HOME/Library/Audio/Plug-Ins/Components"
rm -rf "$OUT"; mkdir -p "$OUT"
export COPYFILE_DISABLE=1

# All modules by their AUR product name (dir name).
MODULES=(EMBER CLARITY GRIP CEIL PRISM HAZE SCOPE IMAGER DELAY MOTION ASSIST FORGE \
  DEESS DEHUM GATE DENOISE CHORUS FLANGER PHASER TREMOLO CLIPPER CRUSH TRANSIENT \
  MULTI EXCITE GAIN GONIO TUNER PITCH CONVO RING \
  AURORA PULSE NOVA MINI PLUCK SUB SUPER CHIP DRONE ORGAN BELL STRING RES KIT8 KIT9 \
  FMPERC WAVE VECTOR GRAIN PAD BEAT HYPER MORPH)

echo "name,size" > "$OUT/manifest.csv"
for m in "${MODULES[@]}"; do
  tmp="$(mktemp -d)"; dst="$tmp/AUR $m"; mkdir -p "$dst"
  v="$VST3/AUR $m.vst3"; c="$COMP/AUR $m.component"
  [ -d "$v" ] && cp -R "$v" "$dst/"
  [ -d "$c" ] && cp -R "$c" "$dst/"
  if [ -z "$(ls -A "$dst" 2>/dev/null)" ]; then echo "  skip $m (no artefacts)"; rm -rf "$tmp"; continue; fi
  cat > "$dst/INSTALL.txt" <<TXT
AUR $m — Aurveda Audio
Copy "AUR $m.vst3"      -> ~/Library/Audio/Plug-Ins/VST3/
Copy "AUR $m.component" -> ~/Library/Audio/Plug-Ins/Components/
Then rescan plug-ins in your DAW. Universal (Apple Silicon + Intel).
TXT
  ( cd "$tmp" && zip -rqX "$OUT/AUR-$m.zip" "AUR $m" )
  sz=$(du -h "$OUT/AUR-$m.zip" | awk '{print $1}')
  echo "$m,$sz" >> "$OUT/manifest.csv"
  echo "  + AUR-$m.zip ($sz)"
  rm -rf "$tmp"
done
echo "Done -> $OUT ($(ls "$OUT"/*.zip 2>/dev/null | wc -l | tr -d ' ') zips)"
