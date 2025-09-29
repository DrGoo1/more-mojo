#!/usr/bin/env bash
set -euo pipefail

# Collects Swift and C++ demo logs into a timestamped folder under ProGUI/logs
# Sources:
#   /tmp/progui_demo_swift.log
#   /tmp/progui_demo.log

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="$ROOT/logs/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$DEST_DIR"

copy_if_exists() {
  local src="$1"; local dst="$2"
  if [ -f "$src" ]; then
    cp -v "$src" "$dst"
  else
    echo "[collect] WARN: missing $src" >&2
  fi
}

copy_if_exists "/tmp/progui_demo_swift.log" "$DEST_DIR/"
copy_if_exists "/tmp/progui_demo.log" "$DEST_DIR/"

echo "[collect] Logs copied to $DEST_DIR"
