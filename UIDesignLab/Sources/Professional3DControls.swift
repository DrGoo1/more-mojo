import SwiftUI

struct Professional3DButton: View {
    var text: String
    var isPressed: Bool
    var action: () -> Void
    
    var width: CGFloat = 100
    var height: CGFloat = 40
    
    var body: some View {
        Button(action: action) {
            ZStack {
                // Button base (shadow when not pressed)
                if !isPressed {
                    RoundedRectangle(cornerRadius: 6)
                        .fill(Color.black.opacity(0.5))
                        .frame(width: width + 2, height: height + 2)
                        .blur(radius: 2)
                        .offset(x: 1, y: 1.5)
                }
                
                // Button body
                RoundedRectangle(cornerRadius: 6)
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                isPressed ? Color(white: 0.55) : Color(white: 0.75),
                                isPressed ? Color(white: 0.35) : Color(white: 0.5)
                            ]),
                            startPoint: isPressed ? .bottom : .top,
                            endPoint: isPressed ? .top : .bottom
                        )
                    )
                    .overlay(
                        RoundedRectangle(cornerRadius: 6)
                            .stroke(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        isPressed ? Color.black.opacity(0.6) : Color.white.opacity(0.7),
                                        isPressed ? Color.white.opacity(0.3) : Color.black.opacity(0.3)
                                    ]),
                                    startPoint: isPressed ? .bottom : .top,
                                    endPoint: isPressed ? .top : .bottom
                                ),
                                lineWidth: 1.5
                            )
                    )
                    .overlay(
                        // Inner bevel
                        RoundedRectangle(cornerRadius: 4)
                            .stroke(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        isPressed ? Color.black.opacity(0.4) : Color.white.opacity(0.6),
                                        isPressed ? Color.white.opacity(0.2) : Color.black.opacity(0.2)
                                    ]),
                                    startPoint: isPressed ? .bottom : .top,
                                    endPoint: isPressed ? .top : .bottom
                                ),
                                lineWidth: 1
                            )
                            .padding(3)
                    )
                    .frame(width: width, height: height)
                    // Create inset when pressed
                    .offset(x: isPressed ? 0.5 : 0, y: isPressed ? 0.5 : 0)
                    // Subtle shadow for 3D effect
                    .shadow(color: .black.opacity(isPressed ? 0.1 : 0.3), radius: isPressed ? 1 : 3, x: 0, y: 0)
                
                // Button text with embossed effect
                ZStack {
                    // Text shadow (creates depth)
                    Text(text)
                        .font(.system(size: 14, weight: .bold))
                        .foregroundColor(.black.opacity(0.3))
                        .offset(x: 0.5, y: 0.5)
                    
                    // Main text
                    Text(text)
                        .font(.system(size: 14, weight: .bold))
                        .foregroundColor(.white.opacity(0.85))
                }
                .offset(x: isPressed ? 0.5 : 0, y: isPressed ? 0.5 : 0)
            }
        }
        .buttonStyle(PlainButtonStyle())
        .frame(width: width, height: height)
    }
}

struct Professional3DSlider: View {
    @Binding var value: Double
    var range: ClosedRange<Double> = 0...1
    var label: String = ""
    
    var width: CGFloat = 200
    var height: CGFloat = 24
    var thumbSize: CGFloat = 18
    
    var body: some View {
        VStack(spacing: 6) {
            if !label.isEmpty {
                HStack {
                    Text(label)
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.white.opacity(0.9))
                        .padding(.leading, 2)
                    
                    Spacer()
                    
                    Text(String(format: "%.2f", value))
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.white.opacity(0.7))
                        .padding(.trailing, 2)
                }
            }
            
