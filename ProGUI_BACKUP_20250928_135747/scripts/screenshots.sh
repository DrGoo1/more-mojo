#!/usr/bin/env bash
set -euo pipefail

# Simple utility to open the TestApp or JUCE Runner and capture a screenshot after a delay.
# Usage:
#   scripts/screenshots.sh testapp   # screenshots ProGUITestApp
#   scripts/screenshots.sh runner    # screenshots ProGUI JUCE Runner
# Output:
#   ./screenshots/<timestamp>_screen.png

mkdir -p screenshots
STAMP=$(date +%Y%m%d_%H%M%S)
OUT="screenshots/${STAMP}_screen.png"

target="$1" || target="testapp"

echo "[shot] Target: $target"
if [[ "$target" == "testapp" ]]; then
  APP="/Volumes/MP 1/MoreMojo/ChatGPT/ProGUI/Tests/TestApp/build/ProGUITestApp.app"
  open -a "$APP"
  echo "[shot] Waiting 2s for TestApp UI..."
  sleep 2
elif [[ "$target" == "runner" ]]; then
  BASE="/Volumes/MP 1/MoreMojo/ChatGPT/ProGUI/Tools/JuceRunner/build"
  # Try common JUCE output locations and names
  CANDIDATES=(
    "$BASE/ProGUIJuceRunner.app"
    "$BASE/ProGUI JUCE Runner.app"
    "$BASE/ProGUIJuceRunner_artefacts/ProGUIJuceRunner.app"
    "$BASE/ProGUIJuceRunner_artefacts/ProGUI JUCE Runner.app"
  )
  APP=""
  for c in "${CANDIDATES[@]}"; do
    if [[ -d "$c" ]]; then APP="$c"; break; fi
  done
  if [[ -z "$APP" ]]; then
    # Fallback: pick any app under build
    APP=$(ls -dt "$BASE"/*.app "$BASE"/*/*.app 2>/dev/null | head -n 1 || true)
  fi
  if [[ -z "$APP" || ! -d "$APP" ]]; then
    echo "[shot] Runner app not found under $BASE" >&2
    exit 1
  fi
  open -a "$APP"
  echo "[shot] Waiting 2s for Runner UI..."
  sleep 2
else
  echo "[shot] Unknown target: $target" >&2
  exit 1
fi

echo "[shot] Capturing full screen to $OUT"
screencapture -x "$OUT"
echo "[shot] Saved $OUT"
