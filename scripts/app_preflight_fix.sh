#!/usr/bin/env bash
set -euo pipefail

# App preflight fixes script to address common build issues
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/app/Sources"

echo "== MoreMojo App Preflight Fix =="

# 1. Add necessary import for SharedTypes
echo "Fixing imports for SharedTypes..."
IMPORT_LINE="import Foundation"
FILES_TO_CHECK=$(grep -r "ProcessorParams" "$SRC" --include="*.swift" | grep -v "SharedTypes\|ProcessorParams+Ext" | cut -d: -f1 | sort -u)

for FILE in $FILES_TO_CHECK; do
  if ! grep -q "import \"SharedTypes.swift\"" "$FILE" && ! grep -q "import SharedTypes" "$FILE"; then
    echo "Adding import for SharedTypes in $FILE"
    sed -i '' "1 a\\
import \"SharedTypes.swift\"
" "$FILE"
  fi
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
