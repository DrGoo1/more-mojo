import SwiftUI

struct CompareKnobsView: View {
    @State private var value1 = 0.5
    @State private var value2 = 0.5
    @State private var value3 = 0.5
    @State private var animatedRMS: CGFloat = 0
    
    // Timer for animation
    let timer = Timer.publish(every: 0.05, on: .main, in: .common).autoconnect()
    
    var body: some View {
        ZStack {
            // Dark background with subtle gradient
            LinearGradient(
                gradient: Gradient(colors: [
                    Color(white: 0.1),
                    Color(white: 0.08)
                ]),
                startPoint: .top,
                endPoint: .bottom
            )
            .ignoresSafeArea()
            
            VStack(spacing: 40) {
                Text("Professional UI Component Comparison")
                    .font(.title)
                    .fontWeight(.bold)
                    .foregroundColor(.white)
                    .padding(.top, 30)
                
                // Comparison section
                HStack(spacing: 60) {
                    // Left column - Original knobs
                    VStack(spacing: 20) {
                        Text("Original Design")
                            .font(.headline)
                            .foregroundColor(.white)
                            .padding(.bottom, 10)
                        
                        // Original knob
                        PRRoundKnob(value: $value1, label: "DRIVE")
                            .frame(width: 120, height: 120)
                        
                        // Original knob
                        PRRoundKnob(value: $value2, label: "MIX")
                            .frame(width: 120, height: 120)
                        
                        // Original knob
                        PRRoundKnob(value: $value3, label: "OUTPUT")
                            .frame(width: 120, height: 120)
                    }
                    
                    // Right column - Enhanced 3D knobs
                    VStack(spacing: 20) {
                        Text("Enhanced 3D Design")
                            .font(.headline)
                            .foregroundColor(.white)
                            .padding(.bottom, 10)
                        
                        // Enhanced knob 1 - blue to red
                        MojoWheelKnob(
                            value: $value1,
                            label: "DRIVE",
                            rms: animatedRMS,
                            startHue: 0.6,   // Blue
                            endHue: 0.0      // Red
                        )
                        .frame(width: 120, height: 120)
                        
                        // Enhanced knob 2 - green to purple
                        MojoWheelKnob(
                            value: $value2,
                            label: "MIX",
                            rms: animatedRMS,
                            startHue: 0.3,   // Green
                            endHue: 0.5      // Purple
                        )
                        .frame(width: 120, height: 120)
                        
                        // Enhanced knob 3 - orange to blue-purple
                        MojoWheelKnob(
                            value: $value3,
                            label: "OUTPUT",
                            rms: animatedRMS,
                            startHue: 0.15,  // Orange
                            endHue: 0.65     // Blue-purple
                        )
                        .frame(width: 120, height: 120)
                    }
                }
                
                Spacer()
                
                Text("The enhanced 3D design combines metallic textures with psychedelic color gradients")
                    .font(.subheadline)
                    .foregroundColor(.gray)
                    .multilineTextAlignment(.center)
                    .padding(.bottom, 20)
            }
            .padding()
        }
        .onReceive(timer) { _ in
            // Simulate audio RMS for animation
            let randomVariation = Double.random(in: -0.15...0.15)
            animatedRMS = CGFloat(max(0, min(1, 0.2 + randomVariation)))
        }
    }
}
