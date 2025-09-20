import SwiftUI

struct HybridMojoKnob: View {
    @Binding var value: Double
    var rms: CGFloat = 0
    
    // Controls for design experimentation
    var knobDepth: Double = 0.15      // Controls how "3D" the knob appears
    var glowIntensity: Double = 0.7    // Controls the edge glow intensity
    var glowRadius: Double = 15        // Controls the edge glow radius
    var reflectionOpacity: Double = 0.4 // Controls the top reflection highlight
    var metalShine: Double = 0.8       // Controls the shine on the metal
    var bevelSize: Double = 0.08       // Size of the beveled edge
    
    // Color adjustments
    var startHue: Double = 0.6         // Start color (dark/blue) - 0.6 is blue
    var endHue: Double = 0.1           // End color (light/yellow) - 0.1 is yellow/orange
    
    var body: some View {
        GeometryReader { geo in
            let size = min(geo.size.width, geo.size.height)
            
            ZStack {
                // Base layer shadow for depth
                Circle()
                    .fill(Color.black.opacity(0.5))
                    .frame(width: size * 1.05, height: size * 1.05)
                    .blur(radius: 8)
                    .offset(x: 3, y: 3)
                
                // Outer casing
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color(white: 0.25),
                                Color(white: 0.15)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .frame(width: size, height: size)
                    // Inner shadow for recessed knob effect
                    .overlay(
                        Circle()
                            .stroke(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color.black.opacity(0.7),
                                        Color.gray.opacity(0.2),
                                        Color.white.opacity(0.2),
                                        Color.black.opacity(0.7)
                                    ]),
                                    startPoint: .topLeading,
                                    endPoint: .bottomTrailing
                                ),
                                lineWidth: size * CGFloat(bevelSize)
                            )
                    )
                    .shadow(color: Color.black.opacity(0.6), radius: 5, x: 0, y: 0)
                
                // Main brushed metal surface with bevel
                Circle()
                    .fill(
                        RadialGradient(
                            gradient: Gradient(colors: [
                                Color(white: 0.9),
                                Color(white: 0.78)
                            ]),
                            center: .center,
                            startRadius: 0,
                            endRadius: size/2
                        )
                    )
                    .frame(width: size * 0.85, height: size * 0.85)
                    
                    // Metallic bevel edge
                    .overlay(
                        Circle()
                            .strokeBorder(
                                AngularGradient(
                                    gradient: Gradient(stops: [
                                        .init(color: Color.white.opacity(0.99), location: 0.0),
                                        .init(color: Color.gray.opacity(0.3), location: 0.25),
                                        .init(color: Color.black.opacity(0.4), location: 0.5),
                                        .init(color: Color.gray.opacity(0.3), location: 0.75),
                                        .init(color: Color.white.opacity(0.8), location: 1.0)
                                    ]),
                                    center: .center
                                ),
                                lineWidth: size * CGFloat(bevelSize)
                            )
                    )
                    
                    // Concentric brushed texture
                    .overlay(
                        ZStack {
                            // Circular brush lines
                            ForEach(0..<18) { i in
                                Circle()
                                    .stroke(
                                        Color.gray.opacity(Double.random(in: 0.02...0.1)),
                                        lineWidth: 0.5
                                    )
                                    .frame(width: size * 0.8 * CGFloat(i) / 18.0)
                            }
                            
                            // Radial brush lines
                            ForEach(0..<40) { i in
                                Rectangle()
                                    .fill(Color.gray.opacity(Double.random(in: 0.02...0.06)))
                                    .frame(width: 0.5, height: size * 0.4)
                                    .offset(y: size * 0.2)
                                    .rotationEffect(.degrees(Double(i) * 9))
                            }
                        }
                    )
                
                // 3D depth inner shadow
                Circle()
                    .fill(
                        RadialGradient(
                            gradient: Gradient(colors: [
                                Color.clear,
                                Color.black.opacity(knobDepth * 0.5)
                            ]),
                            center: .center,
                            startRadius: size * 0.2,
                            endRadius: size * 0.425
                        )
                    )
                    .blendMode(.multiply)
                    .frame(width: size * 0.85, height: size * 0.85)
                
