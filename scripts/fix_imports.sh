#!/usr/bin/env bash
set -eo pipefail

echo "===== Fixing imports and resource links ====="

# Get directory containing this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_ROOT/app/Sources"

# 1. Fix common import issues
for file in "$SRC_DIR"/*.swift; do
  # Convert UIKit to AppKit for macOS
  sed -i.bak 's/import UIKit/import AppKit/g' "$file"
  
  # Fix UIImage references
  sed -i.bak 's/UIImage/NSImage/g' "$file"
  
  # Fix UIColor references
  sed -i.bak 's/UIColor/NSColor/g' "$file"
done

# 2. Fix C-style float suffixes
for file in "$SRC_DIR"/*.swift; do
  sed -i.bak 's/\([0-9]\+\.[0-9]\+\)f/\1/g' "$file"
done

# 3. Ensure InterpMode is consistent
for file in "$SRC_DIR"/*.swift; do
  # Fix nested ProcessorParams.InterpMode to top-level InterpMode
  sed -i.bak 's/ProcessorParams\.InterpMode/InterpMode/g' "$file"
done

# 4. Create an InterpMode.swift if it doesn't exist
INTERP_MODE_FILE="$SRC_DIR/InterpMode.swift"
if [ ! -f "$INTERP_MODE_FILE" ]; then
  echo "Creating InterpMode.swift"
  cat > "$INTERP_MODE_FILE" << 'EOF'
import Foundation

public enum InterpMode: Int, Codable, CaseIterable {
    case liveHB4x, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
}
EOF
fi

# Cleanup backup files
find "$SRC_DIR" -name "*.bak" -delete

echo "Imports fixed successfully"
