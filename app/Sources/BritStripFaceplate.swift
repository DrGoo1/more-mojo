import SwiftUI

struct BritStripFaceplate<Content: View>: View {
    let layout: FaceplateLayout
    @ViewBuilder var content: Content

    var body: some View {
        GeometryReader { geo in
            let baseAR = layout.baseWidth / layout.baseHeight
            let liveAR = geo.size.width / geo.size.height
            let drawSize: CGSize = (liveAR > baseAR)
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
