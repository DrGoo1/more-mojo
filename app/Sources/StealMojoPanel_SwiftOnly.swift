
import SwiftUI
import AVFoundation

// macOS 11-safe prominent button style
struct PMXProminent: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .padding(.horizontal, 12).padding(.vertical, 6)
            .background(LinearGradient(colors: [.pink, .purple, .orange], startPoint: .leading, endPoint: .trailing))
            .foregroundColor(.white)
            .clipShape(Capsule())
            .opacity(configuration.isPressed ? 0.8 : 1.0)
    }
}

struct StealMojoPanel_SwiftOnly: View {
    @State private var refURL: URL?
    @State private var srcURL: URL?
    @State private var part: String = "bass"
    @State private var status: String = ""
    @State private var recommended: ProcessorParams? = nil

    var onApply: ((ProcessorParams)->Void)? = nil

    var body: some View {
        ZStack {
            LinearGradient(colors: [
                Color(red:1.00, green:0.35, blue:0.55),
                Color(red:1.00, green:0.82, blue:0.28),
                Color(red:0.40, green:0.95, blue:0.60),
                Color(red:0.28, green:0.72, blue:1.00)
            ], startPoint: .topLeading, endPoint: .bottomTrailing).ignoresSafeArea()

            VStack(alignment: .leading, spacing: 14) {
                HStack {
                    Image(systemName: "sparkles").foregroundColor(.white).font(.title2)
                    Text("Steal That Mojo™").font(.system(size: 24, weight: .heavy, design: .rounded)).foregroundColor(.white)
                }

                GroupBox {
                    VStack(alignment: .leading, spacing: 10) {
                        HStack {
                            Button("Choose Reference…") { chooseRef() }
                            .buttonStyle(PMXProminent())
                            Text(refURL?.lastPathComponent ?? "No reference")
                                .foregroundColor(.white.opacity(0.85))
                        }

                        HStack {
                            Button("Your Track (optional)…") { chooseSrc() }
                                .buttonStyle(PMXProminent())
                            Text(srcURL?.lastPathComponent ?? "(optional)")
                                .foregroundColor(.white.opacity(0.75))
                        }

                        Picker("Instrument", selection: $part) {
                            Text("Bass").tag("bass"); Text("Vocal").tag("vocal")
                            Text("Drums").tag("drums"); Text("Auto").tag("auto")
                        }
                        .pickerStyle(.segmented).frame(width: 420)

                        HStack {
                            Button("Analyze") { analyze() }
                                .buttonStyle(PMXProminent())
                                .disabled(refURL == nil)

                            if let rec = recommended {
                                Button("Apply to Mojo") { onApply?(rec) }
                                    .buttonStyle(PMXProminent())
                            }
                        }

                        if !status.isEmpty {
                            Text(status).foregroundColor(.white).font(.caption)
                        }
                    }
                }
                .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 12))
            }
            .padding(16)
    }

    private func chooseRef() {
        let p = NSOpenPanel(); p.allowedContentTypes = [.audio]; p.allowsMultipleSelection = false
        if p.runModal() == .OK { refURL = p.url }
    }
    private func chooseSrc() {
        let p = NSOpenPanel(); p.allowedContentTypes = [.audio]; p.allowsMultipleSelection = false
        if p.runModal() == .OK { srcURL = p.url }
    }

    private func analyze() {
        guard let ref = refURL else { return }
        status = "Analyzing…"
        DispatchQueue.global(qos: .userInitiated).async {
            do {
                // Swift-only HPSS separation + features
                let (harm, _, sr) = try SwiftMojoAnalyzer.separateHPSS(url: ref)
                let feats = SwiftMojoAnalyzer.features(from: harm, sr: sr)
                let rec = SwiftMojoAnalyzer.recommend(from: feats, part: part)
                var params = ProcessorParams()
                switch rec.interpMode.lowercased() {
                case "hq","hq sinc8x","sinc8x": params.interpMode = .hqSinc8x
                case "spline","transient spline 4x","spline4x": params.interpMode = .transientSpline4x
                case "adaptive": params.interpMode = .adaptive
                default: params.interpMode = .liveHB4x
                }
                params.drive = rec.drive; params.saturation = rec.saturation
                params.character = rec.character; params.presence = rec.presence
                params.mix = rec.mix; params.output = rec.output

                var eq: MojoEQMatch? = nil
                if let src = self.srcURL { eq = SwiftMojoAnalyzer.eqMatchBands(srcURL: src, refURL: ref) }

                DispatchQueue.main.async {
                    self.recommended = params
                    self.eqMatch = eq
                    self.status = "Analysis complete. Recommended Mojo ready."
                }
            } catch {
                DispatchQueue.main.async { self.status = "Analysis failed: \(error.localizedDescription)" }
            }
        }
    }
}
