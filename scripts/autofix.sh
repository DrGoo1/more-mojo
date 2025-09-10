#!/usr/bin/env bash
set -euo pipefail

SRC="app/Sources"
mkdir -p "$SRC"

# SharedTypes.swift – create if not present
if [ ! -f "$SRC/SharedTypes.swift" ]; then
  cat > "$SRC/SharedTypes.swift" <<'EOF'
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
EOF
fi

# Normalize nested InterpMode references
grep -RIl --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" | xargs -I{} perl -0777 -pe 's/ProcessorParams\.InterpMode/InterpMode/g' -i {}

# macOS 11 UI: replace .borderedProminent with custom style marker
grep -RIl --include="*.swift" '\.buttonStyle\(\.borderedProminent\)' "$SRC" | xargs -I{} perl -0777 -pe 's/\.buttonStyle\(\.borderedProminent\)/.buttonStyle(PMXProminent())/g' -i {}

# Remove duplicate struct/class declarations commonly in ContentView
perl -0777 -pe 's/\n(\s*struct\s+AppState\s*:[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC/ContentView.swift" 2>/dev/null || true
perl -0777 -pe 's/\n(\s*struct\s+MoreMojoContainer[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC/ContentView.swift" 2>/dev/null || true
perl -0777 -pe 's/\n(\s*struct\s+AudioProcessingView[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC/ContentView.swift" 2>/dev/null || true

# Remove legacy MoreMojoContainer_old.swift if present
[ -f "$SRC/MoreMojoContainer_old.swift" ] && git rm -f "$SRC/MoreMojoContainer_old.swift"

# Fix common string interpolation mistake
grep -RIl --include="*.swift" 'EQ Match Bands: \(' "$SRC" | xargs -I{} perl -0777 -pe 's/EQ Match Bands:\s*\(([^)]+)\)/"EQ Match Bands: \\($1\\)"/g' -i {} || true

# Ensure AudioEngine is ObservableObject with @Published basics (light-touch check)
if grep -RIl --include="*.swift" 'final\s+class\s+AudioEngine\b' "$SRC" >/dev/null 2>&1; then
  perl -0777 -pe 's/final\s+class\s+AudioEngine\s*:/final class AudioEngine: ObservableObject, /s' -i "$SRC/AudioEngine.swift" || true
  for prop in isPlaying duration currentTime rmsOut spectrum; do
    grep -q "@Published" "$SRC/AudioEngine.swift" || perl -0777 -pe 's/(class\s+AudioEngine[^{]+{)/$1\n    @Published var isPlaying: Bool = false\n    @Published var duration: TimeInterval = 0\n    @Published var currentTime: TimeInterval = 0\n    @Published var rmsOut: Float = 0\n    @Published var spectrum: [Float] = Array(repeating: 0, count: 128)\n/s' -i "$SRC/AudioEngine.swift"
  done
fi
