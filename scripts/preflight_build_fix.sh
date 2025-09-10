#!/usr/bin/env bash
set -euo pipefail
SRC="${1:-app/Sources}"

echo "=== Preflight fixes in $SRC ==="

# 1) Use AppState everywhere; remove AppStateStub
#    - Ensure AppState.swift exists (or create a minimal one)
if [ ! -f "$SRC/AppState.swift" ]; then
  cat > "$SRC/AppState.swift" <<'EOS'
import Foundation

@MainActor
final class AppState: ObservableObject {
    @Published var uiMode: UIMode = .top
    @Published var lastLoadedURL: URL?
}
EOS
fi

#    - If AppStateStub is referenced, alias it
grep -Rnl --include="*.swift" '\bAppStateStub\b' "$SRC" | while read -r f; do
  if ! grep -q 'typealias AppStateStub' "$SRC/AppState.swift" 2>/dev/null; then
    echo 'typealias AppStateStub = AppState' >> "$SRC/AppState.swift"
  fi
done

# 2) Ensure AudioEngine conforms to ObservableObject + @Published props
#    - Add conformance if missing
perl -0777 -pe 's/final\s+class\s+AudioEngine\s*:/final class AudioEngine: ObservableObject, /s' -i "$SRC/AudioEngine.swift" 2>/dev/null || true
#    - Ensure common published properties exist
awk '
/final class AudioEngine/ {inAE=1}
{print}
' "$SRC/AudioEngine.swift" > "$SRC/AudioEngine.swift.tmp" && mv "$SRC/AudioEngine.swift.tmp" "$SRC/AudioEngine.swift"

# 3) Replace iOS-only code in FaceplateOverlay.swift (fresh macOS-safe file)
cat > "$SRC/FaceplateOverlay.swift" <<'EOS'
import SwiftUI

struct FaceplateLayout: Decodable {
    struct RectPx: Decodable { var x: CGFloat; var y: CGFloat; var w: CGFloat; var h: CGFloat }
    var faceplateAssetName: String
    var baseWidth: CGFloat
    var baseHeight: CGFloat
    var rects: [String: RectPx]
}

struct NormRect {
    var x: CGFloat, y: CGFloat, w: CGFloat, h: CGFloat
    func rect(in size: CGSize) -> CGRect { CGRect(x: x*size.width, y: y*size.height, width: w*size.width, height: h*size.height) }
    static func from(px r: FaceplateLayout.RectPx, base: CGSize) -> NormRect {
        .init(x: r.x/base.width, y: r.y/base.height, w: r.w/base.width, h: r.h/base.height)
    }
}

final class FaceplateLayoutLoader {
    static func load(named: String = "faceplate_layout") -> FaceplateLayout? {
        if let url = Bundle.main.url(forResource: named, withExtension: "json"),
           let data = try? Data(contentsOf: url),
           let m = try? JSONDecoder().decode(FaceplateLayout.self, from: data) { return m }
        let fm = FileManager.default
        if let appSup = try? fm.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true) {
            let url = appSup.appendingPathComponent(named + ".json")
            if let data = try? Data(contentsOf: url),
               let m = try? JSONDecoder().decode(FaceplateLayout.self, from: data) { return m }
        }
        return nil
    }
}

struct BritStripFaceplate<Content: View>: View {
    let layout: FaceplateLayout
    @ViewBuilder var content: Content

    var body: some View {
        GeometryReader { geo in
            let baseAR = layout.baseWidth / layout.baseHeight
            let liveAR = geo.size.width / geo.size.height
            let drawSize: CGSize = liveAR > baseAR
                ? CGSize(width: geo.size.height * baseAR, height: geo.size.height)
                : CGSize(width: geo.size.width, height: geo.size.width / baseAR)
            let xOff = (geo.size.width - drawSize.width)/2
            let yOff = (geo.size.height - drawSize.height)/2

            ZStack(alignment: .topLeading) {
                Image(layout.faceplateAssetName)
                    .resizable().interpolation(.high)
                    .frame(width: drawSize.width, height: drawSize.height)
                content
                    .frame(width: drawSize.width, height: drawSize.height)
                    .offset(x: xOff, y: yOff)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(Color.black)
        }
    }
}

struct Positioned<Content: View>: View {
    let nr: NormRect
    @ViewBuilder var content: Content
    var body: some View {
        GeometryReader { geo in
            let r = nr.rect(in: geo.size)
            content
                .frame(width: r.width, height: r.height)
                .position(x: r.midX, y: r.midY)
        }
    }
}
EOS

# 4) Fix MojoWheel.swift errors: replace with safe version
cat > "$SRC/MojoWheel.swift" <<'EOS'
import SwiftUI
enum MojoMacroMode: String, CaseIterable { case appDecides, stealMacro }

struct MojoWheel: View {
    @Binding var value: Double
    var rms: CGFloat = 0
    var mode: MojoMacroMode = .appDecides
    var onChange: ((Double)->Void)? = nil

