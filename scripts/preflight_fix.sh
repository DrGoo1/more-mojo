#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_SRC="$ROOT/app/Sources"

# 1) Fix C-style float suffixes like 0.0029f -> 0.0029
echo "  - Fix 'f'-suffixed float literals in Swift"
# Only touch .swift files
find "$APP_SRC" -name "*.swift" -print0 | while IFS= read -r -d '' f; do
  # Replace number literals with trailing f/F (outside identifiers)
  perl -0777 -pe 's/\b([0-9]*\.[0-9]+|[0-9]+)[fF]\b/$1/g' -i "$f"
done

# 2) Ensure only one @main and its struct name matches file MoreMojoStudioApp.swift
echo "  - Normalize @main App name to MoreMojoStudioApp"
MAIN_FILE="$APP_SRC/MoreMojoStudioApp.swift"
if [ -f "$MAIN_FILE" ]; then
  perl -0777 -pe 's/@main\s+struct\s+[A-Za-z0-9_]+\s*:\s*App/@main struct MoreMojoStudioApp: App/g' -i "$MAIN_FILE"
fi

# 3) Guard against accidental multiple @main annotations
COUNT=$(grep -R --include="*.swift" -n "@main" "$APP_SRC" | wc -l | tr -d ' ')
if [ "$COUNT" -gt "1" ]; then
  echo "WARNING: Multiple @main annotations detected ($COUNT). Attempting to comment extras."
  # Comment all @main except the one in MoreMojoStudioApp.swift
  grep -R --include="*.swift" -n "@main" "$APP_SRC" | awk -F: '{print $1}' | sort | uniq | \
  while read -r file; do
    if [ "$(basename "$file")" != "MoreMojoStudioApp.swift" ]; then
      perl -0777 -pe 's/@main/\/\/ @main/g' -i "$file"
    fi
  done
fi

# 4) Basic Swift syntax sanity: ensure no bare top-level code in non-@main files
echo "  - Check for obvious top-level statements (print/Task) in non-main files"
find "$APP_SRC" -name "*.swift" ! -name "MoreMojoStudioApp.swift" -print0 | while IFS= read -r -d '' f; do
  if grep -E '^\s*(print|Task\s*\(|Task\s*\{)' "$f" >/dev/null 2>&1; then
    echo "     NOTE: top-level code in $f (commenting out)"
    perl -0777 -pe 's/^(\s*)(print|Task\s*\(|Task\s*\{)/\/\/ \1\2/mg' -i "$f"
  fi
done

# 5) Check for proper Swift optimization level in build settings
echo "  - Check Swift optimization level for Debug builds"
find "$ROOT" -name "*.xcodeproj" -print0 | while IFS= read -r -d '' project; do
  if [ -f "$project/project.pbxproj" ]; then
    echo "    Checking $project"
    # Look for Debug configurations with wrong optimization level
    if grep -A 5 "Debug \*\* = {" "$project/project.pbxproj" | grep -q "SWIFT_OPTIMIZATION_LEVEL = \"-O\";"; then
      echo "    ⚠️ Debug configuration with Release optimization level detected"
      # Modify the project file to set correct optimization level
      perl -0777 -i -pe 's/(Debug \*\* = \{[^}]*)(SWIFT_OPTIMIZATION_LEVEL = \"-O\";)/$1SWIFT_OPTIMIZATION_LEVEL = \"-Onone\";/g' "$project/project.pbxproj"
      echo "    ✅ Fixed: Set Debug configuration to use -Onone"
    fi
  fi
done

echo "  - Preflight fixups complete"
