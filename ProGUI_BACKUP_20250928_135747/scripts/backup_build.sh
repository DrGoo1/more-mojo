#!/usr/bin/env bash
set -euo pipefail

# Backup current build artefacts, screenshots, and logs into ProGUI/backups/<timestamp>
ROOT_DIR="$(cd "$(dirname "$0")"/../.. && pwd)"
PROGUI_DIR="$ROOT_DIR/ProGUI"
BACKUP_DIR="$PROGUI_DIR/backups"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
DEST="$BACKUP_DIR/$TIMESTAMP"

mkdir -p "$DEST"
mkdir -p "$DEST/screenshots"
mkdir -p "$DEST/logs"

# 1) Build via acceptance to ensure artefacts and screenshots
if [[ -x "$PROGUI_DIR/scripts/acceptance.sh" ]]; then
  echo "[backup] Running acceptance build..."
  bash "$PROGUI_DIR/scripts/acceptance.sh" || true
else
  echo "[backup] acceptance.sh not found or not executable; skipping build step" >&2
fi

# 2) Copy TestApp and Runner (if present)
TESTAPP="$PROGUI_DIR/Tests/TestApp/build/ProGUITestApp.app"
RUNNER="$PROGUI_DIR/Tools/JuceRunner/build/ProGUIJuceRunner_artefacts/ProGUI JUCE Runner.app"

if [[ -d "$TESTAPP" ]]; then
  echo "[backup] Copying TestApp..."
  rsync -a "$TESTAPP" "$DEST/" || true
else
  echo "[backup] TestApp not found at: $TESTAPP" >&2
fi

if [[ -d "$RUNNER" ]]; then
  echo "[backup] Copying JUCE Runner..."
  rsync -a "$RUNNER" "$DEST/" || true
else
  echo "[backup] Runner not found at: $RUNNER" >&2
fi

# 3) Copy screenshots (if any)
if [[ -d "$PROGUI_DIR/screenshots" ]]; then
  echo "[backup] Copying screenshots..."
  rsync -a "$PROGUI_DIR/screenshots/" "$DEST/screenshots/" || true
fi

# 4) Copy logs from /tmp
for LOG in /tmp/progui_demo.log /tmp/progui_demo_swift.log; do
  if [[ -f "$LOG" ]]; then
    cp "$LOG" "$DEST/logs/" || true
  fi
done

# 5) Copy specs/readmes for traceability
cp "$PROGUI_DIR/README_PRO_GUI.md" "$DEST/" 2>/dev/null || true
cp "$PROGUI_DIR/README_PROGRESS.md" "$DEST/" 2>/dev/null || true

# 6) Summarize
echo "[backup] Backup created at: $DEST"

# 7) Tarball for external sharing (optional)
TARBALL="$BACKUP_DIR/backup_$TIMESTAMP.tar.gz"
(tar -C "$BACKUP_DIR" -czf "$TARBALL" "$TIMESTAMP") || true
if [[ -f "$TARBALL" ]]; then
  echo "[backup] Tarball: $TARBALL"
fi