            // Slider control
            ZStack(alignment: .leading) {
                // Groove/track with inset effect
                ZStack {
                    // Shadow layer for inset effect
                    Capsule()
                        .fill(Color.black.opacity(0.4))
                        .frame(width: width, height: height * 0.35)
                        .offset(x: 0, y: 0.5)
                    
                    // Inner groove with gradient
                    Capsule()
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(white: 0.2),
                                    Color(white: 0.3)
                                ]),
                                startPoint: .top,
                                endPoint: .bottom
                            )
                        )
                        .frame(width: width - 2, height: height * 0.3)
                    
                    // Progress fill
                    GeometryReader { geo in
                        let thumbOffset = width * CGFloat((value - range.lowerBound) / (range.upperBound - range.lowerBound))
                        
                        Capsule()
                            .fill(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color(hue: 0.6, saturation: 0.6, brightness: 0.7),
                                        Color(hue: 0.6, saturation: 0.5, brightness: 0.5)
                                    ]),
                                    startPoint: .leading,
                                    endPoint: .trailing
                                )
                            )
                            .frame(width: thumbOffset, height: height * 0.25)
                            .shadow(color: Color(hue: 0.6, saturation: 0.6, brightness: 0.7).opacity(0.6), radius: 3, x: 0, y: 0)
                    }
                }
                
                // Thumb with 3D effect
                GeometryReader { geo in
                    let thumbOffset = width * CGFloat((value - range.lowerBound) / (range.upperBound - range.lowerBound)) - thumbSize/2
                    
                    Circle()
                        .fill(
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(white: 0.85),
                                    Color(white: 0.7)
                                ]),
                                startPoint: .top,
                                endPoint: .bottom
                            )
                        )
                        .overlay(
                            Circle()
                                .stroke(
                                    LinearGradient(
                                        gradient: Gradient(colors: [
                                            Color.white.opacity(0.9),
                                            Color.black.opacity(0.3)
                                        ]),
                                        startPoint: .topLeading,
                                        endPoint: .bottomTrailing
                                    ),
                                    lineWidth: 1
                                )
                        )
                        .overlay(
                            // Light reflection
                            Circle()
                                .fill(
                                    LinearGradient(
                                        gradient: Gradient(colors: [
                                            Color.white.opacity(0.7),
                                            Color.white.opacity(0.0)
                                        ]),
                                        startPoint: .topLeading,
                                        endPoint: .bottomTrailing
                                    )
                                )
                                .padding(4)
                        )
                        .shadow(color: .black.opacity(0.3), radius: 2, x: 0.5, y: 0.5)
                        .frame(width: thumbSize, height: thumbSize)
                        .position(x: max(thumbSize/2, min(width - thumbSize/2, thumbOffset + thumbSize/2)), y: height/2)
                        .gesture(
                            DragGesture(minimumDistance: 0)
                                .onChanged { gesture in
                                    let newValue = range.lowerBound + (range.upperBound - range.lowerBound) * Double(max(0, min(1, gesture.location.x / width)))
                                    value = newValue
                                }
                        )
                }
            }
            .frame(width: width, height: height)
        }
    }
}

struct Professional3DToggle: View {
    @Binding var isOn: Bool
    var label: String
    
    var width: CGFloat = 48
    var height: CGFloat = 24
    
