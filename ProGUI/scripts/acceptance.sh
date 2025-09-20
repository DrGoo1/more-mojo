#!/usr/bin/env bash
set -euo pipefail

# Acceptance script: builds and launches apps, captures screenshots, sanity-checks file size.
# Fails if screenshots are suspiciously small (< 50KB), indicating blank UI.

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SHOT="$ROOT/screenshots"
THRESHOLD=51200 # 50 KB
mkdir -p "$SHOT"

# Build TestApp if script exists
if [ -x "$ROOT/Tests/TestApp/build_test_app.sh" ]; then
  echo "[accept] Building TestApp..."
  bash "$ROOT/Tests/TestApp/build_test_app.sh"
else
  echo "[accept] WARN: TestApp build script not found; skipping build."
fi

# Build JuceRunner (CMake)
if [ -d "$ROOT/Tools/JuceRunner" ]; then
  echo "[accept] Building JuceRunner..."
  mkdir -p "$ROOT/Tools/JuceRunner/build"
  cmake -S "$ROOT/Tools/JuceRunner" -B "$ROOT/Tools/JuceRunner/build" -G "Unix Makefiles"
  cmake --build "$ROOT/Tools/JuceRunner/build" --config Release --parallel 3
else
  echo "[accept] WARN: JuceRunner directory not found; skipping."
fi

# Capture screenshots
bash "$ROOT/scripts/screenshots.sh" testapp || echo "[accept] WARN: TestApp screenshot failed"
bash "$ROOT/scripts/screenshots.sh" runner  || echo "[accept] WARN: Runner screenshot failed"

# Check latest screenshots
LATEST=$(ls -t "$ROOT/screenshots"/*.png 2>/dev/null | head -n 2)
COUNT=0
for f in $LATEST; do
  if [ -f "$f" ]; then
    SIZE=$(stat -f%z "$f" 2>/dev/null || stat -c%s "$f")
    echo "[accept] Screenshot: $f ($SIZE bytes)"
    if [ "$SIZE" -lt "$THRESHOLD" ]; then
      echo "[accept] ERROR: Screenshot too small (blank UI suspected): $f" >&2
      exit 1
    fi
    COUNT=$((COUNT+1))
  fi
done

if [ "$COUNT" -eq 0 ]; then
  echo "[accept] ERROR: No screenshots captured." >&2
  exit 1
fi

echo "[accept] OK: Screenshots look healthy."
