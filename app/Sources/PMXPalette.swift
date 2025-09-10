import SwiftUI

/// Peter-Max inspired color palette
enum PMXPalette {
    // Primary colors
    static let mojoRed = Color(red: 0.95, green: 0.2, blue: 0.2)
    static let mojoBlue = Color(red: 0.2, green: 0.4, blue: 0.95)
    static let mojoYellow = Color(red: 0.95, green: 0.85, blue: 0.2)
    static let mojoGreen = Color(red: 0.2, green: 0.85, blue: 0.4)
    static let mojoPurple = Color(red: 0.7, green: 0.3, blue: 0.9)
    
    // Background
    static let darkBackground = Color(red: 0.12, green: 0.12, blue: 0.15)
    static let mediumBackground = Color(red: 0.16, green: 0.16, blue: 0.20)
    static let lightBackground = Color(red: 0.20, green: 0.20, blue: 0.25)
    
    // Accents
    static let highlight = Color.white
    static let shadow = Color.black.opacity(0.6)
    
    // Gradients
    static let redGradient = LinearGradient(
        gradient: Gradient(colors: [mojoRed, mojoRed.opacity(0.7)]),
        startPoint: .top,
        endPoint: .bottom
    )
    
    static let blueGradient = LinearGradient(
        gradient: Gradient(colors: [mojoBlue, mojoBlue.opacity(0.7)]),
        startPoint: .top,
        endPoint: .bottom
    )
    
    static let rainbowGradient = LinearGradient(
        gradient: Gradient(colors: [mojoRed, mojoYellow, mojoGreen, mojoBlue, mojoPurple]),
        startPoint: .leading,
        endPoint: .trailing
    )
    
    static let psychedelicGradient = AngularGradient(
        gradient: Gradient(colors: [
            mojoRed, mojoYellow, mojoGreen, 
            mojoBlue, mojoPurple, mojoRed
        ]),
        center: .center
    )
}

extension View {
    func pmxShadow() -> some View {
        self.shadow(color: PMXPalette.shadow, radius: 5, x: 2, y: 2)
    }
    
    func pmxBorder() -> some View {
        self.overlay(
            RoundedRectangle(cornerRadius: 10)
                .stroke(PMXPalette.highlight, lineWidth: 2)
        )
    }
}
