import SwiftUI

struct BrushedMetalKnob: View {
    @Binding var value: Double
    var minValue: Double = 0.0
    var maxValue: Double = 1.0
    var size: CGFloat = 200
    
    // Design parameters
    var indicatorColor: Color = .green  // Color can be changed from green to other colors based on value
    var showLabel: Bool = true
    var depth3D: Double = 0.2         // Controls how much 3D depth effect
    var bevelSize: Double = 0.08      // Size of the beveled edge
    var metalShine: Double = 0.8      // Controls the shine on the metal
    
    private var angle: Double {
        return (value - minValue) / (maxValue - minValue) * 270.0 - 135.0
    }
    
    var body: some View {
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
                            Color(white: 0.2),
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
                
                // 3D depth effect with radial gradient shadow
                .overlay(
                    Circle()
                        .fill(
                            RadialGradient(
                                gradient: Gradient(colors: [
                                    Color.clear,
                                    Color.black.opacity(CGFloat(depth3D) * 0.5)
                                ]),
                                center: .center,
                                startRadius: size * 0.2,
                                endRadius: size * 0.425
                            )
                        )
                        .blendMode(.multiply)
                )
                
                // Top highlight/reflection for 3D effect
                .overlay(
                    Circle()
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color.white.opacity(CGFloat(metalShine) * 0.9),
                                    Color.white.opacity(0.0)
                                ]),
                                startPoint: .topLeading,
                                endPoint: .center
                            )
                        )
                        .frame(width: size * 0.75, height: size * 0.75)
                        .offset(x: -size * 0.05, y: -size * 0.05)
                        .blendMode(.screen)
                )
                
                // Side glare highlight
                .overlay(
                    Capsule()
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color.white.opacity(CGFloat(metalShine) * 0.8),
                                    Color.white.opacity(0.0)
                                ]),
                                startPoint: .leading,
                                endPoint: .trailing
                            )
                        )
                        .frame(width: size * 0.08, height: size * 0.4)
                        .offset(x: size * 0.28, y: 0)
                        .blendMode(.screen)
                )
            
            // Indicator dot
            Circle()
                .fill(Color.black)
                .frame(width: size * 0.06, height: size * 0.06)
                .offset(y: -size * 0.35)
                .rotationEffect(Angle(degrees: angle))
            
            // Base indicator track (filled from min to current)
            Circle()
                .trim(from: 0.125, to: 0.125 + (value * 0.75))
                .stroke(
                    AngularGradient(
                        gradient: Gradient(colors: [
                            indicatorColor.opacity(0.3),
                            indicatorColor
                        ]),
                        center: .center
                    ),
                    style: StrokeStyle(lineWidth: size * 0.04, lineCap: .round)
                )
                .frame(width: size * 1.05, height: size * 1.05)
                .rotationEffect(.degrees(135))
            
            // Value label
            if showLabel {
                VStack {
                    Text("\(Int(value * 100))%")
                        .font(.system(size: size * 0.12, weight: .bold, design: .rounded))
                        .foregroundColor(.white)
                        .shadow(color: .black, radius: 1)
                }
            }
        }
        .gesture(
            DragGesture(minimumDistance: 0)
                .onChanged { gesture in
                    let center = CGPoint(x: size / 2, y: size / 2)
                    let location = gesture.location
                    let angle = atan2(location.y - center.y, location.x - center.x)
                    
                    // Convert angle to 0-1 value, taking into account the restricted rotation range
                    var angleValue = (Double(angle) / (2 * .pi)) + 0.5 // 0-1
                    
                    // Map the angle to our restricted rotation (270 degrees)
                    if angleValue < 0.125 { angleValue += 1 }
                    
                    // Constrain to valid range (0.125 to 0.875 in our angle space)
                    if angleValue >= 0.125 && angleValue <= 0.875 {
                        let normalizedValue = (angleValue - 0.125) / 0.75
                        self.value = min(max(normalizedValue * (maxValue - minValue) + minValue, minValue), maxValue)
                    }
                }
        )
    }
}

struct PsychedelicKnob: View {
    @Binding var value: Double
    var rms: CGFloat = 0
    
    // Controls for design experimentation
    var knobDepth: Double = 0.15      // Controls how "3D" the knob appears
    var glowIntensity: Double = 0.7    // Controls the edge glow intensity
    var glowRadius: Double = 15        // Controls the edge glow radius
    var reflectionOpacity: Double = 0.4 // Controls the top reflection highlight
    var bevelSize: Double = 0.1        // Controls size of the beveled edge
    
