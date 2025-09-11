#!/usr/bin/env bash
set -euo pipefail

# App preflight fixes script to address common build issues
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/app/Sources"

echo "== MoreMojo App Preflight Fix =="

# 1. Move SharedTypes to app module or create umbrella header
echo "Setting up shared module approach..."

# First ensure SharedTypes is properly imported in all files that use it
echo "import Foundation" > "$SRC/MojoTypes.swift"
echo "// Umbrella header for all shared types" >> "$SRC/MojoTypes.swift"

# Add the shared type definitions to the module file
cat "$SRC/SharedTypes.swift" >> "$SRC/MojoTypes.swift"

# Update all files that need to import the shared types
FILES_TO_CHECK=$(grep -r "ProcessorParams\|MojoEQBand" "$SRC" --include="*.swift" | grep -v "SharedTypes\|ProcessorParams+Ext\|MojoTypes" | cut -d: -f1 | sort -u)

for FILE in $FILES_TO_CHECK; do
  echo "Adding proper import for $FILE"
  # Check if import is already there
  if ! grep -q "^import Foundation" "$FILE"; then
    # Add import at the top after any existing imports
    sed -i '' '1s/^/import Foundation\n/' "$FILE"
  fi
  
  # Ensure SharedTypes contents are directly available
  echo "// Added by preflight fix" >> "$SRC/MojoTypes.swift"
  cat "$FILE" >> "$SRC/MojoTypes.swift"
done

# 2. Create MojoMacroMode enum if it doesn't exist
MOJO_MACRO_MODE="$SRC/MojoMacroMode.swift"
if [ ! -f "$MOJO_MACRO_MODE" ]; then
  echo "Creating MojoMacroMode.swift..."
  cat > "$MOJO_MACRO_MODE" << 'EOF'
import Foundation

public enum MojoMacroMode: Int, Codable, Identifiable {
    case appDecides = 0, stealMacro
    public var id: Int { rawValue }
    public var displayName: String {
        switch self {
        case .appDecides: return "App Decides"
        case .stealMacro: return "Steal Macro"
        }
    }
}
EOF
fi

# 3. Add missing methods to MojoMacro.swift
echo "Adding missing methods to MojoMacro..."
MOJO_MACRO="$SRC/MojoMacro.swift"
if ! grep -q "applyAppDecides" "$MOJO_MACRO"; then
  cat >> "$MOJO_MACRO" << 'EOF'

// Extension for macro modes
extension MojoMacro {
    static func applyAppDecides(_ value: Double) -> Double {
        // App-decided macro processing algorithm
        return value * 0.8 + 0.1
    }
    
    static func applyStealMacro(_ value: Double, base: ProcessorParams) -> Double {
        // Macro processing based on stolen parameters
        let driveWeight = Double(base.drive) * 0.7
        return value * (1.0 - driveWeight) + driveWeight * 0.5
    }
}
EOF
fi

echo "== Preflight fixes complete =="
