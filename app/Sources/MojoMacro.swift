import SwiftUI

struct MojoMacro: View {
    @Binding var house: Bool
    @State private var curve: [CGPoint] = []
    
    // Macro settings
    var baseHeight: CGFloat = 100
    var segments: Int = 32
    var smoothness: Double = 0.8
    
    var body: some View {
        ZStack {
            // Background
            RoundedRectangle(cornerRadius: 12)
                .fill(PMXPalette.darkBackground)
                .pmxBorder()
            
            // Macro curve
            Path { path in
                guard curve.count > 1 else { return }
                
                path.move(to: curve[0])
                for i in 1..<curve.count {
                    path.addLine(to: curve[i])
                }
            }
            .stroke(house ? PMXPalette.mojoGreen : PMXPalette.mojoBlue, lineWidth: 3)
            .padding(12)
            
            // Mode indicator
            VStack {
                HStack {
                    Spacer()
                    Text(house ? "HOUSE" : "CUSTOM")
                        .font(.caption)
                        .fontWeight(.bold)
                        .foregroundColor(house ? PMXPalette.mojoGreen : PMXPalette.mojoBlue)
                        .padding(4)
                        .background(
                            RoundedRectangle(cornerRadius: 4)
                                .fill(PMXPalette.darkBackground.opacity(0.7))
                        )
                        .padding(8)
                }
                Spacer()
            }
        }
        .onAppear {
            generateCurve()
        }
        .onChange(of: house) { _ in
            withAnimation(.easeInOut(duration: 0.5)) {
                generateCurve()
            }
        }
    }
    
    private func generateCurve() {
        var points: [CGPoint] = []
        
        // Calculate width step
        let step = baseHeight / CGFloat(segments)
        
        for i in 0...segments {
            let x = CGFloat(i) * step
            let y: CGFloat
            
            if house {
                // House curve (smooth bell)
                let normalized = Double(i) / Double(segments)
                let centered = normalized * 2.0 - 1.0
                y = baseHeight - baseHeight * CGFloat(1.0 - pow(centered, 2) * smoothness)
            } else {
                // Custom curve (random walk)
                let phase = Double(i) / Double(segments)
                let randomComponent = sin(phase * 10) + sin(phase * 17) + sin(phase * 31)
                y = baseHeight - baseHeight * CGFloat(0.5 + 0.4 * randomComponent / 3.0)
            }
            
            points.append(CGPoint(x: x, y: y))
        }
        
        self.curve = points
    }
}
