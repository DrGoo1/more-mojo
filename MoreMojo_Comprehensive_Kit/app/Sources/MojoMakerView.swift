
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

    var body: some View {
        VStack(spacing: 16) {
            HStack {
                Text("Mojo Maker").font(.system(size: 22, weight: .bold, design: .rounded))
                Spacer()
                Button("Back") { withAnimation(.spring()) { onBack() } }
            }.padding(.horizontal, 16)

            HStack(spacing: 20) {
                PRVUMeter(value: vuL).frame(width: 240, height: 160)
                CanvasSpectrum(values: spectrum).frame(height: 90)
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
                    Text("DRIVE / MIX").font(.system(size: 10, weight: .semibold, design: .rounded))
                    HStack(spacing: 18) {
                        PRRoundKnob(value: $drive, label: "DRIVE").onChange(of: drive) { _ in onChange() }
                        PRRoundKnob(value: $mix,   label: "MIX")  .onChange(of: mix)   { _ in onChange() }
                    }
                }
                VStack(spacing: 10) {
                    Text("MASTER").font(.system(size: 10, weight: .semibold, design: .rounded))
                    HStack(spacing: 18) {
                        PRRoundKnob(value: $outputNorm, label: "OUTPUT").onChange(of: outputNorm) { _ in onChange() }
                    }
                }
            }
            .padding(.horizontal, 16)
        }
        .padding(.vertical, 12).background(Color.black.opacity(0.85))
    }
}

struct CanvasSpectrum: View {
    var values: [Float]
    var body: some View {
        Canvas { ctx, size in
            let n = max(1, values.count)
            let w = max(1, size.width / CGFloat(n))
            let slot = RoundedRectangle(cornerRadius: 6).path(in: CGRect(origin: .zero, size: size))
            ctx.fill(slot, with: .color(.black.opacity(0.8)))
            ctx.stroke(slot, with: .color(.white.opacity(0.2)), lineWidth: 1)
            for i in 0..<n {
                let h = max(1, CGFloat(values[i]) * size.height)
                let rect = CGRect(x: CGFloat(i)*w, y: size.height - h, width: w-1, height: h)
                ctx.fill(Path(roundedRect: rect, cornerRadius: 2),
                         with: .linearGradient(Gradient(colors: [.green.opacity(0.95), .yellow.opacity(0.9)]),
                                               startPoint: CGPoint(x: rect.minX, y: rect.maxY),
                                               endPoint:   CGPoint(x: rect.minX, y: rect.minY)))
            }
        }
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}