                // Color glow around the knob (270 degree sweep from dark to light)
                Arc(startAngle: .degrees(135), endAngle: .degrees(135 + 270 * value), clockwise: true)
                    .stroke(
                        // Use Angular gradient for the sweeping color effect
                        AngularGradient(
                            gradient: Gradient(stops: [
                                .init(color: Color(hue: startHue, saturation: 0.8, brightness: 0.6).opacity(0), location: 0),
                                .init(color: Color(hue: startHue, saturation: 0.8, brightness: 0.6), location: 0.1),
                                .init(color: Color(hue: mix(startHue, endHue, 0.25), saturation: 0.8, brightness: 0.7), location: 0.25),
                                .init(color: Color(hue: mix(startHue, endHue, 0.5), saturation: 0.8, brightness: 0.8), location: 0.5),
                                .init(color: Color(hue: mix(startHue, endHue, 0.75), saturation: 0.8, brightness: 0.9), location: 0.75),
                                .init(color: Color(hue: endHue, saturation: 0.7, brightness: 1.0), location: 1.0),
                            ]),
                            center: .center,
                            angle: .degrees(135)
                        ),
                        style: StrokeStyle(lineWidth: size * 0.05, lineCap: .round)
                    )
                    .rotationEffect(.degrees(-135))
                    .frame(width: size * 1.05, height: size * 1.05)
                    .shadow(color: getGlowColor().opacity(glowIntensity), radius: glowRadius, x: 0, y: 0)
                    .opacity(0.8)
                
