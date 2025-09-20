import SwiftUI

struct MojoWheel3D: View {
    @Binding var value: Double
    var rms: CGFloat = 0
    
    // Controls for design experimentation
    var knobDepth: Double = 0.15      // Controls how "3D" the knob appears
    var glowIntensity: Double = 0.7    // Controls the edge glow intensity
    var glowRadius: Double = 15        // Controls the edge glow radius
    var reflectionOpacity: Double = 0.4 // Controls the top reflection highlight
    
    var body: some View {
        GeometryReader { geo in
            let size = min(geo.size.width, geo.size.height)
            let thickness = size * 0.13
            
            ZStack {
                // Base layer - 3D appearance with gradient and shadow
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(stops: [
                                .init(color: Color(hue: 0.96, saturation: 0.73, brightness: 0.9), location: 0.0),
                                .init(color: Color(hue: 0.04, saturation: 0.90, brightness: 0.8), location: 0.5),
                                .init(color: Color(hue: 0.96, saturation: 0.73, brightness: 0.7), location: 1.0)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    // Inner shadow for 3D depth
                    .overlay(
                        Circle()
                            .stroke(
                                LinearGradient(
                                    gradient: Gradient(stops: [
                                        .init(color: Color.white.opacity(0.7), location: 0.0),
                                        .init(color: Color.white.opacity(0.1), location: 0.5),
                                        .init(color: Color.black.opacity(0.2), location: 1.0)
                                    ]),
                                    startPoint: .topLeading,
                                    endPoint: .bottomTrailing
                                ),
                                lineWidth: 2
                            )
                    )
                    // 3D effect with shadow
                    .shadow(color: Color.black.opacity(0.6), radius: 10, x: 5, y: 5)
                    // Additional depth with inner shadow
                    .overlay(
                        Circle()
                            .fill(
                                RadialGradient(
                                    gradient: Gradient(colors: [
                                        Color.white.opacity(0.0),
                                        Color.black.opacity(knobDepth)
                                    ]),
                                    center: .center,
                                    startRadius: 0,
                                    endRadius: size/2
                                )
                            )
                            .scaleEffect(0.85)
                            .blendMode(.multiply)
                    )
                
                // Edge glow ring
                Circle().strokeBorder(
                    AngularGradient(colors: [
                        Color(red: 1.00, green: 0.31, blue: 0.00),
                        Color(red: 1.00, green: 0.73, blue: 0.00),
                        Color(red: 0.00, green: 0.80, blue: 0.45),
                        Color(red: 0.00, green: 0.62, blue: 0.98),
                        Color(red: 0.54, green: 0.00, blue: 1.00),
                        Color(red: 1.00, green: 0.31, blue: 0.68),
                        Color(red: 1.00, green: 0.31, blue: 0.00)
                    ], center: .center, angle: .degrees(value*360)),
                    lineWidth: thickness
                )
                // Edge glow effect
                .shadow(
                    color: Color(
                        hue: (value * 0.8).truncatingRemainder(dividingBy: 1.0),
                        saturation: 0.9,
                        brightness: 0.9
                    ).opacity(glowIntensity),
                    radius: glowRadius * (1 + min(0.5, rms*1.2)),
                    x: 0,
                    y: 0
                )
                
                // Tick marks
                ForEach(0..<12) { i in
                    Capsule()
                        .fill(Color.white.opacity(0.85))
                        .frame(width: 3, height: thickness*0.6)
                        .offset(y: -size*0.38)
                        .rotationEffect(.degrees(Double(i) * 30))
                        .shadow(color: .black.opacity(0.3), radius: 1, x: 1, y: 1)
                }
                
                // Knob indicator
                Capsule()
                    .fill(Color.white)
                    .frame(width: 4, height: size*0.38)
                    .offset(y: -size*0.19)
                    .rotationEffect(.degrees(value*360))
                    .shadow(color: Color.white.opacity(0.6), radius: 4)
                
                // Top reflection for 3D effect
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
                    .scaleEffect(0.7)
                
                // Center text
                VStack(spacing: 2) {
                    Text("MOJO")
                        .font(.system(size: size*0.14, weight: .heavy, design: .rounded))
                        .foregroundColor(.white)
                    
                    Text("MAX")
                        .font(.system(size: size*0.08, weight: .semibold, design: .rounded))
                        .foregroundColor(.white.opacity(0.9))
                }
                .shadow(color: Color.black.opacity(0.5), radius: 2, x: 1, y: 1)
            }
            .scaleEffect(1 + 0.025 * min(1, rms*3))
            .gesture(DragGesture(minimumDistance: 0).onChanged { g in
                let center = CGPoint(x: geo.size.width/2, y: geo.size.height/2)
                let dx = g.location.x - center.x
                let dy = g.location.y - center.y
                var ang = atan2(dy, dx) * 180 / .pi + 90
                if ang < 0 { ang += 360 }
                let v = min(1, max(0, ang / 360))
                value = v
            })
        }
    }
}

struct ControlPanel: View {
    @State private var value: Double = 0.5
    @State private var rmsLevel: Double = 0.0
    
    // Design adjustment controls
    @State private var knobDepth: Double = 0.15
    @State private var glowIntensity: Double = 0.7
    @State private var glowRadius: Double = 15
    @State private var reflectionOpacity: Double = 0.4
    
    // Animated RMS simulation
    let timer = Timer.publish(every: 0.05, on: .main, in: .common).autoconnect()
    
    var body: some View {
        VStack(spacing: 20) {
            Text("MojoWheel Design Lab")
                .font(.largeTitle)
                .fontWeight(.bold)
                .padding(.top, 20)
            
            // Preview area with background
            ZStack {
                // Psychedelic background
                RadialGradient(
                    gradient: Gradient(colors: [
                        Color(hue: 0.7, saturation: 0.3, brightness: 0.1),
                        Color(hue: 0.8, saturation: 0.2, brightness: 0.05)
                    ]),
                    center: .center,
                    startRadius: 50,
                    endRadius: 300
                )
                .frame(width: 400, height: 400)
                .clipShape(RoundedRectangle(cornerRadius: 20))
                
                // MojoWheel
                MojoWheel3D(
                    value: $value,
                    rms: CGFloat(rmsLevel),
                    knobDepth: knobDepth,
                    glowIntensity: glowIntensity,
                    glowRadius: glowRadius,
                    reflectionOpacity: reflectionOpacity
                )
                .frame(width: 300, height: 300)
            }
            .padding()
            
            // Design controls
            Form {
                Section(header: Text("3D Controls")) {
                    VStack(alignment: .leading) {
                        Text("Knob Depth: \(String(format: "%.2f", knobDepth))")
                        Slider(value: $knobDepth, in: 0...0.3)
                    }
                    
                    VStack(alignment: .leading) {
                        Text("Reflection Opacity: \(String(format: "%.2f", reflectionOpacity))")
                        Slider(value: $reflectionOpacity, in: 0...0.8)
                    }
                }
                
                Section(header: Text("Glow Controls")) {
                    VStack(alignment: .leading) {
                        Text("Glow Intensity: \(String(format: "%.2f", glowIntensity))")
                        Slider(value: $glowIntensity, in: 0...1.0)
                    }
                    
                    VStack(alignment: .leading) {
                        Text("Glow Radius: \(String(format: "%.1f", glowRadius))")
                        Slider(value: $glowRadius, in: 5...30)
                    }
                }
                
                Section(header: Text("Animation Test")) {
                    VStack(alignment: .leading) {
                        Text("Audio Level (RMS): \(String(format: "%.2f", rmsLevel))")
                        Slider(value: $rmsLevel, in: 0...1.0)
                        
                        Toggle("Auto-Animate", isOn: .constant(true))
                    }
                }
            }
            .frame(height: 300)
        }
        .padding()
        .onReceive(timer) { _ in
            // Simulate audio levels for testing glow response
            if rmsLevel < 0.05 {
                rmsLevel += Double.random(in: 0.01...0.15)
            } else if rmsLevel > 0.95 {
                rmsLevel -= Double.random(in: 0.01...0.15)
            } else {
                rmsLevel += Double.random(in: -0.1...0.1)
            }
            
            // Bound to valid range
            rmsLevel = min(1.0, max(0.0, rmsLevel))
        }
    }
}

#Preview {
    ControlPanel()
}
