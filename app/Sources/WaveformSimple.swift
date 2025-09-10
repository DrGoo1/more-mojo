import SwiftUI

struct WaveformSimple: View {
    var samples: [Float]
    var color: Color = PMXPalette.mojoBlue
    var backgroundColor: Color = PMXPalette.darkBackground
    
    @State private var animating = false
    
    var body: some View {
        ZStack {
            backgroundColor
            
            // Draw waveform
            GeometryReader { geometry in
                Path { path in
                    let width = geometry.size.width
                    let height = geometry.size.height
                    let midHeight = height / 2
                    
                    let horizontalScale = width / CGFloat(samples.count)
                    
                    // Start at left edge at the middle height
                    path.move(to: CGPoint(x: 0, y: midHeight))
                    
                    for i in 0..<samples.count {
                        let x = CGFloat(i) * horizontalScale
                        let y = midHeight - CGFloat(samples[i]) * midHeight * (animating ? 1.0 : 0.8)
                        path.addLine(to: CGPoint(x: x, y: y))
                    }
                    
                    // Continue to the right edge
                    path.addLine(to: CGPoint(x: width, y: midHeight))
                    path.closeSubpath()
                }
                .fill(color.opacity(0.5))
                
                // Line only version for the outline
                Path { path in
                    let width = geometry.size.width
                    let height = geometry.size.height
                    let midHeight = height / 2
                    
                    let horizontalScale = width / CGFloat(samples.count)
                    
                    // Start at left edge at the middle height
                    path.move(to: CGPoint(x: 0, y: midHeight))
                    
                    for i in 0..<samples.count {
                        let x = CGFloat(i) * horizontalScale
                        let y = midHeight - CGFloat(samples[i]) * midHeight * (animating ? 1.0 : 0.8)
                        path.addLine(to: CGPoint(x: x, y: y))
                    }
                }
                .stroke(color, lineWidth: 2)
            }
        }
        .cornerRadius(8)
        .onAppear {
            withAnimation(Animation.easeInOut(duration: 1.5).repeatForever(autoreverses: true)) {
                animating = true
            }
        }
    }
    
    // Factory method to create a demo waveform with random data
    static func demo(color: Color = PMXPalette.mojoBlue) -> some View {
        let sampleCount = 80
        var samples = [Float](repeating: 0, count: sampleCount)
        
        // Generate a demo waveform
        for i in 0..<sampleCount {
            let normalized = Float(i) / Float(sampleCount)
            let phase = normalized * Float.pi * 2
            
            // Mix several frequencies
            let value = sin(phase * 4) * 0.4 + 
                        sin(phase * 7) * 0.3 + 
                        sin(phase * 10) * 0.2 +
                        sin(phase * 20) * 0.1
            
            samples[i] = value
        }
        
        return WaveformSimple(samples: samples, color: color)
    }
}
