#!/usr/bin/env bash
# Build a single macOS .pkg that installs the whole AUR suite (VST3 + AU)
# system-wide. Run after every module has been built in Release.
#
#   ./scripts/build-installer.sh
#
# Output: dist/AUR-Suite-<version>.pkg
set -euo pipefail

export COPYFILE_DISABLE=1   # keep AppleDouble ._ sidecars out of the payload

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="0.1.0"
MODULES=(EMBER CLARITY GRIP CEIL PRISM HAZE SCOPE IMAGER DELAY MOTION ASSIST FORGE DEESS DEHUM GATE DENOISE CHORUS FLANGER PHASER TREMOLO CLIPPER CRUSH TRANSIENT MULTI EXCITE GAIN GONIO TUNER PITCH CONVO RING AURORA PULSE NOVA MINI PLUCK SUB SUPER CHIP DRONE ORGAN BELL STRING RES KIT8 KIT9 FMPERC WAVE VECTOR GRAIN PAD BEAT HYPER MORPH)

STAGE="$ROOT/dist/stage"
VST3_DST="$STAGE/Library/Audio/Plug-Ins/VST3"
AU_DST="$STAGE/Library/Audio/Plug-Ins/Components"

rm -rf "$ROOT/dist"
mkdir -p "$VST3_DST" "$AU_DST"

echo "Staging plug-ins…"
for m in "${MODULES[@]}"; do
    art="$ROOT/$m/build/${m}_artefacts/Release"
    vst3="$(ls -d "$art"/VST3/*.vst3 2>/dev/null | head -1 || true)"
    au="$(ls -d "$art"/AU/*.component 2>/dev/null | head -1 || true)"
    # Fallback: stage from the installed copies (COPY_PLUGIN_AFTER_BUILD puts them there).
    [ -z "$vst3" ] && vst3="$(ls -d "$HOME/Library/Audio/Plug-Ins/VST3/AUR ${m}.vst3" 2>/dev/null || true)"
    [ -z "$au" ]   && au="$(ls -d "$HOME/Library/Audio/Plug-Ins/Components/AUR ${m}.component" 2>/dev/null || true)"
    [ -n "$vst3" ] && cp -R "$vst3" "$VST3_DST/" && echo "  + $(basename "$vst3")"
    [ -n "$au" ]   && cp -R "$au"   "$AU_DST/"   && echo "  + $(basename "$au")"
done

echo "Building component package…"
pkgbuild \
    --root "$STAGE" \
    --identifier "com.aurveda.aur.suite" \
    --version "$VERSION" \
    --install-location "/" \
    "$ROOT/dist/aur-suite-component.pkg"

# Distribution wrapper (title, welcome, license) via productbuild.
cat > "$ROOT/dist/distribution.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
    <title>AUR Audio Suite</title>
    <organization>com.aurveda</organization>
    <options customize="never" require-scripts="false" hostArchitectures="x86_64,arm64"/>
    <welcome file="welcome.html"/>
    <license file="LICENSE.txt"/>
    <pkg-ref id="com.aurveda.aur.suite"/>
    <choices-outline><line choice="default"/></choices-outline>
    <choice id="default"><pkg-ref id="com.aurveda.aur.suite"/></choice>
    <pkg-ref id="com.aurveda.aur.suite" version="$VERSION" onConclusion="none">aur-suite-component.pkg</pkg-ref>
</installer-gui-script>
XML

cat > "$ROOT/dist/welcome.html" <<HTML
<html><body style="font-family:-apple-system;padding:16px">
<h2>AUR Audio Suite $VERSION</h2>
<p>Installs the full <b>AUR</b> suite (AU + VST3): <b>31 effects</b> — EQ,
saturation, compression, limiting, reverbs, modulation, restoration, metering —
and <b>23 instruments</b> — synthesizers (virtual-analog, FM, wavetable,
granular, additive, physical modelling) and drum machines.</p>
<p>Universal (Apple Silicon + Intel). Rescan plug-ins in your DAW after installing.</p>
</body></html>
HTML

cp "$ROOT/LICENSE.md" "$ROOT/dist/LICENSE.txt" 2>/dev/null || echo "AUR Suite" > "$ROOT/dist/LICENSE.txt"

productbuild \
    --distribution "$ROOT/dist/distribution.xml" \
    --package-path "$ROOT/dist" \
    --resources "$ROOT/dist" \
    "$ROOT/dist/AUR-Suite-$VERSION.pkg"

echo ""
echo "Done → dist/AUR-Suite-$VERSION.pkg"
echo "(Unsigned: right-click → Open, or 'sudo installer -pkg dist/AUR-Suite-$VERSION.pkg -target /')"
