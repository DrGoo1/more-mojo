#!/usr/bin/env python3
"""Swift agent to fix common Swift compilation issues."""

import re
from pathlib import Path

from .base_agent import BaseAgent
from .registry import register_agent

@register_agent
class SwiftAgent(BaseAgent):
    """Unify SharedTypes / fix wheel cases / macOS 11 style / EQ bands."""
    
    KEYS = [
        "cannot find type 'ProcessorParams' in scope",
        "type 'MojoMacroMode' has no member 'app'",
        "buttonStyle(.borderedProminent)",
        "cannot infer contextual base in reference to member 'init'",
        "could not build Objective-C module 'CoreData'",
        "'Foundation/Foundation.h' file not found",
        "SwiftMojoAnalyzer", 
        "MojoEQMatch",
    ]
    
    SHARED = """import Foundation
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
    public var input: Float = 0.0
    public var output: Float = 0.0
    public var drive: Float = 0.55
    public var character: Float = 0.50
    public var saturation: Float = 0.45
    public var presence: Float = 0.50
    public var warmth: Float = 0.60
    public var mix: Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode: Int = 1
    
    public init() {}
}

public struct MojoEQBand: Codable, Equatable { 
    public var lo: Float
    public var hi: Float
    public var gain_dB: Float
    
    public init(lo: Float, hi: Float, gain_dB: Float) {
        self.lo = lo
        self.hi = hi
        self.gain_dB = gain_dB
    }
}

public struct MojoEQMatch: Codable, Equatable { 
    public var bands: [MojoEQBand] 
    
    public init(bands: [MojoEQBand]) {
        self.bands = bands
    }
}

public struct MojoRecommendation: Codable, Equatable {
    public var interpMode: String
    public var drive: Float
    public var saturation: Float
    public var character: Float
    public var presence: Float
    public var mix: Float
    public var output: Float
    
    public init(interpMode: String, drive: Float, saturation: Float, character: Float, presence: Float, mix: Float, output: Float) {
        self.interpMode = interpMode
        self.drive = drive
        self.saturation = saturation
        self.character = character
        self.presence = presence
        self.mix = mix
        self.output = output
    }
}
"""
    
    EXT = """import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
    public var warmthNormalized: Float { warmth }
}
"""
    
    PMX = """
// macOS 11-safe prominent button style
struct PMXProminent: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .padding(.horizontal, 12).padding(.vertical, 6)
            .background(LinearGradient(colors: [.pink, .purple, .orange], startPoint: .leading, endPoint: .trailing))
            .foregroundColor(.white)
            .clipShape(Capsule())
            .opacity(configuration.isPressed ? 0.8 : 1.0)
    }
}
"""
    
    @classmethod
    def name(cls) -> str:
        return "SwiftAgent"
    
    @classmethod
    def description(cls) -> str:
        return "Fixes common Swift compilation issues, unifies shared types, and ensures macOS 11 compatibility"
    
    def wants(self, app_logs: str, plugin_logs: str) -> bool:
        """Determine if the agent wants to run based on log content."""
        return any(k in app_logs for k in self.KEYS)
    
    def run(self) -> bool:
        """Fix Swift compilation issues."""
        changed = False
        
        # Ensure SharedTypes.swift exists with correct content
        changed |= self.write_file(self.src / "SharedTypes.swift", self.SHARED)
        
        # Remove duplicate ProcessorParams.swift to prevent type duplication
        pp = self.src / "ProcessorParams.swift"
        if pp.exists():
            self.logger.info("Removing duplicate ProcessorParams.swift (now in SharedTypes.swift)")
            try:
                # Create backup
                backup = self.src / "ProcessorParams_DEPRECATED.swift"
                backup.write_text("// DEPRECATED\n" + pp.read_text(errors="ignore"))
                pp.unlink()
                changed = True
                self.logger.info(f"Backed up to {backup} and removed original")
            except Exception as e:
                self.logger.error(f"Could not remove ProcessorParams.swift: {e}")
        
        # Add extension file
        changed |= self.write_file(self.src / "ProcessorParams+Ext.swift", self.EXT)
        
        # Normalize nested refs & wheel enums
        for f in self.src.glob("*.swift"):
            if f.name == "SharedTypes.swift":
                continue
                
            text = f.read_text(errors="ignore")
            new_text = text
            
            # Fix common references
            new_text = re.sub(r'ProcessorParams\.InterpMode', 'InterpMode', new_text)
            new_text = re.sub(r'\.app\b', '.appDecides', new_text)
            new_text = re.sub(r'\.steal\b', '.stealMacro', new_text)
            
            # Fix EQ bands
            new_text = new_text.replace("bands.append(.init(", "bands.append(MojoEQBand(")
            if "bands.append(MojoEQBand(" in new_text and "var bands: [MojoEQBand]" not in new_text:
                new_text = re.sub(r'(bands\.append\(MojoEQBand\()', r'var bands: [MojoEQBand] = []\n\1', new_text, count=1)
            
            # Fix macOS 11 button style
            new_text = new_text.replace(".buttonStyle(.borderedProminent)", ".buttonStyle(PMXProminent())")
            if ".buttonStyle(PMXProminent())" in new_text and "struct PMXProminent" not in new_text:
                if "import SwiftUI" in new_text:
                    new_text = new_text.replace("import SwiftUI", "import SwiftUI\n" + self.PMX)
                else:
                    new_text = "import SwiftUI\n" + self.PMX + "\n" + new_text
            
            if new_text != text:
                f.write_text(new_text)
                self.logger.info(f"Updated {f.name}")
                changed = True
        
        return changed
