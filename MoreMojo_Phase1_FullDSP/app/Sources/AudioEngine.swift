
import Foundation
import AVFoundation
import Accelerate

final class AudioEngine: ObservableObject {
    @Published var rmsOut: Float = 0
    @Published var spectrum: [Float] = Array(repeating: 0, count: 128)

    private let engine = AVAudioEngine()
    private var source: AVAudioSourceNode!

    private let meter = Metering(fftSize: 1024)
    private let integrity = IntegrityCalculator()

    private let hb4 = HB4x()
    private let sinc8 = Sinc8x()
    private let spline4 = TransientSpline4x()
    private let adaptive = AdaptiveInterp()
    private let analog = RealFeelProcessorPro()

    private var file: AVAudioFile?

    private var paramsCurrent = ProcessorParams()
    private var paramsRT = ProcessorParams()
    private var paramsDirty: UInt32 = 0

    private var mixSmooth: Float = 1.0
    private var outSmooth: Float = 1.0

    init() {
        setupSource()
        try? engine.start()
    }

    func setParams(_ p: ProcessorParams) { paramsCurrent = p; paramsDirty &+= 1 }

    func load(url: URL) throws { file = try AVAudioFile(forReading: url) }

    private func setupSource() {
        source = AVAudioSourceNode { [weak self] _, _, frameCount, abl in
            guard let self = self else { return noErr }
            if self.paramsDirty != 0 { self.paramsRT = self.paramsCurrent; self.paramsDirty = 0 }

            let n = Int(frameCount)
            let ablPtr = UnsafeMutableAudioBufferListPointer(abl)
            let outL = ablPtr[0].mData!.assumingMemoryBound(to: Float.self)
            let outR = (ablPtr.count > 1 ? ablPtr[1].mData! : ablPtr[0].mData!).assumingMemoryBound(to: Float.self)

            // For demo: generate dry input as silence; pipeline still runs. Replace with player decode for full app.
            var inL = [Float](repeating: 0, count: n)
            var inR = [Float](repeating: 0, count: n)

            // Interpolation/OS
            var dnL = [Float](), dnR = [Float]()
            switch self.paramsRT.interpMode {
            case .liveHB4x:
                let upN = n * 4
                var upL=[Float](repeating:0,count:upN), upR=upL
                self.hb4.up(&inL, &inR, n, &upL, &upR)
                upL.withUnsafeMutableBufferPointer { L in
                    upR.withUnsafeMutableBufferPointer { R in
                        self.analog.processStereo(L.baseAddress!, R.baseAddress!,
                            frames: upN,
                            mode: RFMode(rawValue: Int(self.paramsRT.mode)) ?? .vintage,
                            intensity: self.paramsRT.drive,
                            character: self.paramsRT.character,
                            presence: self.paramsRT.presence)
                    }
                }
                self.hb4.down(upL, upR, upN, &dnL, &dnR)
            case .hqSinc8x:
                let upN = n * 8
                var upL=[Float](repeating:0,count:upN), upR=upL
                self.sinc8.up(&inL, &inR, n, &upL, &upR)
                upL.withUnsafeMutableBufferPointer { L in
                    upR.withUnsafeMutableBufferPointer { R in
                        self.analog.processStereo(L.baseAddress!, R.baseAddress!,
                            frames: upN,
                            mode: RFMode(rawValue: Int(self.paramsRT.mode)) ?? .vintage,
                            intensity: self.paramsRT.drive,
                            character: self.paramsRT.character,
                            presence: self.paramsRT.presence)
                    }
                }
                self.sinc8.down(upL, upR, upN, &dnL, &dnR)
            case .transientSpline4x:
                let upN = n * 4
                var upL=[Float](repeating:0,count:upN), upR=upL
                self.spline4.process(&inL, &inR, n, &upL, &upR)
                upL.withUnsafeMutableBufferPointer { L in
                    upR.withUnsafeMutableBufferPointer { R in
                        self.analog.processStereo(L.baseAddress!, R.baseAddress!,
                            frames: upN,
                            mode: RFMode(rawValue: Int(self.paramsRT.mode)) ?? .vintage,
                            intensity: self.paramsRT.drive,
                            character: self.paramsRT.character,
                            presence: self.paramsRT.presence)
                    }
                }
                self.hb4.down(upL, upR, upN, &dnL, &dnR)
            case .adaptive:
                self.adaptive.process(&inL, &inR, n, &dnL, &dnR) { upL, upR, upN in
                    upL.withUnsafeMutableBufferPointer { L in
                        upR.withUnsafeMutableBufferPointer { R in
                            self.analog.processStereo(L.baseAddress!, R.baseAddress!,
                                frames: upN,
                                mode: RFMode(rawValue: Int(self.paramsRT.mode)) ?? .vintage,
                                intensity: self.paramsRT.drive,
                                character: self.paramsRT.character,
                                presence: self.paramsRT.presence)
                        }
                    }
                }
            case .aiAnalogHook:
                // For now: HQ path without AI (bypass unless model present)
                let upN = n * 8
                var upL=[Float](repeating:0,count:upN), upR=upL
                self.sinc8.up(&inL, &inR, n, &upL, &upR)
                upL.withUnsafeMutableBufferPointer { L in
                    upR.withUnsafeMutableBufferPointer { R in
                        self.analog.processStereo(L.baseAddress!, R.baseAddress!,
                            frames: upN,
                            mode: RFMode(rawValue: Int(self.paramsRT.mode)) ?? .vintage,
                            intensity: self.paramsRT.drive,
                            character: self.paramsRT.character,
                            presence: self.paramsRT.presence)
                    }
                }
                self.sinc8.down(upL, upR, upN, &dnL, &dnR)
            }

            // Mix & output (smoothed)
            let s: Float = 0.05
            self.mixSmooth += s * (self.paramsRT.mix - self.mixSmooth)
            let targetGain = powf(10, self.paramsRT.output / 20)
            self.outSmooth += s * (targetGain - self.outSmooth)
            let wetMix = self.mixSmooth, dryMix: Float = 1 - wetMix, gain = self.outSmooth

            for i in 0..<n {
                let oL = (dryMix * inL[i] + wetMix * dnL[i]) * gain
                let oR = (dryMix * inR[i] + wetMix * dnR[i]) * gain
                outL[i] = oL; outR[i] = oR
            }

            // Post-analysis
            let fmt = AVAudioFormat(standardFormatWithSampleRate: 48000, channels: 2)!
            if let post = AVAudioPCMBuffer(pcmFormat: fmt, frameCapacity: AVAudioFrameCount(n)) {
                post.frameLength = AVAudioFrameCount(n)
                let pl = post.floatChannelData![0], pr = post.floatChannelData![1]
                outL.withMemoryRebound(to: Float.self, capacity: n) { src in pl.assign(from: src, count: n) }
                outR.withMemoryRebound(to: Float.self, capacity: n) { src in pr.assign(from: src, count: n) }
                self.meter.analyze(post)
                let snap = self.meter.snapshot()
                DispatchQueue.main.async {
                    self.rmsOut = snap.rms
                    self.spectrum = Array(snap.spectrum.prefix(self.spectrum.count))
                    _ = self.integrity.update(output: post, meter: snap)
                }
            }
            return noErr
        }
        engine.attach(source)
        engine.connect(source, to: engine.mainMixerNode, format: nil)
    }
}
