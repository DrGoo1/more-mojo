#!/usr/bin/env bash
# Robust, idempotent autofix – never hard-fail on non-critical steps.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/app/Sources"
PLUGIN_DIR="$ROOT/plugin"

echo "== More Mojo autofix: start =="
echo "ROOT=$ROOT  SRC=$SRC  PLUGIN_DIR=$PLUGIN_DIR"

changed=0

# --- A) Canonical SharedTypes.swift ------------------------------------------------
mkdir -p "$SRC"
cat > "$SRC/SharedTypes.swift.new" <<'EOF2'
import Foundation
public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
    public var id: Int { rawValue }
}
public struct ProcessorParams: Codable, Equatable {
    public var input: Float = 0.0
    public var output: Float = 0.0
    public var drive: Float = 0.55
    public var character: Float = 0.50
    public var saturation: Float = 0.45
    public var presence: Float = 0.50
    public var mix: Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode: Int = 1
}
public struct MojoEQBand: Codable, Equatable { public var lo: Float; public var hi: Float; public var gain_dB: Float }
public struct MojoEQMatch: Codable, Equatable { public var bands: [MojoEQBand] }
EOF2

if ! cmp -s "$SRC/SharedTypes.swift.new" "$SRC/SharedTypes.swift" 2>/dev/null; then
  mv -f "$SRC/SharedTypes.swift.new" "$SRC/SharedTypes.swift"
  echo ">> Wrote canonical SharedTypes.swift"
  changed=1
else
  rm -f "$SRC/SharedTypes.swift.new"
fi

# --- B) Make ProcessorParams extension-only ---------------------------------------
PP_FILE="$SRC/ProcessorParams.swift"
if [ -f "$PP_FILE" ] && grep -Eq '\b(struct|enum)\s+(ProcessorParams|InterpMode)\b' "$PP_FILE"; then
  echo ">> Quarantining duplicate ProcessorParams/InterpMode in ProcessorParams.swift"
  mv -f "$PP_FILE" "$SRC/ProcessorParams_DEPRECATED.swift" || true
  # comment out legacy declarations to avoid duplicate symbols
  perl -0777 -pe 's/(\b(struct|enum)\s+(ProcessorParams|InterpMode)\b[^}]+})/\/\/ DEPRECATED duplicate removed\n\/\/ $1\n/s' -i "$SRC/ProcessorParams_DEPRECATED.swift" || true
  # create a minimal extension file
  cat > "$SRC/ProcessorParams+Ext.swift" <<'EOF2'
import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
}
EOF2
  changed=1
fi

# ensure extension file exists
[ -f "$SRC/ProcessorParams+Ext.swift" ] || cat > "$SRC/ProcessorParams+Ext.swift" <<'EOF2'
import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
}
EOF2

# --- C) Normalize nested references ------------------------------------------------
files=$(grep -RIl --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" || true)
if [ -n "${files}" ]; then
  echo ">> Normalizing nested ProcessorParams.InterpMode -> InterpMode"
  while IFS= read -r f; do
    perl -0777 -pe 's/ProcessorParams\.InterpMode/InterpMode/g' -i "$f" || true
    changed=1
  done <<< "$files"
fi

# --- D) EQ bands fix in StealMojoSwift.swift --------------------------------------
SMS="$SRC/StealMojoSwift.swift"
if [ -f "$SMS" ]; then
  if grep -q 'bands\.append(.init' "$SMS"; then
    echo ">> Fixing bands.append(.init...) -> MojoEQBand(...)"
    perl -0777 -pe 's/bands\.append\(\.init\(/bands.append(MojoEQBand(/g' -i "$SMS" || true
    changed=1
  fi
  if ! grep -q 'var bands: \[MojoEQBand\]' "$SMS"; then
    echo ">> Inserting explicit var bands: [MojoEQBand] = []"
    # Insert after a likely comment, else at first append occurrence
    if grep -q 'compress to bands' "$SMS"; then
      perl -0777 -pe 's/(compress to bands[^\n]*\n)/$1var bands: [MojoEQBand] = []\n/s' -i "$SMS" || true
    else
      perl -0777 -pe 's/(bands\.append\(MojoEQBand\()/var bands: [MojoEQBand] = []\n$1/s' -i "$SMS" || true
    fi
    changed=1
  fi
fi

# --- E) Fix MojoWheel enum/call sites ---------------------------------------------
echo ">> Normalizing MojoWheel enum cases and call-sites"
perl -0777 -pe 's/\.app\b/.appDecides/g; s/\.steal\b/.stealMacro/g' -i "$SRC"/*.swift 2>/dev/null || true
TCV="$SRC/TopConsumerView.swift"
if [ -f "$TCV" ] && grep -q 'MojoWheel' "$TCV"; then
  perl -0777 -pe 's/MojoWheel\s*\(\s*value:\s*\$wheelVal[^)]*\)/MojoWheel(value: \$wheelVal, rms: CGFloat(engine.rmsOut), mode: macroMode)/s' -i "$TCV" || true
  changed=1
fi

# --- F) macOS 11 button style -----------------------------------------------------
PANEL="$SRC/StealMojoPanel_SwiftOnly.swift"
if [ -f "$PANEL" ]; then
  if ! grep -q 'struct PMXProminent' "$PANEL"; then
    echo ">> Adding PMXProminent button style"
    perl -0777 -pe 's/import AVFoundation/import AVFoundation\n\nstruct PMXProminent: ButtonStyle {\n    func makeBody(configuration: Configuration) -> some View {\n        configuration.label\n            .padding(.horizontal, 12).padding(.vertical, 6)\n            .background(LinearGradient(colors: [.pink, .purple, .orange], startPoint: .leading, endPoint: .trailing))\n            .foregroundColor(.white)\n            .clipShape(Capsule())\n            .opacity(configuration.isPressed ? 0.8 : 1.0)\n    }\n}\n/s' -i "$PANEL" || true
    changed=1
  fi
  if grep -q '\.buttonStyle\(\.borderedProminent\)' "$PANEL"; then
    echo ">> Replacing .borderedProminent with PMXProminent()"
    perl -0777 -pe 's/\.buttonStyle\(\.borderedProminent\)/.buttonStyle(PMXProminent())/g' -i "$PANEL" || true
    changed=1
  fi
fi

# --- G) Clean plugin CMake (Option A) ---------------------------------------------
mkdir -p "$PLUGIN_DIR"
cat > "$PLUGIN_DIR/CMakeLists.txt.new" <<'EOF2'
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
    IS_SYNTH FALSE)
target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h)
target_compile_definitions(MoreMojoPlugin PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(MoreMojoPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp)
EOF2

if ! cmp -s "$PLUGIN_DIR/CMakeLists.txt.new" "$PLUGIN_DIR/CMakeLists.txt" 2>/dev/null; then
  mv -f "$PLUGIN_DIR/CMakeLists.txt.new" "$PLUGIN_DIR/CMakeLists.txt"
  echo ">> Rewrote plugin/CMakeLists.txt (Option A)"
  changed=1
else
  rm -f "$PLUGIN_DIR/CMakeLists.txt.new"
fi

echo "== Autofix completed. changed=$changed =="
exit 0