    var body: some View {
        HStack(spacing: 8) {
            // Toggle switch with 3D effect
            ZStack {
                // Base/track with 3D inset effect
                Capsule()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color(white: 0.25),
                                Color(white: 0.3)
                            ]),
                            startPoint: .top,
                            endPoint: .bottom
                        )
                    )
                    .overlay(
                        Capsule()
                            .stroke(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color.black.opacity(0.5),
                                        Color.white.opacity(0.2)
                                    ]),
                                    startPoint: .top,
                                    endPoint: .bottom
                                ),
                                lineWidth: 1
                            )
                    )
                    .shadow(color: .black.opacity(0.3), radius: 1, x: 0, y: 1)
                
                // Indicator track (changes color when on)
                Capsule()
                    .fill(
                        isOn ? 
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(hue: 0.4, saturation: 0.6, brightness: 0.6),
                                    Color(hue: 0.4, saturation: 0.8, brightness: 0.4)
                                ]),
                                startPoint: .leading,
                                endPoint: .trailing
                            )
                            :
                            LinearGradient(
                                gradient: Gradient(colors: [
                                    Color(white: 0.2),
                                    Color(white: 0.25)
                                ]),
                                startPoint: .leading,
                                endPoint: .trailing
                            )
                    )
                    .padding(3)
                
                // Thumb with 3D effect
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color(white: 0.9),
                                Color(white: 0.75)
                            ]),
                            startPoint: .top,
                            endPoint: .bottom
                        )
                    )
                    .overlay(
                        Circle()
                            .stroke(
                                LinearGradient(
                                    gradient: Gradient(colors: [
                                        Color.white.opacity(0.9),
                                        Color.black.opacity(0.3)
                                    ]),
                                    startPoint: .topLeading,
                                    endPoint: .bottomTrailing
                                ),
                                lineWidth: 1
                            )
                    )
                    .shadow(color: .black.opacity(0.4), radius: 1, x: 0.5, y: 0.5)
                    .frame(width: height - 6, height: height - 6)
                    .offset(x: isOn ? width/2 - height/2 + 3 : -width/2 + height/2 - 3, y: 0)
                    // Highlight when on
                    .overlay(
                        Circle()
                            .fill(
                                RadialGradient(
                                    gradient: Gradient(colors: [
                                        Color.white.opacity(0.9),
                                        Color.white.opacity(0)
                                    ]),
                                    center: .topLeading,
                                    startRadius: 1,
                                    endRadius: height
                                )
                            )
                            .padding(5)
                    )
                
                // Subtle glow when on
                if isOn {
                    Capsule()
                        .fill(Color(hue: 0.4, saturation: 0.8, brightness: 0.8))
                        .frame(width: width, height: height)
                        .blur(radius: 5)
                        .opacity(0.3)
                }
            }
            .frame(width: width, height: height)
            .onTapGesture {
                withAnimation(.spring(response: 0.35, dampingFraction: 0.7, blendDuration: 0)) {
                    isOn.toggle()
                }
            }
            
            // Label with embossed text
            ZStack {
                // Text shadow
                Text(label)
                    .font(.system(size: 14, weight: .medium))
                    .foregroundColor(.black.opacity(0.5))
                    .offset(x: 0.5, y: 0.5)
                
                // Main text
                Text(label)
                    .font(.system(size: 14, weight: .medium))
                    .foregroundColor(.white.opacity(0.9))
            }
        }
    }
}

struct Professional3DKnob: View {
    @Binding var value: Double
    var label: String = ""
    var size: CGFloat = 60
    
