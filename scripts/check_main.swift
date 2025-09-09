import Foundation

// Usage: swift check_main.swift <SourcesDir>
let args = CommandLine.arguments.dropFirst()
guard let dir = args.first else {
    fputs("Usage: swift check_main.swift <SourcesDir>\n", stderr)
    exit(2)
}

let fm = FileManager.default
guard let e = fm.enumerator(atPath: dir) else { exit(0) }
var mainFiles: [String] = []
while let x = e.nextObject() as? String {
    if x.hasSuffix(".swift") {
        let p = (dir as NSString).appendingPathComponent(x)
        if let s = try? String(contentsOfFile: p, encoding: .utf8),
           s.contains("@main") {
            mainFiles.append(p)
        }
    }
}

if mainFiles.count == 0 {
    fputs("ERROR: No @main struct found.\n", stderr); exit(1)
}
if mainFiles.count > 1 {
    fputs("ERROR: Multiple @main found:\n", stderr)
    mainFiles.forEach { fputs(" - \($0)\n", stderr) }
    exit(1)
}
// Check the main file contains "struct MoreMojoStudioApp: App"
let mainFile = mainFiles.first!
if let content = try? String(contentsOfFile: mainFile, encoding: .utf8),
   content.range(of: #"struct\s+MoreMojoStudioApp\s*:\s*App"#, options: .regularExpression) == nil {
    fputs("ERROR: @main struct must be MoreMojoStudioApp in MoreMojoStudioApp.swift\n", stderr)
    fputs("Offending file: \(mainFile)\n", stderr); exit(1)
}
print("OK: @main validation passed")
