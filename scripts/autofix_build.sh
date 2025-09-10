#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/app/Sources"

echo "== More Mojo autofix: start =="

mkdir -p "$SRC"

# -------------------------------
# 1) Canonical SharedTypes.swift
# -------------------------------
cat > "$SRC/SharedTypes.swift" <<'EOF'
import Foundation

// Single source of truth for core types
public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
    public var id: Int { rawValue }
    public var displayName: String {
        switch self {
        case .liveHB4x: return "HB 4×"
        case .hqSinc8x: return "HQ Sinc 8×"
        case .transientSpline4x: return "Spline 4×"
        case .adaptive: return "Adaptive"
        case .aiAnalogHook: return "Live+AI"
        }
    }
}

public struct ProcessorParams: Codable, Equatable {
    public var input:  Float = 0.0   // dB
    public var output: Float = 0.0   // dB
    public var drive:      Float = 0.55
    public var character:  Float = 0.50
    public var saturation: Float = 0.45
    public var presence:   Float = 0.50
    public var mix:        Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode:       Int = 1
}

public struct MojoEQBand: Codable, Equatable {
    public var lo: Float
    public var hi: Float
    public var gain_dB: Float
}

public struct MojoEQMatch: Codable, Equatable {
    public var bands: [MojoEQBand]
}
EOF

# ----------------------------------------------------------
# 2) Quarantine duplicate ProcessorParams definitions
#    and create an extension-only file
# ----------------------------------------------------------
PP_FILE="$SRC/ProcessorParams.swift"
if [ -f "$PP_FILE" ]; then
  if grep -qE 'struct\s+ProcessorParams\b' "$PP_FILE"; then
    echo ">> Quarantining duplicate ProcessorParams in ProcessorParams.swift"
    mv "$PP_FILE" "$SRC/ProcessorParams_DEPRECATED.swift"
    # comment the struct body to avoid compile
    perl -0777 -pe 's/(\bstruct\s+ProcessorParams\b[^}]+})/\/\/ DEPRECATED: duplicate removed\n\/\/ $1\n/s' -i "$SRC/ProcessorParams_DEPRECATED.swift"
  fi
fi

# Create/overwrite a small extension file
cat > "$SRC/ProcessorParams+Ext.swift" <<'EOF'
import Foundation

// Extension-only file. Canonical types live in SharedTypes.swift
extension ProcessorParams {
    /// Example convenience mapping for UI (-12..+12 dB -> 0..1)
    public var outputNormalized: Float { (output + 12) / 24 }
}
EOF

# ----------------------------------------------------------
# 3) Normalize nested InterpMode references
# ----------------------------------------------------------
grep -RIl --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" | while read -r f; do
  perl -0777 -pe 's/ProcessorParams\.InterpMode/InterpMode/g' -i "$f"
done

# ----------------------------------------------------------
# 4) Fix StealMojoSwift bands error
#    - Ensure explicit element type + explicit initializer
# ----------------------------------------------------------
SMS="$SRC/StealMojoSwift.swift"
if [ -f "$SMS" ]; then
  # Replace .init(...) with MojoEQBand(...)
  perl -0777 -pe 's/bands\.append\(\.init\(/bands.append(MojoEQBand(/g' -i "$SMS"
  # Insert explicit declaration if not present in the function
  if ! grep -q 'var bands: \[MojoEQBand\]' "$SMS"; then
    # add after a likely "compress to bands" comment, else before first append
    perl -0777 -pe 's/(compress to bands[^\n]*\n)/$1var bands: [MojoEQBand] = []\n/s' -i "$SMS" || true
    if ! grep -q 'var bands: \[MojoEQBand\] = \[\]' "$SMS"; then
      perl -0777 -pe 's/(bands\.append\(MojoEQBand\()/var bands: [MojoEQBand] = []\n$1/s' -i "$SMS"
    fi
  fi
fi

