#!/usr/bin/env bash
# Upload the AUR marketing site (site/) to Beget shared hosting under maksutovdesign.ru.
# Creates the target folder on the server automatically (mkdir -p).
# Run this yourself — it will prompt for your Beget SSH password.
#
#   ./scripts/deploy-beget.sh              # -> maksutovdesign.ru/public_html/aur
#   TARGET= ./scripts/deploy-beget.sh      # -> maksutovdesign.ru/public_html (domain root)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
HOST="maksutic@maksutic.beget.tech"
BASE="maksutovdesign.ru/public_html"
TARGET="${TARGET-aur}"                    # subfolder under public_html ('' = domain root)
DEST="$BASE${TARGET:+/$TARGET}"

echo "Uploading site/ -> $HOST:$DEST"
echo "  (index.html + images/ + downloads/  — downloads are ~316 MB, this can take a while)"
ssh "$HOST" "mkdir -p '$DEST'"
if command -v rsync >/dev/null 2>&1; then
  rsync -avz --delete "$ROOT/site/" "$HOST:$DEST/"
else
  scp -r "$ROOT/site/." "$HOST:$DEST/"
fi
echo ""
echo "Done. URL:  https://maksutovdesign.ru/${TARGET:+$TARGET/}"
echo "(If a page caches stale, Beget caches by full URL — see project_beget_url_cache.)"