    var body: some View {
        GeometryReader { geo in
            let size = min(geo.size.width, geo.size.height)
            let thickness = size * 0.13
            ZStack {
                Circle()
                    .fill(LinearGradient(colors: [
                        Color(hue: 0.96, saturation: 0.73, brightness: 0.96),
                        Color(hue: 0.04, saturation: 0.90, brightness: 0.98)
                    ], startPoint: .top, endPoint: .bottom))
                    .overlay(Circle().stroke(Color.white.opacity(0.85), lineWidth: 2))
                    .shadow(color: Color.black.opacity(0.35), radius: 6, x: 0, y: 4)

                Circle().strokeBorder(
                    AngularGradient(colors: [
                        Color(red: 1.00, green: 0.31, blue: 0.00),
                        Color(red: 1.00, green: 0.73, blue: 0.00),
                        Color(red: 0.00, green: 0.80, blue: 0.45),
                        Color(red: 0.00, green: 0.62, blue: 0.98),
                        Color(red: 0.54, green: 0.00, blue: 1.00),
                        Color(red: 1.00, green: 0.31, blue: 0.68)
                    ], center: .center, angle: .degrees(value*360)),
                    lineWidth: thickness
                )
                .shadow(color: Color.white.opacity(0.2 + min(0.5, rms*0.8)), radius: 12)

                ForEach(0..<12) { i in
                    Capsule()
                        .fill(Color.white.opacity(0.85))
                        .frame(width: 3, height: thickness*0.6)
                        .offset(y: -size*0.38)
                        .rotationEffect(.degrees(Double(i) * 30))
                        .shadow(color: .black.opacity(0.2), radius: 1)
                }

                Capsule()
                    .fill(Color.white)
                    .frame(width: 4, height: size*0.38)
                    .offset(y: -size*0.19)
                    .rotationEffect(.degrees(value*360))
                    .shadow(color: Color.white.opacity(0.6), radius: 4)

                VStack(spacing: 2) {
                    Text("MOJO").font(.system(size: size*0.14, weight: .heavy, design: .rounded)).foregroundColor(.white)
                    Text(mode == .appDecides ? "APP" : "STEAL")
                        .font(.system(size: size*0.08, weight: .semibold, design: .rounded))
                        .foregroundColor(.white.opacity(0.9))
                }
            }
            .overlay(Circle().stroke(Color.white.opacity(0.25), lineWidth: 4))
            .scaleEffect(1 + 0.025 * min(1, rms*3))
            .gesture(DragGesture(minimumDistance: 0).onChanged { g in
                let center = CGPoint(x: geo.size.width/2, y: geo.size.height/2)
                let dx = g.location.x - center.x
                let dy = g.location.y - center.y
                var ang = atan2(dy, dx) * 180 / .pi + 90
                if ang < 0 { ang += 360 }
                let v = min(1, max(0, ang / 360))
                value = v; onChange?(v)
            })
        }
    }
}
EOS

# 5) Kill duplicates:
#    - comment any inline AppState/MoreMojoContainer/AudioProcessingView in ContentView.swift
if [ -f "$SRC/ContentView.swift" ]; then
  perl -0777 -pe 's/\n(\s*struct\s+AppState\s*:[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC/ContentView.swift"
  perl -0777 -pe 's/\n(\s*struct\s+MoreMojoContainer[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC/ContentView.swift"
  perl -0777 -pe 's/\n(\s*struct\s+AudioProcessingView[^}]+}\n)/\n\/\/ DUPLICATE REMOVED: $1\n/s' -i "$SRC/ContentView.swift"
fi
#    - remove/rename MoreMojoContainer_old.swift
if [ -f "$SRC/MoreMojoContainer_old.swift" ]; then
  echo "  - Removing $SRC/MoreMojoContainer_old.swift"
  rm -f "$SRC/MoreMojoContainer_old.swift"
fi

# 6) StealMojoPanel_SwiftOnly onApply type mismatch: ensure ProcessorParams
#    Replace any callback taking [String: Any] with ProcessorParams
grep -Rnl --include="*.swift" 'onApply:\s*\(\s*\[String:\s*Any\]\s*\)\s*->\s*Void' "$SRC" | while read -r f; do
  perl -0777 -pe 's/onApply:\s*\(\s*\[String:\s*Any\]\s*\)\s*->\s*Void/onApply: ((ProcessorParams)->Void)?/g' -i "$f"
done

# 7) InterpMode top-level usage: fix nested references
grep -Rnl --include="*.swift" 'ProcessorParams\.InterpMode' "$SRC" | while read -r f; do
  perl -0777 -pe 's/ProcessorParams\.InterpMode/InterpMode/g' -i "$f"
done
if [ -f "$SRC/Recommendations.swift" ]; then
  perl -0777 -pe 's/private\s+func\s+formatInterpolation\(\s*_?\s*mode:\s*ProcessorParams\.InterpMode\)/private func formatInterpolation(_ mode: InterpMode)/g' -i "$SRC/Recommendations.swift"
fi

echo "=== Done. Try: ./scripts/build_app.sh ==="
