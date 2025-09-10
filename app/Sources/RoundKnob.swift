import SwiftUI

struct RoundKnob: View {
    @Binding var value: Double
    var label: String
    var range: ClosedRange<Double> = 0.0...1.0
    var size: CGFloat = 80
    var color: Color = PMXPalette.mojoBlue
    var onChange: ((Double) -> Void)? = nil
    
    @State private var isDragging = false
    @State private var startY: CGFloat = 0
    @State private var startValue: Double = 0
    
    private let knobTicks = 16
    
    var body: some View {
        VStack(spacing: 6) {
            ZStack {
                // Knob background
                Circle()
                    .fill(
                        RadialGradient(
                            gradient: Gradient(colors: [
                                Color.black.opacity(0.7),
                                Color.black.opacity(0.9)
                            ]),
                            center: .center,
                            startRadius: 0,
                            endRadius: size / 2
                        )
                    )
                    .frame(width: size, height: size)
                    .shadow(color: .black.opacity(0.5), radius: 2)
                
                // Indicator ticks
                ForEach(0..<knobTicks, id: \.self) { i in
                    let angle = Double(i) * (360 / Double(knobTicks))
                    let isHighlighted = isTickHighlighted(angle: angle)
                    
                    Rectangle()
                        .fill(isHighlighted ? color : Color.gray.opacity(0.5))
                        .frame(width: 2, height: size * 0.15)
                        .offset(y: -size * 0.35)
                        .rotationEffect(Angle(degrees: angle))
                }
                
                // Knob pointer
                Rectangle()
                    .fill(color)
                    .frame(width: 4, height: size * 0.4)
                    .offset(y: -size * 0.2)
                    .rotationEffect(Angle(degrees: rotationFromValue()))
                
                // Central dot
                Circle()
                    .fill(Color.gray.opacity(0.3))
                    .frame(width: size * 0.2, height: size * 0.2)
                    .overlay(
                        Circle()
                            .fill(color.opacity(isDragging ? 0.7 : 0.4))
                            .frame(width: size * 0.15, height: size * 0.15)
                    )
                    .shadow(color: color.opacity(0.4), radius: isDragging ? 5 : 2)
            }
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { gesture in
                        if !isDragging {
                            isDragging = true
                            startY = gesture.location.y
                            startValue = value
                        }
                        
                        let delta = startY - gesture.location.y
                        let sensitivity: Double = 0.005
                        
                        // Calculate new value based on vertical drag
                        let newValue = (startValue + Double(delta) * sensitivity)
                            .clamped(to: range)
                        
                        if newValue != value {
                            value = newValue
                            onChange?(value)
                        }
                    }
                    .onEnded { _ in
                        isDragging = false
                    }
            )
            
            // Label and value
            VStack(spacing: 2) {
                Text(label)
                    .font(.caption)
                    .fontWeight(.medium)
                    .foregroundColor(.white)
                
                // Value as percentage
                Text(String(format: "%.0", value * 100) + "%")
                    .font(.caption2)
                    .foregroundColor(color)
            }
        }
    }
    
    // Determine whether a tick should be highlighted based on the angle
    private func isTickHighlighted(angle: Double) -> Bool {
        let knobRotation = rotationFromValue()
        let diff = abs((angle - knobRotation).truncatingRemainder(dividingBy: 360))
        return diff < 30 || diff > 330
    }
    
    // Convert the current value to rotation degrees
    private func rotationFromValue() -> Double {
        let normalized = (value - range.lowerBound) / (range.upperBound - range.lowerBound)
        return normalized * 270 - 135 // Map from 0-1 to -135 to 135 degrees
    }
}

// Helper extension for clamping values
extension Double {
    func clamped(to range: ClosedRange<Double>) -> Double {
        return min(max(self, range.lowerBound), range.upperBound)
    }
}
