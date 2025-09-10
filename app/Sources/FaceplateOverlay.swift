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
