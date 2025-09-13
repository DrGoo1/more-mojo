import Foundation

// ProcessorParams moved to SharedTypes.swift - keeping this file for backward compatibility
// and to prevent build errors from references in the project

// This will cause a compile error if SharedTypes already has ProcessorParams
#if !SHARED_TYPES_DEFINED
public // DEPRECATED duplicate removed
// struct ProcessorParams: Codable, Equatable {
    public var drive: Float = 0.5
    public var tone: Float = 0.5
    public var output: Float = 0.5
    public var mix: Float = 1.0
    
    public init(drive: Float = 0.5, tone: Float = 0.5, output: Float = 0.5, mix: Float = 1.0) {
        self.drive = drive
        self.tone = tone
        self.output = output
        self.mix = mix
    }

    
    public static func ==(lhs: ProcessorParams, rhs: ProcessorParams) -> Bool {
        return lhs.drive == rhs.drive && lhs.tone == rhs.tone && 
               lhs.output == rhs.output && lhs.mix == rhs.mix
    }
}
#endif
