import SwiftUI

/// Standard round knob control (original design)
struct PRRoundKnob: View {
    @Binding var value: Double
    var label: String = "KNOB"
    var onChange: (() -> Void)? = nil
    
    // Frame images and configuration
    private let frameCount = 64
    
    var body: some View {
        VStack(spacing: 6) {
            // Knob visualization
            ZStack {
                // Base circle
                Circle()
                    .fill(LinearGradient(
                        gradient: Gradient(colors: [
                            Color(white: 0.3),
                            Color(white: 0.2)
                        ]),
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    ))
                    .shadow(color: .black.opacity(0.5), radius: 2, x: 1, y: 1)
                
                // Inner knob
                Circle()
                    .fill(LinearGradient(
                        gradient: Gradient(colors: [
                            Color(white: 0.75),
                            Color(white: 0.6)
                        ]),
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    ))
                    .frame(width: 100, height: 100)
                
                // Indicator line
                Rectangle()
                    .fill(Color.black)
                    .frame(width: 2, height: 40)
                    .offset(y: -25)
                    .rotationEffect(Angle(degrees: value * 270 - 135))
            }
            .frame(width: 128, height: 128)
            .gesture(DragGesture(minimumDistance: 0).onChanged { g in
                value = (value - Double(g.translation.height/140)).clamped01
                onChange?()
            })
            
            Text(label.uppercased()).font(.system(size: 10, weight: .semibold, design: .rounded))
        }
    }
}

private extension Double {
    var clamped01: Double { min(1, max(0, self)) }
}
