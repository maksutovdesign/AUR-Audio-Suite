#!/usr/bin/env bash
# Rebuild every AUR module with the persistent CMake (~/aur-cmake), repointing
# each build dir's Makefiles away from the (wiped) scratchpad cmake first, then
# rebuild the suite installer. Idempotent; up-to-date modules are near-instant.
set -uo pipefail
ROOT="/Users/maksutovdesign/Desktop/Github/AUR"
CM="$HOME/aur-cmake/cmake-3.30.5-macos-universal/CMake.app/Contents/bin/cmake"
NEW="$HOME/aur-cmake/cmake-3.30.5-macos-universal"
BASE="/private/tmp/claude-501/-Users-maksutovdesign-Desktop-Github-aurveda/6fc25343-83a2-44fa-9a4f-ff0bf51bddbf/scratchpad"
LOG="$ROOT/tools/rebuild_all.log"; : > "$LOG"

MODULES=(EMBER CLARITY GRIP CEIL PRISM HAZE SCOPE IMAGER DELAY MOTION ASSIST FORGE \
         DEESS DEHUM GATE DENOISE CHORUS FLANGER PHASER TREMOLO CLIPPER CRUSH \
         TRANSIENT MULTI EXCITE GAIN GONIO TUNER PITCH CONVO RING AURORA PULSE NOVA MINI PLUCK)

for N in "${MODULES[@]}"; do
  D="$ROOT/$N"
  [ -d "$D/build" ] || { echo "$N: no build dir" >>"$LOG"; continue; }
  find "$D/build" -type f -name Makefile -print0 2>/dev/null | xargs -0 sed -i '' \
      -e "s#$BASE/cmfresh/cmake-3.30.5-macos-universal#$NEW#g" \
      -e "s#$BASE/cm/cmake-3.30.5-macos-universal#$NEW#g" \
      -e "s#$BASE/cmake-3.30.5-macos-universal#$NEW#g" 2>/dev/null
  ( cd "$D" && "$CM" --build build --target ${N}_VST3 ${N}_AU --config Release ) >>"$LOG" 2>&1
  if grep -q "Built target ${N}_AU" "$LOG"; then echo "OK  $N" ; else echo "FAIL $N"; fi
done

echo "--- building installer ---"
"$ROOT/scripts/build-installer.sh" >>"$LOG" 2>&1 && echo "installer done" || echo "installer FAIL"
echo "ALL DONE"