    var body: some View {
        VStack(spacing: 4) {
            // Knob with 3D effect
            ZStack {
                // Shadow base
                Circle()
                    .fill(Color.black.opacity(0.4))
                    .frame(width: size + 4, height: size + 4)
                    .blur(radius: 3)
                    .offset(x: 2, y: 2)
                
                // Outer ring/bezel
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color(white: 0.35),
                                Color(white: 0.2)
                            ]),
                            startPoint: .top,
                            endPoint: .bottom
                        )
                    )
                    .frame(width: size, height: size)
                    .shadow(color: .black.opacity(0.6), radius: 2, x: 0, y: 1)
                
                // Main knob body
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color(white: 0.85),
                                Color(white: 0.7)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
                    .frame(width: size * 0.85, height: size * 0.85)
                    .overlay(
                        // Brushed metal texture
                        ZStack {
                            ForEach(0..<12) { i in
                                Circle()
                                    .stroke(Color.gray.opacity(0.1), lineWidth: 0.5)
                                    .frame(width: size * 0.85 * CGFloat(i) / 12)
                            }
                        }
                    )
                    .overlay(
                        // Circular bevel edge
                        Circle()
                            .strokeBorder(
                                AngularGradient(
                                    gradient: Gradient(stops: [
                                        .init(color: Color.white.opacity(0.9), location: 0.0),
                                        .init(color: Color.gray.opacity(0.5), location: 0.25),
                                        .init(color: Color.black.opacity(0.5), location: 0.5),
                                        .init(color: Color.gray.opacity(0.5), location: 0.75),
                                        .init(color: Color.white.opacity(0.9), location: 1.0)
                                    ]),
                                    center: .center
                                ),
                                lineWidth: 1.5
                            )
                    )
                
                // Knob indicator line
                Rectangle()
                    .fill(Color.black)
                    .frame(width: 2, height: size * 0.3)
                    .offset(y: -size * 0.2)
                    .rotationEffect(Angle(degrees: value * 280 - 140))
                
                // Light reflection
                Circle()
                    .fill(
                        LinearGradient(
                            gradient: Gradient(colors: [
                                Color.white.opacity(0.6),
                                Color.white.opacity(0.0)
                            ]),
                            startPoint: .topLeading,
                            endPoint: .center
                        )
                    )
                    .frame(width: size * 0.7, height: size * 0.7)
                    .offset(x: -size * 0.05, y: -size * 0.05)
            }
            .frame(width: size, height: size)
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { gesture in
                        // Calculate center of circle
                        let center = CGPoint(x: size / 2, y: size / 2)
                        // Calculate angle of drag relative to center
                        let dragAngle = atan2(gesture.location.y - center.y, gesture.location.x - center.x)
                        // Convert to 0-360 degree range
                        var angleDegrees = dragAngle * 180 / .pi + 90
                        if angleDegrees < 0 { angleDegrees += 360 }
                        
                        // Only allow values within our 280 degree range (-140 to +140 from vertical)
                        if angleDegrees > 40 && angleDegrees < 320 {
                            // Map angle to value (0-1)
                            let mappedValue = (angleDegrees - 40) / 280
                            value = min(1.0, max(0.0, mappedValue))
                        }
                    }
            )
            
            // Label with value display
            if !label.isEmpty {
                ZStack {
                    // Shadow
                    Text("\(label): \(Int(value * 100))")
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.black.opacity(0.5))
                        .offset(x: 0.5, y: 0.5)
                    
                    // Text
                    Text("\(label): \(Int(value * 100))")
                        .font(.system(size: 12, weight: .medium))
                        .foregroundColor(.white.opacity(0.9))
                }
            }
        }
    }
}

struct ControlsDemoView: View {
    @State private var knobValue1 = 0.3
    @State private var knobValue2 = 0.7
    @State private var sliderValue1 = 0.5
    @State private var sliderValue2 = 0.25
    @State private var buttonPressed = false
    @State private var toggleOn = true
    
    var body: some View {
        ZStack {
            // Background
            Color(white: 0.12)
                .ignoresSafeArea()
            
            VStack(spacing: 30) {
                Text("Professional 3D Controls")
                    .font(.title)
                    .fontWeight(.bold)
                    .foregroundColor(.white)
                    .padding(.top, 20)
                
                // Knobs section
                VStack(spacing: 20) {
                    Text("Knobs")
                        .font(.headline)
                        .foregroundColor(.white)
                    
                    HStack(spacing: 50) {
                        Professional3DKnob(value: $knobValue1, label: "Gain")
                        Professional3DKnob(value: $knobValue2, label: "Tone")
                    }
                }
                .padding(.vertical)
                
                // Sliders section
                VStack(spacing: 20) {
                    Text("Sliders")
                        .font(.headline)
                        .foregroundColor(.white)
                    
                    Professional3DSlider(value: $sliderValue1, label: "Volume")
                    Professional3DSlider(value: $sliderValue2, label: "Resonance")
                }
                .padding(.vertical)
                .frame(width: 250)
                
                // Button and toggle section
                VStack(spacing: 20) {
                    Text("Buttons & Toggles")
                        .font(.headline)
                        .foregroundColor(.white)
                    
                    HStack(spacing: 30) {
                        Professional3DButton(
                            text: "PROCESS",
                            isPressed: buttonPressed,
                            action: { buttonPressed.toggle() }
                        )
                        
                        Professional3DToggle(isOn: $toggleOn, label: "Bypass")
                    }
                }
                .padding(.vertical)
            }
            .padding()
        }
    }
}

#Preview {
    ControlsDemoView()
}
