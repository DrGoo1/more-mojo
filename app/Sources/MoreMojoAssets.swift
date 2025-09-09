import SwiftUI

/// Asset definitions for More Mojo app
/// Replace these strings with your actual image asset names
struct MoreMojoAssets {
    // Round knob image sprite strip
    static let roundKnobImage = "RoundKnob"
    static let knobFrameCount = 128
    
    // Chicken-head selector image sprite strip
    static let selectorImage = "ChickenHeadSelector"
    static let selectorFrameCount = 64
    
    // Toggle switch images
    static let toggleOnImage = "ToggleOn"
    static let toggleOffImage = "ToggleOff"
    
    // VU meter background image
    static let vuMeterImage = "VUMeterBackground"
    
    // Button images
    static let buttonImage = "ButtonNormal"
    static let buttonPressedImage = "ButtonPressed"
    
    // LED images
    static let ledOnImage = "LEDOn"
    static let ledOffImage = "LEDOff"
    
    // Background textures
    static let metalPanelTexture = "MetalPanelTexture"
    static let woodPanelTexture = "WoodPanelTexture"
    
    // Logo
    static let mojoLogo = "MojoLogo"
    
    // Placeholders for development (system images)
    static func placeholderImage(for assetName: String) -> Image {
        switch assetName {
        case roundKnobImage:
            return Image(systemName: "dial.fill")
        case selectorImage:
            return Image(systemName: "switch.2")
        case toggleOnImage, toggleOffImage:
            return Image(systemName: "power")
        case vuMeterImage:
            return Image(systemName: "waveform.path")
        case buttonImage, buttonPressedImage:
            return Image(systemName: "square.fill")
        case ledOnImage, ledOffImage:
            return Image(systemName: "circle.fill")
        case mojoLogo:
            return Image(systemName: "music.note")
        default:
            return Image(systemName: "questionmark")
        }
    }
}