    var body: some View {
        GeometryReader { geo in
            let size = min(geo.size.width, geo.size.height)
            let thickness = size * 0.13
            
            ZStack {
                // Drop shadow for 3D floating effect
                Circle()
                    .fill(Color.black.opacity(0.5))
                    .frame(width: size * 1.05, height: size * 1.05)
                    .blur(radius: 10)
                    .offset(x: 4, y: 4)
                
                // Outer ring casing
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(stops: [
                                .init(color: Color(hue: 0.7, saturation: 0.6, brightness: 0.3), location: 0.0),
                                .init(color: Color(hue: 0.7, saturation: 0.5, brightness: 0.2), location: 1.0)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .frame(width: size * 1.0, height: size * 1.0)
                
                // Inner bevel effect
                Circle()
                    .strokeBorder(
                        AngularGradient(
                            gradient: Gradient(stops: [
                                .init(color: Color(hue: 0.7, saturation: 0.2, brightness: 0.1), location: 0.0),
                                .init(color: Color(hue: 0.7, saturation: 0.2, brightness: 0.4), location: 0.25),
                                .init(color: Color(hue: 0.7, saturation: 0.2, brightness: 0.1), location: 0.5),
                                .init(color: Color(hue: 0.7, saturation: 0.2, brightness: 0.4), location: 0.75),
                                .init(color: Color(hue: 0.7, saturation: 0.2, brightness: 0.1), location: 1.0)
                            ]),
                            center: .center
                        ),
                        lineWidth: size * CGFloat(bevelSize)
                    )
                    .frame(width: size * 0.95, height: size * 0.95)
                
                // Main knob body with 3D gradient
                Circle()
                    .fill(
                        RadialGradient(
                            gradient: Gradient(stops: [
                                .init(color: Color(hue: 0.96, saturation: 0.73, brightness: 0.95), location: 0.0),
                                .init(color: Color(hue: 0.04, saturation: 0.85, brightness: 0.8), location: 0.5),
                                .init(color: Color(hue: 0.95, saturation: 0.7, brightness: 0.65), location: 1.0)
                            ]),
                            center: .center,
                            startRadius: 0,
                            endRadius: size * 0.45
                        )
                    )
                    .frame(width: size * 0.85, height: size * 0.85)
                    
                    // Inner beveled edge
                    .overlay(
                        Circle()
                            .strokeBorder(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color.white.opacity(0.8),
                                        Color(hue: 0.95, saturation: 0.7, brightness: 0.4).opacity(0.6),
                                        Color.black.opacity(0.5),
                                        Color(hue: 0.05, saturation: 0.7, brightness: 0.4).opacity(0.6),
                                        Color.white.opacity(0.8)
                                    ]),
                                    startPoint: .topLeading,
                                    endPoint: .bottomTrailing
                                ),
                                lineWidth: size * CGFloat(bevelSize) * 0.7
                            )
                    )
                
                    // 3D depth inner shadow
                    .overlay(
                        Circle()
                            .fill(
                                RadialGradient(
                                    gradient: Gradient(colors: [
                                        Color.clear,
                                        Color.black.opacity(knobDepth * 0.8)
                                    ]),
                                    center: .center,
                                    startRadius: size * 0.05,
                                    endRadius: size * 0.45
                                )
                            )
                            .mask(
                                Circle()
                                    .frame(width: size * 0.75, height: size * 0.75)
                            )
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

struct KnobDesignSelector: View {
    @State private var selectedDesign = 2 // Default to the new hybrid design
    @State private var value: Double = 0.5
    @State private var rmsLevel: Double = 0.0
    
    // Design adjustment controls
    @State private var knobDepth: Double = 0.15
    @State private var glowIntensity: Double = 0.7
    @State private var glowRadius: Double = 15
    @State private var reflectionOpacity: Double = 0.4
    @State private var metalShine: Double = 0.8
    @State private var bevelSize: Double = 0.08
    @State private var indicatorColor: Color = .green
    @State private var startHue: Double = 0.6  // More blue/purple
    @State private var endHue: Double = 0.1    // Yellow/orange
    
    // Animated RMS simulation
    let timer = Timer.publish(every: 0.05, on: .main, in: .common).autoconnect()
    
    var body: some View {
        VStack(spacing: 20) {
            Text("MojoWheel Design Lab")
                .font(.largeTitle)
                .fontWeight(.bold)
                .padding(.top, 20)
            
            // Design selector
            Picker("Knob Style", selection: $selectedDesign) {
                Text("Psychedelic Knob").tag(0)
                Text("Brushed Metal Knob").tag(1)
                Text("Hybrid Knob").tag(2)
            }
            .pickerStyle(SegmentedPickerStyle())
            .padding(.horizontal)
            
            // Preview area with background
            ZStack {
                // Background
                if selectedDesign == 0 {
                    // Psychedelic background
                    RoundedRectangle(cornerRadius: 20)
                        .fill(
                            RadialGradient(
                                gradient: Gradient(colors: [
                                    Color(hue: 0.7, saturation: 0.3, brightness: 0.1),
                                    Color(hue: 0.8, saturation: 0.2, brightness: 0.05)
                                ]),
                                center: .center,
                                startRadius: 50,
                                endRadius: 300
                            )
                        )
                        .frame(width: 400, height: 400)
                } else {
                    // Neutral dark background for metal and hybrid
                    RoundedRectangle(cornerRadius: 20)
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(white: 0.15),
                                    Color(white: 0.05)
                                ]),
                                startPoint: .top,
                                endPoint: .bottom
                            )
                        )
                        .frame(width: 400, height: 400)
                }
                
                switch selectedDesign {
                case 0:
                    // Psychedelic Knob
                    PsychedelicKnob(
                        value: $value,
                        rms: CGFloat(rmsLevel),
                        knobDepth: knobDepth,
                        glowIntensity: glowIntensity,
                        glowRadius: glowRadius,
                        reflectionOpacity: reflectionOpacity,
                        bevelSize: bevelSize
                    )
                    .frame(width: 300, height: 300)
                    
                case 1:
                    // Brushed Metal Knob
                    BrushedMetalKnob(
                        value: $value,
                        indicatorColor: indicatorColor,
                        depth3D: knobDepth,
                        bevelSize: bevelSize,
                        metalShine: metalShine
                    )
                    .frame(width: 300, height: 300)
                    
                case 2:
                    // Hybrid Knob
                    HybridMojoKnob(
                        value: $value,
                        rms: CGFloat(rmsLevel),
                        knobDepth: knobDepth,
                        glowIntensity: glowIntensity,
                        glowRadius: glowRadius,
                        reflectionOpacity: reflectionOpacity,
                        metalShine: metalShine,
                        bevelSize: bevelSize,
                        startHue: startHue,
                        endHue: endHue
                    )
                    .frame(width: 300, height: 300)
                    
                default:
                    Text("Invalid design selected")
                }
            }
            .padding()
            
            // Design controls based on selected design
            Form {
                switch selectedDesign {
                case 0: // Psychedelic knob controls
                    Section(header: Text("3D Controls")) {
                        VStack(alignment: .leading) {
                            Text("Knob Depth: \(String(format: "%.2f", knobDepth))")
                            Slider(value: $knobDepth, in: 0...0.3)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Reflection Opacity: \(String(format: "%.2f", reflectionOpacity))")
                            Slider(value: $reflectionOpacity, in: 0...0.8)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Bevel Size: \(String(format: "%.2f", bevelSize))")
                            Slider(value: $bevelSize, in: 0.05...0.15)
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
                    
                case 1: // Metal knob controls
                    Section(header: Text("Metal Controls")) {
                        VStack(alignment: .leading) {
                            Text("Metal Shine: \(String(format: "%.2f", metalShine))")
                            Slider(value: $metalShine, in: 0.4...1.0)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Knob Depth: \(String(format: "%.2f", knobDepth))")
                            Slider(value: $knobDepth, in: 0...0.3)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Bevel Size: \(String(format: "%.2f", bevelSize))")
                            Slider(value: $bevelSize, in: 0.05...0.15)
                        }
                        
                        ColorPicker("Indicator Color", selection: $indicatorColor)
                    }
                    
                case 2: // Hybrid knob controls
                    Section(header: Text("Color Controls")) {
                        VStack(alignment: .leading) {
                            Text("Start Color (Dark): \(String(format: "%.2f", startHue))")
                            Slider(value: $startHue, in: 0...1.0)
                                .tint(Color(hue: startHue, saturation: 0.8, brightness: 0.6))
                        }
                        
                        VStack(alignment: .leading) {
                            Text("End Color (Light): \(String(format: "%.2f", endHue))")
                            Slider(value: $endHue, in: 0...1.0)
                                .tint(Color(hue: endHue, saturation: 0.8, brightness: 0.9))
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Glow Intensity: \(String(format: "%.2f", glowIntensity))")
                            Slider(value: $glowIntensity, in: 0...1.0)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Glow Radius: \(String(format: "%.1f", glowRadius))")
                            Slider(value: $glowRadius, in: 5...30)
                        }
                    }
                    
                    Section(header: Text("3D Controls")) {
                        VStack(alignment: .leading) {
                            Text("Metal Shine: \(String(format: "%.2f", metalShine))")
                            Slider(value: $metalShine, in: 0.4...1.0)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Knob Depth: \(String(format: "%.2f", knobDepth))")
                            Slider(value: $knobDepth, in: 0...0.3)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Bevel Size: \(String(format: "%.2f", bevelSize))")
                            Slider(value: $bevelSize, in: 0.05...0.15)
                        }
                        
                        VStack(alignment: .leading) {
                            Text("Reflection Opacity: \(String(format: "%.2f", reflectionOpacity))")
                            Slider(value: $reflectionOpacity, in: 0...0.8)
                        }
                    }
                    
                default:
                    Text("Invalid design")
                }
                
                Section(header: Text("Animation Test")) {
                    VStack(alignment: .leading) {
                        Text("Audio Level (RMS): \(String(format: "%.2f", rmsLevel))")
                        Slider(value: $rmsLevel, in: 0...1.0)
                        
                        Toggle("Auto-Animate", isOn: .constant(true))
                    }
                }
            }
            .frame(height: 250)
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
            
            // Also update indicator color for metal knob based on value
            if selectedDesign == 1 {
                indicatorColor = Color(
                    hue: min(0.3, value) / 0.3, // Green to red as value increases
                    saturation: 0.8,
                    brightness: 0.8
                )
            }
        }
    }
}

#Preview {
    KnobDesignSelector()
}
