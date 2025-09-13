import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
}
