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
MODULES=(EMBER CLARITY GRIP CEIL PRISM HAZE SCOPE IMAGER DELAY MOTION ASSIST FORGE DEESS DEHUM GATE DENOISE CHORUS FLANGER PHASER TREMOLO CLIPPER CRUSH TRANSIENT MULTI EXCITE GAIN GONIO TUNER PITCH CONVO RING AURORA PULSE)

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
<p>Installs 7 plug-ins (AU + VST3): <b>PRISM</b> EQ, <b>EMBER</b> saturator,
<b>GRIP</b> compressor, <b>CLARITY</b> resonance suppressor, <b>HAZE</b> reverb,
<b>CEIL</b> limiter, and <b>SCOPE</b> metering.</p>
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
