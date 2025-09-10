import Foundation
import SwiftUI
import Combine

// This file ensures all necessary imports are available to dependent files
// and provides a shared location for import-based fixes

// Re-export the SharedTypes definitions for use in other modules
@_exported import struct SwiftUI.Image
@_exported import struct SwiftUI.Color

// This allows older macOS versions to use newer SwiftUI APIs conditionally
extension View {
    @ViewBuilder
    func compatButtonStyle<S: ButtonStyle>(_ style: S) -> some View {
        if #available(macOS 12.0, *) {
            self.buttonStyle(style)
        } else {
            self // Use default style on older versions
        }
    }
    
    @ViewBuilder
    func compatMaterial(_ style: Material) -> some View {
        if #available(macOS 12.0, *) {
            self.background(style)
        } else {
            self.background(Color.black.opacity(0.5))
        }
    }
}

// Make macOS version checks easier across the codebase
enum PlatformCompatibility {
    static var supportsMaterial: Bool {
        if #available(macOS 12.0, *) {
            return true
        }
        return false
    }
    
    static var supportsBorderedProminent: Bool {
        if #available(macOS 12.0, *) {
            return true
        }
        return false
    }
    
    static func buttonStyle(color: Color = .blue) -> some ButtonStyle {
        if #available(macOS 12.0, *) {
            return .borderedProminent
        } else {
            return PMXProminent()
        }
    }
}
