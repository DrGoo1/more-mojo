
import SwiftUI
import AVFoundation

struct StealMojoPanel_SwiftOnly: View {
    @State private var refURL: URL?
    @State private var srcURL: URL?
    @State private var part: String = "bass"
    @State private var status: String = ""
    @State private var recommended: ProcessorParams? = nil
    @State private var eqMatch: MojoEQMatch? = nil

    var onApply: ((ProcessorParams)->Void)? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Steal That Mojo™ (Self-contained)").font(.title3).foregroundColor(.white)
            HStack {
                Button("Choose Reference…") { chooseRef() }
                Text(refURL?.lastPathComponent ?? "No reference").foregroundColor(.white.opacity(0.8))
            }
            HStack {
                Button("Your Track for EQ Match…") { chooseSrc() }
                Text(srcURL?.lastPathComponent ?? "(optional)").foregroundColor(.white.opacity(0.6))
            }
            Picker("Instrument", selection: $part) {
                Text("Bass").tag("bass"); Text("Vocal").tag("vocal"); Text("Drums").tag("drums"); Text("Auto").tag("auto")
            }.pickerStyle(.segmented).frame(width: 420)

            HStack {
                Button("Analyze") { analyze() }.buttonStyle(.borderedProminent).disabled(refURL == nil)
                if let rec = recommended {
                    Button("Apply to Mojo") { onApply?(rec) }.buttonStyle(.bordered)
                }
            }
            if let eq = eqMatch, !eq.bands.isEmpty {
                Text("EQ Match Bands: (eq.bands.count)").font(.caption).foregroundColor(.white.opacity(0.8))
            }
            if !status.isEmpty { Text(status).foregroundColor(.white.opacity(0.9)).font(.caption) }
        }
        .padding(16).background(Color.black.opacity(0.9))
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
                DispatchQueue.main.async { self.status = "Analysis failed: (error.localizedDescription)" }
            }
        }
    }
}