                // Top highlight for 3D effect
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color.white.opacity(reflectionOpacity),
                                Color.white.opacity(0.0)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .center
                        )
                    )
                    .frame(width: size * 0.7, height: size * 0.7)
                    .offset(x: -size * 0.05, y: -size * 0.05)
                
                // Side glare highlight
                Capsule()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color.white.opacity(metalShine * 0.7),
                                Color.white.opacity(0.0)
                            ]),
                            startPoint: .leading,
                            endPoint: .trailing
                        )
                    )
                    .frame(width: size * 0.08, height: size * 0.4)
                    .offset(x: size * 0.28, y: 0)
                    .blendMode(.screen)
                
                // Center text with embossed/embedded look
                ZStack {
                    // Shadow layer first (creates the embedded effect)
                    VStack(spacing: 0) {
                        Text("MORE")
                            .font(.system(size: size*0.11, weight: .heavy, design: .rounded))
                            .foregroundColor(Color.black.opacity(0.5))
                            .offset(x: 1.5, y: 1.5)
                        
                        Text("MOJO")
                            .font(.system(size: size*0.14, weight: .heavy, design: .rounded))
                            .foregroundColor(Color.black.opacity(0.5))
                            .offset(x: 1.5, y: 1.5)
                    }
                    
                    // Highlight layer (creates the raised edge effect)
                    VStack(spacing: 0) {
                        Text("MORE")
                            .font(.system(size: size*0.11, weight: .heavy, design: .rounded))
                            .foregroundColor(Color.white.opacity(0.3))
                            .offset(x: -0.5, y: -0.5)
                        
                        Text("MOJO")
                            .font(.system(size: size*0.14, weight: .heavy, design: .rounded))
                            .foregroundColor(Color.white.opacity(0.3))
                            .offset(x: -0.5, y: -0.5)
                    }
                    
                    // Main text layer with more subtle coloring for embedded look
                    VStack(spacing: 0) {
                        // MORE - first line
                        Text("MORE")
                            .font(.system(size: size*0.11, weight: .heavy, design: .rounded))
                            .foregroundStyle(
                                // Lighter, semi-transparent gradient
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color(hue: mix(startHue, endHue, 0.2), saturation: 0.6, brightness: 1.0).opacity(0.7),
                                        Color(hue: mix(startHue, endHue, 0.5), saturation: 0.5, brightness: 1.0).opacity(0.7),
                                        Color(hue: mix(startHue, endHue, 0.8), saturation: 0.6, brightness: 1.0).opacity(0.7)
                                    ]),
                                    startPoint: .leading,
                                    endPoint: .trailing
                                )
                            )
                            .blendMode(.softLight) // Helps with the embossed look
                        
                        // MOJO - second line
                        Text("MOJO")
                            .font(.system(size: size*0.14, weight: .heavy, design: .rounded))
                            .foregroundStyle(
                                // Lighter, semi-transparent gradient
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color(hue: mix(startHue, endHue, 0.8), saturation: 0.6, brightness: 1.0).opacity(0.7),
                                        Color(hue: mix(startHue, endHue, 0.5), saturation: 0.5, brightness: 1.0).opacity(0.7),
                                        Color(hue: mix(startHue, endHue, 0.2), saturation: 0.6, brightness: 1.0).opacity(0.7)
                                    ]),
                                    startPoint: .leading,
                                    endPoint: .trailing
                                )
                            )
                            .blendMode(.softLight) // Helps with the embossed look
                    }
                }
                .offset(y: size * 0.01) // Slight offset for visual balance
                .mask(
                    // This slightly blurs the edges of the text to help it blend with the metal
                    VStack(spacing: 0) {
                        Text("MORE")
                            .font(.system(size: size*0.11, weight: .heavy, design: .rounded))
                        Text("MOJO")
                            .font(.system(size: size*0.14, weight: .heavy, design: .rounded))
                    }
                    .blur(radius: 0.3)
                )
            }
            .scaleEffect(1 + 0.02 * min(1, rms*2.5)) // Slight pulsation with audio
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { gesture in
                        let center = CGPoint(x: geo.size.width/2, y: geo.size.height/2)
                        let location = gesture.location
                        let angle = atan2(location.y - center.y, location.x - center.x)
                        
                        // Map from angle to value (taking into account our 270 degree range)
                        // We're using 135 to -135 degrees (270 degree span)
                        var angleInDegrees = (angle * 180 / .pi)
                        
                        // Convert to 0-360 range
                        if angleInDegrees < 0 {
                            angleInDegrees += 360
                        }
                        
                        // Check if within our limited range (135 to 135+270)
                        if angleInDegrees >= 135 && angleInDegrees <= 135 + 270 {
                            // Map to 0-1 value
                            value = (angleInDegrees - 135) / 270
                        }
                        else if angleInDegrees >= 0 && angleInDegrees < 135 {
                            // Handle the wrap-around case
                            angleInDegrees += 360
                            if angleInDegrees >= 135 && angleInDegrees <= 135 + 270 {
                                value = (angleInDegrees - 135) / 270
                            }
                        }
                    }
            )
        }
    }
    
    // Helper to interpolate between two hue values
    private func mix(_ h1: Double, _ h2: Double, _ t: Double) -> Double {
        // Interpolate along the shortest path
        let diff = h2 - h1
        if abs(diff) <= 0.5 {
            return h1 + diff * t
        } else {
            // Go the other way around the color wheel
            let adjustedDiff = diff > 0 ? diff - 1 : diff + 1
            var result = h1 + adjustedDiff * t
            if result < 0 { result += 1 }
            if result > 1 { result -= 1 }
            return result
        }
    }
    
    // Get a color for the glow based on the current value
    private func getGlowColor() -> Color {
        let hue = mix(startHue, endHue, value)
        return Color(hue: hue, saturation: 0.8, brightness: 0.9)
    }
}

// Custom Arc shape to create partial circle for color indicator
struct Arc: Shape {
    var startAngle: Angle
    var endAngle: Angle
    var clockwise: Bool
    
    func path(in rect: CGRect) -> Path {
        var path = Path()
        path.addArc(center: CGPoint(x: rect.midX, y: rect.midY),
                    radius: rect.width / 2,
                    startAngle: startAngle,
                    endAngle: endAngle,
                    clockwise: clockwise)
        return path
    }
}

// Preview
struct HybridMojoKnob_Previews: PreviewProvider {
    static var previews: some View {
        HybridMojoKnob(value: .constant(0.5))
            .frame(width: 300, height: 300)
            .padding()
            .background(Color.black)
            .previewLayout(.sizeThatFits)
    }
}
