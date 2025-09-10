#!/usr/bin/env bash
set -euo pipefail

SRC_DIR="${1:-app/Sources}"

echo "==> De-duplicate types and strip inline duplicates in $SRC_DIR"

# 1) AppState — keep AppState.swift; remove duplicates in ContentView & AudioProcessingView
#    Comment out 'class AppState' in any file that's NOT AppState.swift
grep -Rnl --include="*.swift" 'class[[:space:]]\+AppState' "$SRC_DIR" | while read -r f; do
  base="$(basename "$f")"
  if [ "$base" != "AppState.swift" ]; then
    echo "  - Commenting duplicate AppState in $f"
    perl -0777 -pe 's/\n(\s*class\s+AppState\s*:[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$f"
  fi
done

# 2) AnalogInterpolator — keep AnalogInterpolator.swift; strip duplicates in AudioEngine.swift
if [ -f "$SRC_DIR/AnalogInterpolator.swift" ]; then
  if [ -f "$SRC_DIR/AudioEngine.swift" ]; then
    echo "  - Removing duplicate AnalogInterpolator in AudioEngine.swift"
    perl -0777 -pe 's/\n(\s*(public\s+)?(final\s+)?class\s+AnalogInterpolator[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC_DIR/AudioEngine.swift"
  fi
fi

# 3) MoreMojoContainer — keep MoreMojoContainer.swift; remove in ContentView & *_new.swift
#    a) Comment inline duplicate in ContentView.swift
if [ -f "$SRC_DIR/ContentView.swift" ]; then
  echo "  - Stripping inline MoreMojoContainer & AudioProcessingView from ContentView.swift"
  perl -0777 -pe 's/\n(\s*struct\s+MoreMojoContainer[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC_DIR/ContentView.swift"
  perl -0777 -pe 's/\n(\s*struct\s+AudioProcessingView[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC_DIR/ContentView.swift"
fi
#    b) Rename MoreMojoContainer_new.swift -> _old.swift to keep the project clean
if [ -f "$SRC_DIR/MoreMojoContainer_new.swift" ]; then
  echo "  - Renaming MoreMojoContainer_new.swift -> MoreMojoContainer_old.swift"
  mv "$SRC_DIR/MoreMojoContainer_new.swift" "$SRC_DIR/MoreMojoContainer_old.swift"
fi

# 4) AudioProcessingView — keep the dedicated file; already removed inline above.

# 5) InterpMode mismatch — prefer top-level enum
#    a) Change any ProcessorParams.InterpMode references to InterpMode
echo "  - Normalizing ProcessorParams.InterpMode -> InterpMode"
grep -Rnl --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC_DIR" | while read -r f; do
  perl -0777 -pe 's/ProcessorParams\.InterpMode/InterpMode/g' -i "$f"
done
#    b) In Recommendations.swift, fix function signatures that use nested type
if [ -f "$SRC_DIR/Recommendations.swift" ]; then
  echo "  - Fixing Recommendations.swift InterpMode signature"
  perl -0777 -pe 's/private\s+func\s+formatInterpolation\(\s*_?\s*mode:\s*ProcessorParams\.InterpMode\)/private func formatInterpolation(_ mode: InterpMode)/g' -i "$SRC_DIR/Recommendations.swift"
fi

echo "==> Done. Now build."
