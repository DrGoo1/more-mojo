import SwiftUI

struct MojoMakerView: View {
    @Binding var drive: Double
    @Binding var mix: Double
    @Binding var outputNorm: Double
    @Binding var selectorIndex: Int
    @Binding var toggle1: Bool

    var spectrum: [Float]
    var vuL: CGFloat
    var vuR: CGFloat
    var onChange: ()->Void
    var onBack: ()->Void

    @State private var character: Double = 0.5
    @State private var saturation: Double = 0.45
    @State private var presence: Double = 0.5

    var body: some View {
        VStack(spacing: 16) {
            HStack {
                Text("Mojo Maker").font(.system(size: 22, weight: .bold, design: .rounded))
                Spacer()
                Button("Back") { withAnimation(.spring()) { onBack() } }
            }.padding(.horizontal, 16)

            HStack(spacing: 20) {
                PRVUMeter(value: vuL).frame(width: 240, height: 160)
                SpectrumAnalyzer(values: spectrum).frame(height: 90)
                PRVUMeter(value: vuR).frame(width: 240, height: 160)

                VStack(spacing: 8) {
                    PRToggleNeve(isOn: $toggle1, label: "HP")
                    HStack(spacing: 6) {
                        Text("POWER").font(.system(size: 9, weight: .semibold, design: .rounded)).foregroundColor(.white.opacity(0.9))
                        PRLED(color: .red, on: true)
                    }
                }.frame(width: 120)
            }
            .padding(.horizontal, 16)

            Divider().overlay(Color.white.opacity(0.2))

            HStack(spacing: 24) {
                VStack(spacing: 10) {
                    Text("SELECT").font(.system(size: 10, weight: .semibold, design: .rounded))
                    PRChickenHeadSelector(index: $selectorIndex, label: "MODE")
                        .frame(width: 120, height: 150)
                        .onChange(of: selectorIndex) { _ in onChange() }
                }
                
                VStack(spacing: 10) {
                    Text("CHARACTER").font(.system(size: 10, weight: .semibold, design: .rounded))
                    HStack(spacing: 18) {
                        PRRoundKnob(value: $character, label: "CHAR")
                            .onChange(of: character) { _ in onChange() }
                        PRRoundKnob(value: $saturation, label: "SAT")
                            .onChange(of: saturation) { _ in onChange() }
                    }
                }
                
                VStack(spacing: 10) {
                    Text("DRIVE / MIX").font(.system(size: 10, weight: .semibold, design: .rounded))
                    HStack(spacing: 18) {
                        PRRoundKnob(value: $drive, label: "DRIVE")
                            .onChange(of: drive) { _ in onChange() }
                        PRRoundKnob(value: $mix, label: "MIX")
                            .onChange(of: mix) { _ in onChange() }
                    }
                }
                
                VStack(spacing: 10) {
                    Text("MASTER").font(.system(size: 10, weight: .semibold, design: .rounded))
                    HStack(spacing: 18) {
                        PRRoundKnob(value: $presence, label: "PRESENCE")
                            .onChange(of: presence) { _ in onChange() }
                        PRRoundKnob(value: $outputNorm, label: "OUTPUT")
                            .onChange(of: outputNorm) { _ in onChange() }
                    }
                }
            }
            .padding(.horizontal, 16)
        }
        .padding(.vertical, 12)
        .background(Color.black.opacity(0.85))
        .cornerRadius(12)
    }
}

struct SpectrumAnalyzer: View {
    var values: [Float]
    
    var body: some View {
        // Fallback implementation using standard SwiftUI components
        ZStack {
            // Background
            RoundedRectangle(cornerRadius: 6)
                .fill(Color.black.opacity(0.8))
                .overlay(
                    RoundedRectangle(cornerRadius: 6)
                        .stroke(Color.white.opacity(0.2), lineWidth: 1)
                )
            
            // Bars
            HStack(spacing: 1) {
                ForEach(0..<min(values.count, 30), id: \.self) { i in
                    SpectrumBar(value: CGFloat(values[i]))
                }
            }
            .padding(2)
        }
    }
}

struct SpectrumBar: View {
    var value: CGFloat
    
    var body: some View {
        LinearGradient(
            gradient: Gradient(colors: [.green.opacity(0.95), .yellow.opacity(0.9)]),
            startPoint: .bottom,
            endPoint: .top
        )
        .frame(height: max(1, value * 90))
        .cornerRadius(2)
    }
}
