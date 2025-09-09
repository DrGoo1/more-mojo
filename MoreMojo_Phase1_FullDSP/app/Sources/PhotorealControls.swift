
import SwiftUI

private extension Double { var clamped01: Double { min(1, max(0, self)) } }
private extension CGFloat { var clamped01: CGFloat { min(1, max(0, self)) } }

struct SpriteStrip: View {
    let imageName: String
    let frames: Int
    let index: Int
    var body: some View {
        GeometryReader { geo in
            let frameW = geo.size.width
            Image(imageName).resizable().interpolation(.high).aspectRatio(contentMode: .fill)
                .frame(width: frameW * CGFloat(frames), height: geo.size.height, alignment: .leading)
                .offset(x: -CGFloat(index) * frameW)
                .clipped()
        }
    }
}

struct PRRoundKnob: View {
    @Binding var value: Double
    var label: String = "KNOB"
    var onChange: (() -> Void)? = nil
    var body: some View {
        VStack(spacing: 6) {
            if let spr = MoreMojoAssets.roundSprite {
                SpriteStrip(imageName: spr, frames: MoreMojoAssets.roundFrames, index: frameIndex)
                    .frame(width: 128, height: 128)
                    .gesture(DragGesture(minimumDistance: 0).onChanged { g in
                        value = (value - Double(g.translation.height/140)).clamped01
                        onChange?()
                    })
            } else {
                Circle().fill(.gray).frame(width: 128, height: 128)
            }
            Text(label.uppercased()).font(.system(size: 10, weight: .semibold, design: .rounded))
        }
    }
    private var frameIndex: Int {
        let idx = Int(round(value.clamped01 * Double(max(MoreMojoAssets.roundFrames-1, 1))))
        return idx
    }
}

struct PRChickenHeadSelector: View {
    @Binding var index: Int
    var label: String = "SELECT"
    var positions: Int = MoreMojoAssets.chickenPositions
    var body: some View {
        VStack(spacing: 6) {
            if let base = MoreMojoAssets.chickenBase {
                Image(base).resizable().interpolation(.high).scaledToFit()
                    .rotationEffect(angleFor(index))
                    .shadow(color: .black.opacity(0.4), radius: 1)
                    .frame(width: 112, height: 112)
                    .gesture(DragGesture(minimumDistance: 0).onChanged { g in
                        let dy = g.translation.height
                        let delta = Int(round(-dy / 30))
                        index = min(positions-1, max(0, index + delta))
                    })
            } else {
                Circle().fill(.gray).frame(width: 112, height: 112)
            }
            Text(label.uppercased()).font(.system(size: 10, weight: .semibold, design: .rounded))
        }
    }
    private func angleFor(_ idx: Int) -> Angle {
        let span: Double = 270
        let step = positions > 1 ? span / Double(positions-1) : 0
        return .degrees(-135 + Double(idx) * step)
    }
}

struct PRToggleNeve: View {
    @Binding var isOn: Bool
    var label: String = ""
    var body: some View {
        VStack(spacing: 6) {
            if let up = MoreMojoAssets.switchUp, let down = MoreMojoAssets.switchDown {
                Image(isOn ? up : down).resizable().interpolation(.high).scaledToFit()
                    .contentShape(Rectangle())
                    .onTapGesture { isOn.toggle() }
            } else {
                RoundedRectangle(cornerRadius: 5).fill(isOn ? .green : .black.opacity(0.7))
                    .frame(width: 36, height: 72).onTapGesture { isOn.toggle() }
            }
            if !label.isEmpty {
                Text(label.uppercased()).font(.system(size: 9, weight: .semibold, design: .rounded))
                    .foregroundColor(.white.opacity(0.8))
            }
        }
    }
}

enum PRLEDColor { case red, green, amber }
struct PRLED: View {
    var color: PRLEDColor = .red
    var on: Bool = true
    var body: some View {
        let name: String? = {
            switch color {
            case .red:   return MoreMojoAssets.ledRed
            case .green: return MoreMojoAssets.ledGreen
            case .amber: return MoreMojoAssets.ledAmber
            }
        }()
        if let n = name {
            Image(n).resizable().interpolation(.high)
                .frame(width: 14, height: 14)
                .opacity(on ? 1 : 0.35)
                .shadow(color: .white.opacity(on ? 0.25 : 0), radius: on ? 2 : 0)
        } else {
            Circle().fill(on ? .red : .gray).frame(width: 12, height: 12)
        }
    }
}

struct PRVUMeter: View {
    var value: CGFloat
    var body: some View {
        ZStack {
            if let paper = MoreMojoAssets.vuPaper {
                Image(paper).resizable().interpolation(.high).scaledToFit()
            } else {
                RoundedRectangle(cornerRadius: 8).fill(Color(red:0.98, green:0.96, blue:0.86))
            }
            if let needle = MoreMojoAssets.vuNeedle {
                Image(needle).resizable().scaledToFit()
                    .shadow(color: .black.opacity(0.5), radius: 2)
                    .rotationEffect(.degrees(Double(value.clamped01 * 120 - 60)))
                    .offset(y: -2)
            } else {
                Capsule().fill(Color.red.opacity(0.9)).frame(width: 2, height: 40)
                    .rotationEffect(.degrees(Double(value.clamped01 * 120 - 60)))
            }
            if let glass = MoreMojoAssets.vuGlass, !glass.isEmpty {
                Image(glass).resizable().interpolation(.high).scaledToFit().opacity(0.85)
            }
        }
    }
}
