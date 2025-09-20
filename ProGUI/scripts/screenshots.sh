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
  RUNNER="/Volumes/MP 1/MoreMojo/ChatGPT/ProGUI/Tools/JuceRunner/build/ProGUIJuceRunner.app"
  if [[ ! -d "$RUNNER" ]]; then
    echo "[shot] Runner app not found at $RUNNER" >&2
    exit 1
  fi
  open -a "$RUNNER"
  echo "[shot] Waiting 2s for Runner UI..."
  sleep 2
else
  echo "[shot] Unknown target: $target" >&2
  exit 1
fi

echo "[shot] Capturing full screen to $OUT"
screencapture -x "$OUT"
echo "[shot] Saved $OUT"
