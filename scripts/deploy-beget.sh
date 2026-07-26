#!/usr/bin/env bash
# Upload the AUR marketing site (site/) to Beget shared hosting.
# Creates the target folder on the server automatically (mkdir -p).
# Run this yourself — it will prompt for your SSH password.
#
# Configure your server in scripts/.deploy-env (gitignored), e.g.:
#     HOST="user@user.beget.tech"
#     BASE="yourdomain.ru/public_html"
# Then:
#     ./scripts/deploy-beget.sh              # -> <BASE>/aur
#     TARGET= ./scripts/deploy-beget.sh      # -> <BASE> (domain root)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
[ -f "$ROOT/scripts/.deploy-env" ] && source "$ROOT/scripts/.deploy-env"
HOST="${HOST:-user@your.beget.tech}"
BASE="${BASE:-yourdomain.ru/public_html}"
TARGET="${TARGET-aur}"                    # subfolder under public_html ('' = domain root)
DEST="$BASE${TARGET:+/$TARGET}"

if [ "$HOST" = "user@your.beget.tech" ]; then
  echo "Set HOST/BASE in scripts/.deploy-env first (see header)."; exit 1
fi
echo "Uploading site/ -> $HOST:$DEST"
echo "  (index.html + images/ + downloads/  — downloads can be large, this may take a while)"
ssh "$HOST" "mkdir -p '$DEST'"
if command -v rsync >/dev/null 2>&1; then
  rsync -avz --delete "$ROOT/site/" "$HOST:$DEST/"
else
  scp -r "$ROOT/site/." "$HOST:$DEST/"
fi
echo ""
echo "Done. URL:  https://${BASE%%/*}/${TARGET:+$TARGET/}"