# ----------------------------------------------------------
# 5) Fix MojoWheel call & enum cases in TopConsumerView
# ----------------------------------------------------------
TCV="$SRC/TopConsumerView.swift"
if [ -f "$TCV" ]; then
  # replace wrong enum cases .app / .steal
  perl -0777 -pe 's/\.app\b/.appDecides/g' -i "$TCV"
  perl -0777 -pe 's/\.steal\b/.stealMacro/g' -i "$TCV"
  # fix initializer: remove unsupported labels (houseMacro etc.) and enforce correct signature
  # Replace any multi-line call that starts with MojoWheel( and contains value:, rms:, mode:
  # If there's a known broken pattern with houseMacro param, just replace common snippet:
  perl -0777 -pe 's/MojoWheel\s*\(\s*value:\s*\$wheelVal[^)]*\)/MojoWheel(value: \$wheelVal, rms: CGFloat(engine.rmsOut), mode: macroMode)/s' -i "$TCV"
fi

# ----------------------------------------------------------
# 6) macOS 11 button style fix in StealMojoPanel_SwiftOnly
# ----------------------------------------------------------
PANEL="$SRC/StealMojoPanel_SwiftOnly.swift"
if [ -f "$PANEL" ]; then
  # Add PMXProminent if missing
  if ! grep -q 'struct PMXProminent:' "$PANEL"; then
    perl -0777 -pe 's/import AVFoundation/import AVFoundation\n\nstruct PMXProminent: ButtonStyle {\n    func makeBody(configuration: Configuration) -> some View {\n        configuration.label\n            .padding(.horizontal, 12).padding(.vertical, 6)\n            .background(LinearGradient(colors: [.pink, .purple, .orange], startPoint: .leading, endPoint: .trailing))\n            .foregroundColor(.white)\n            .clipShape(Capsule())\n            .opacity(configuration.isPressed ? 0.8 : 1.0)\n    }\n}\n/s' -i "$PANEL"
  fi
  # Replace .borderedProminent with PMXProminent()
  perl -0777 -pe 's/\.buttonStyle\(\.borderedProminent\)/.buttonStyle(PMXProminent())/g' -i "$PANEL"
fi

# ----------------------------------------------------------
# 7) Remove duplicate source files from target, if present
#    (keep FaceplateOverlay.swift; keep your chosen Interpolators*.swift)
# ----------------------------------------------------------
if [ -f "$SRC/BritStripFaceplate.swift" ]; then
  echo ">> Removing duplicate BritStripFaceplate.swift"
  git rm -f "$SRC/BritStripFaceplate.swift" || rm -f "$SRC/BritStripFaceplate.swift"
fi

# If both Interpolator.swift and Interpolators.swift exist, remove the single-name one
if [ -f "$SRC/Interpolator.swift" ] && ls "$SRC"/Interpolators*.swift >/dev/null 2>&1; then
  echo ">> Removing duplicate Interpolator.swift"
  git rm -f "$SRC/Interpolator.swift" || rm -f "$SRC/Interpolator.swift"
fi

# ----------------------------------------------------------
# 8) Add preflight guard (to prevent regressions)
# ----------------------------------------------------------
mkdir -p "$ROOT/scripts"
cat > "$ROOT/scripts/preflight_guard.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
SRC="${1:-app/Sources}"

fail() { echo "❌ $*"; exit 1; }

IM=$(grep -RIn --include="*.swift" 'enum[[:space:]]\+InterpMode' "$SRC" | wc -l | tr -d ' ')
PP=$(grep -RIn --include="*.swift" 'struct[[:space:]]\+ProcessorParams\b' "$SRC" | wc -l | tr -d ' ')
[ "$IM" = "1" ] || fail "InterpMode must be defined exactly once (found $IM)."
[ "$PP" = "1" ] || fail "ProcessorParams must be defined exactly once (found $PP)."

grep -RIn --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" >/dev/null && \
  fail "Found nested ProcessorParams.InterpMode; use top-level InterpMode instead."

grep -RIn --include="*.swift" '\.buttonStyle\(\.borderedProminent\)' "$SRC" >/dev/null && \
  fail "Found .borderedProminent (macOS 12+); use PMXProminent() instead."

FPL=$(ls "$SRC"/*Faceplate*.swift 2>/dev/null | wc -l | tr -d ' ')
INT=$(ls "$SRC"/Interpolator*.swift "$SRC"/Interpolators*.swift 2>/dev/null | wc -l | tr -d ' ')
[ "$FPL" -le 1 ] || fail "Multiple Faceplate overlays; keep exactly one."
[ "$INT" -le 1 ] || fail "Multiple Interpolator sources; keep exactly one."

echo "✅ Preflight guard passed."
EOF
chmod +x "$ROOT/scripts/preflight_guard.sh"

echo "== More Mojo autofix: done =="
