#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/app/Sources"
PLUGIN_DIR="$ROOT/plugin"
CMK="$PLUGIN_DIR/CMakeLists.txt"

echo "== MoreMojo autofix: start =="

mkdir -p "$SRC" "$PLUGIN_DIR/scripts"

# -------------------------------
# A) Canonical types in SharedTypes.swift
# -------------------------------
cat > "$SRC/SharedTypes.swift" <<'EOF'
import Foundation

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
    public var input:  Float = 0.0
    public var output: Float = 0.0
    public var drive:      Float = 0.55
    public var character:  Float = 0.50
    public var saturation: Float = 0.45
    public var presence:   Float = 0.50
    public var mix:        Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode:       Int = 1
}

public struct MojoEQBand: Codable, Equatable { public var lo: Float; public var hi: Float; public var gain_dB: Float }
public struct MojoEQMatch: Codable, Equatable { public var bands: [MojoEQBand] }
EOF

# -------------------------------
# B) Make ProcessorParams.swift extension-only
# -------------------------------
PP_FILE="$SRC/ProcessorParams.swift"
if [ -f "$PP_FILE" ]; then
  if grep -qE '\bstruct\s+ProcessorParams\b' "$PP_FILE" || grep -qE '\benum\s+InterpMode\b' "$PP_FILE"; then
    mv "$PP_FILE" "$SRC/ProcessorParams_DEPRECATED.swift"
    perl -0777 -pe 's/(\b(struct|enum)\s+(ProcessorParams|InterpMode)\b[^}]+})/\/\/ DEPRECATED duplicate removed\n\/\/ $1\n/s' -i "$SRC/ProcessorParams_DEPRECATED.swift"
  fi
fi
cat > "$SRC/ProcessorParams+Ext.swift" <<'EOF'
import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
}
EOF

# -------------------------------
# C) Replace nested ProcessorParams.InterpMode → InterpMode
# -------------------------------
grep -RIl --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" | while read -r f; do
  perl -0777 -pe 's/ProcessorParams\.InterpMode/InterpMode/g' -i "$f"
done

# -------------------------------
# D) macOS 11: add PMXProminent style & replace .borderedProminent
# -------------------------------
PANEL="$SRC/StealMojoPanel_SwiftOnly.swift"
if [ -f "$PANEL" ] && ! grep -q 'struct PMXProminent' "$PANEL"; then
  perl -0777 -pe 's/import AVFoundation/import AVFoundation\n\nstruct PMXProminent: ButtonStyle {\n    func makeBody(configuration: Configuration) -> some View {\n        configuration.label\n            .padding(.horizontal, 12).padding(.vertical, 6)\n            .background(LinearGradient(colors: [.pink, .purple, .orange], startPoint: .leading, endPoint: .trailing))\n            .foregroundColor(.white)\n            .clipShape(Capsule())\n            .opacity(configuration.isPressed ? 0.8 : 1.0)\n    }\n}\n/s' -i "$PANEL"
fi
grep -RIl --include="*.swift" '\.buttonStyle\(\.borderedProminent\)' "$SRC" | while read -r f; do
  perl -0777 -pe 's/\.buttonStyle\(\.borderedProminent\)/.buttonStyle(PMXProminent())/g' -i "$f"
done

# -------------------------------
# E) Fix MojoWheel call sites & enum cases
# -------------------------------
# enum cases
perl -0777 -pe 's/\.app\b/.appDecides/g; s/\.steal\b/.stealMacro/g' -i "$SRC"/*.swift 2>/dev/null || true
# init signature cleanup (best-effort)
TCV="$SRC/TopConsumerView.swift"
if [ -f "$TCV" ]; then
  perl -0777 -pe 's/MojoWheel\s*\(\s*value:\s*\$wheelVal[^)]*\)/MojoWheel(value: \$wheelVal, rms: CGFloat(engine.rmsOut), mode: macroMode)/s' -i "$TCV"
fi

# -------------------------------
# F) Fix EQ bands in StealMojoSwift.swift
# -------------------------------
SMS="$SRC/StealMojoSwift.swift"
if [ -f "$SMS" ]; then
  perl -0777 -pe 's/bands\.append\(\.init\(/bands.append(MojoEQBand(/g' -i "$SMS"
  # ensure explicit declaration
  grep -q 'var bands: \[MojoEQBand\]' "$SMS" || perl -0777 -pe 's/(compress to bands[^\n]*\n)/$1var bands: [MojoEQBand] = []\n/s' -i "$SMS" || true
fi

# -------------------------------
# G) Remove duplicate files
# -------------------------------
for f in \
  "$SRC/BritStripFaceplate.swift" \
  "$SRC/Interpolator.swift" \
  "$SRC/MoreMojoContainer_old.swift" ; do
  if [ -f "$f" ]; then
    echo "Removing duplicate $f"
    git rm -f "$f" || rm -f "$f"
  fi
done

# -------------------------------
# H) macOS-safe Faceplate overlay (keep only FaceplateOverlay.swift)
#    (If you still need the latest one from earlier, skip)
# -------------------------------
# (No-op here if you already replaced it earlier)

# -------------------------------
# I) Plugin CMake: clean Option-A file (no custom post-build)
# -------------------------------
cat > "$CMK" <<'EOF'
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

find_package(JUCE CONFIG REQUIRED)
set(MOJO_FORMATS "AU;VST3;Standalone" CACHE STRING "Plugin formats to build")

juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS ${MOJO_FORMATS}
    PRODUCT_NAME "More Mojo by Umbo Gumbo"
    COPY_PLUGIN_AFTER_BUILD TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
)

target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

target_compile_definitions(MoreMojoPlugin PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MoreMojoPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)
EOF

# -------------------------------
# J) Preflight guard (so this doesn't regress)
# -------------------------------
cat > "$ROOT/scripts/preflight_guard.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
SRC="${1:-app/Sources}"

fail(){ echo "❌ $*"; exit 1; }

IM=$(grep -RIn --include="*.swift" 'enum[[:space:]]\+InterpMode' "$SRC" | wc -l | tr -d ' ')
PP=$(grep -RIn --include="*.swift" 'struct[[:space:]]\+ProcessorParams\b' "$SRC" | wc -l | tr -d ' ')
[ "$IM" = "1" ] || fail "InterpMode must be defined exactly once (found $IM)."
[ "$PP" = "1" ] || fail "ProcessorParams must be defined exactly once (found $PP)."

grep -RIn --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" >/dev/null && \
  fail "Found nested ProcessorParams.InterpMode; use top-level InterpMode."

grep -RIn --include="*.swift" '\.buttonStyle\(\.borderedProminent\)' "$SRC" >/dev/null && \
  fail "Found .borderedProminent; use PMXProminent()."

FPL=$(ls "$SRC"/*Faceplate*.swift 2>/dev/null | wc -l | tr -d ' ')
INT=$(ls "$SRC"/Interpolator*.swift "$SRC"/Interpolators*.swift 2>/dev/null | wc -l | tr -d ' ')
[ "$FPL" -le 1 ] || fail "Multiple Faceplate overlays; keep exactly one."
[ "$INT" -le 1 ] || fail "Multiple Interpolator sources; keep exactly one."

echo "✅ Preflight guard passed."
EOF
chmod +x "$ROOT/scripts/preflight_guard.sh"

echo "== MoreMojo autofix: done =="
