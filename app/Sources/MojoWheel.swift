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
