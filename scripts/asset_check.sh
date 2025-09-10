#!/usr/bin/env bash
set -eo pipefail

echo "===== Checking required assets ====="

# Get directory containing this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_ROOT/app/Sources"
RESOURCES_DIR="$PROJECT_ROOT/app/Resources"

# Create resources directory if it doesn't exist
mkdir -p "$RESOURCES_DIR"

# Check for faceplate layout JSON
if [ ! -f "$RESOURCES_DIR/faceplate_layout.json" ]; then
  echo "Creating minimal faceplate_layout.json"
  cat > "$RESOURCES_DIR/faceplate_layout.json" << 'EOF'
{
  "faceplateAssetName": "heritage-audio-britStrip-GUI-01",
  "baseWidth": 2000,
  "baseHeight": 750,
  "rects": {
    "input": {"x": 100, "y": 100, "w": 80, "h": 80},
    "drive": {"x": 300, "y": 100, "w": 80, "h": 80},
    "character": {"x": 500, "y": 100, "w": 80, "h": 80},
    "saturation": {"x": 700, "y": 100, "w": 80, "h": 80},
    "presence": {"x": 900, "y": 100, "w": 80, "h": 80},
    "mix": {"x": 1100, "y": 100, "w": 80, "h": 80},
    "output": {"x": 1300, "y": 100, "w": 80, "h": 80}
  }
}
EOF
fi

# Check for default faceplate image
if [ ! -f "$RESOURCES_DIR/heritage-audio-britStrip-GUI-01.jpg" ]; then
  echo "Creating minimal faceplate image"
  # Create a simple black rectangle as a placeholder
  if command -v convert &> /dev/null; then
    convert -size 2000x750 xc:black "$RESOURCES_DIR/heritage-audio-britStrip-GUI-01.jpg"
  else
    # If ImageMagick is not available, create a tiny black JPG
    cat > "$RESOURCES_DIR/heritage-audio-britStrip-GUI-01.jpg" << 'EOF'
����JFIF��C
    



  
��C�		��2�"�������������������������������������	
������}!1AQa"q2���#B��R��$3br�	
%&'()*456789:CDEFGHIJSTUVWXYZcdefghijstuvwxyz���������������������������������������������������������������������������	
������w!1AQaq"2�B����	#3R�br�
$4�%�&'()*56789:CDEFGHIJSTUVWXYZcdefghijstuvwxyz��������������������������������������������������������������������������?��(��?
EOF
    echo "WARNING: Created minimal image placeholder"
  fi
fi

# Check for MojoEQMatch type
if ! grep -q "MojoEQBand" "$SRC_DIR/SwiftMojoAnalyzer.swift" 2>/dev/null; then
  echo "Creating SwiftMojoAnalyzer with EQ match types"
  cat > "$SRC_DIR/SwiftMojoAnalyzer.swift" << 'EOF'
import Foundation
import AVFoundation

/// Basic implementation of the SwiftMojoAnalyzer
enum SwiftMojoAnalyzer {
    
    struct MojoRecommendation {
        var drive: Float = 0.5
        var character: Float = 0.5
        var saturation: Float = 0.5
        var presence: Float = 0.5
        var mix: Float = 1.0
        var output: Float = 0.0
        var interpMode: String = "liveHB4x"
    }
    
    static func separateHPSS(url: URL) throws -> ([Float], [Float], Double) {
        return (Array(repeating: 0, count: 1024), Array(repeating: 0, count: 1024), 44100.0)
    }
    
    static func features(from harmonic: [Float], sr: Double) -> [String: Double] {
        return ["spectralCentroid": 1200.0, "spectralSpread": 800.0]
    }
    
    static func recommend(from features: [String: Double], part: String) -> MojoRecommendation {
        var rec = MojoRecommendation()
        
        switch part.lowercased() {
        case "bass":
            rec.drive = 0.6
            rec.character = 0.7
        case "drums":
            rec.drive = 0.5
            rec.character = 0.4
        case "vocal":
            rec.drive = 0.4
            rec.character = 0.6
        default:
            break
        }
        
        return rec
    }
    
    static func eqMatchBands(srcURL: URL, refURL: URL) -> MojoEQMatch? {
        return MojoEQMatch(bands: [
            MojoEQBand(freq: 100, gain: 2.0, q: 1.0),
            MojoEQBand(freq: 500, gain: -1.5, q: 1.0)
        ])
    }
}

struct MojoEQBand {
    var freq: Float
    var gain: Float
    var q: Float
}

struct MojoEQMatch {
    var bands: [MojoEQBand]
}
EOF
fi

# Check for InterpMode enum
if [ ! -f "$SRC_DIR/InterpMode.swift" ]; then
  echo "Creating InterpMode.swift"
  cat > "$SRC_DIR/InterpMode.swift" << 'EOF'
import Foundation

public enum InterpMode: Int, Codable, CaseIterable {
    case liveHB4x, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
}
EOF
fi

# Check for ProcessorParams struct
if ! grep -q "ProcessorParams" "$SRC_DIR/ProcessorParams.swift" 2>/dev/null; then
  echo "Creating ProcessorParams.swift"
  cat > "$SRC_DIR/ProcessorParams.swift" << 'EOF'
import Foundation

struct ProcessorParams {
    var drive: Float = 0.5
    var character: Float = 0.5
    var saturation: Float = 0.5
    var presence: Float = 0.5
    var mix: Float = 1.0
    var output: Float = 0.0
    var interpMode: InterpMode = .liveHB4x
}
EOF
fi

echo "===== Asset check complete ====="
