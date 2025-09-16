import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
    public var warmthNormalized: Float { warmth }
}
