#!/usr/bin/env python3
import os, re, subprocess, sys, json, pathlib, shutil

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC  = ROOT / "app" / "Sources"
PLUGIN = ROOT / "plugin"
LOGS_DL = ROOT / "failed_artifacts"  # artifacts downloaded by the workflow
LOGS_CI = ROOT / "ci_logs"           # inline logs created by the build steps
SUMMARY = ROOT / "swarm_summary.md"

def sh(cmd, check=True):
    print(f"$ {cmd}")
    p = subprocess.run(cmd, shell=True, text=True)
    if check and p.returncode != 0:
        raise SystemExit(p.returncode)
    return p.returncode

def write(p: pathlib.Path, s: str) -> bool:
    p.parent.mkdir(parents=True, exist_ok=True)
    old = ""
    try: old = p.read_text()
    except: pass
    if old == s: return False
    p.write_text(s)
    print(f"wrote {p}")
    return True

def stage_all(): sh("git add -A", check=False)
def has_changes() -> bool: return subprocess.run("git diff --quiet", shell=True).returncode != 0

def read_globs(*globs):
    buf = ""
    for g in globs:
        for p in ROOT.glob(g):
            try: buf += p.read_text(errors="ignore")
            except: pass
    return buf

def read_artifact_logs():
    # app logs
    app = ""
    app += read_globs("ci_logs/xcodebuild_app_stdout.log",
                      "ci_logs/app_preflight.txt")
    app += read_globs("failed_artifacts/app-build-logs/**/xcodebuild_app_stdout.log",
                      "failed_artifacts/app-build-logs/**/app_preflight.txt",
                      "failed_artifacts/app-build-logs/**/*.log")
    # plugin logs
    plug = ""
    plug += read_globs("ci_logs/cmake_configure.log", "ci_logs/cmake_build.log")
    plug += read_globs("failed_artifacts/plugin-build-logs/**/cmake_configure.log",
                       "failed_artifacts/plugin-build-logs/**/cmake_build.log",
                       "failed_artifacts/plugin-build-logs/**/CMake*.log")
    return app, plug

# ------------------------ Agents ------------------------
class ProjectAgent:
    PATTERNS = [
        "future Xcode project file format",
        "Unable to read project",
        "The project .* cannot be opened because it is in a future Xcode project file format",
    ]
    @staticmethod
    def wants(app_logs:str)->bool:
        return any(pat in app_logs for pat in ProjectAgent.PATTERNS)
    @staticmethod
    def run():
        print("ProjectAgent: attempting XcodeGen regeneration (if project.yml exists)")
        sh("which xcodegen || (brew update || true; brew install xcodegen)", check=False)
        projy = ROOT/"app"/"project.yml"
        if projy.exists():
            sh(f"(cd {ROOT/'app'} && xcodegen generate)", check=False)

class SwiftAgent:
    KEYS = [
        "cannot find type 'ProcessorParams' in scope",
        "type 'MojoMacroMode' has no member 'app'",
        "buttonStyle(.borderedProminent)",
        "cannot infer contextual base in reference to member 'init'",
        "could not build Objective-C module 'CoreData'",
        "'Foundation/Foundation.h' file not found",
        "xcodebuild: error:",                # generic xcodebuild error
        "error: Scheme .* is not currently configured for the build action",
        "error: No shared schemes found"
    ]
    SHARED = """import Foundation
public enum InterpMode: Int, Codable, CaseIterable, Identifiable {
    case liveHB4x = 0, hqSinc8x, transientSpline4x, adaptive, aiAnalogHook
    public var id: Int { rawValue }
}
public struct ProcessorParams: Codable, Equatable {
    public var input: Float = 0.0
    public var output: Float = 0.0
    public var drive: Float = 0.55
    public var character: Float = 0.50
    public var saturation: Float = 0.45
    public var presence: Float = 0.50
    public var mix: Float = 1.00
    public var interpMode: InterpMode = .liveHB4x
    public var mode: Int = 1
}
public struct MojoEQBand: Codable, Equatable { public var lo: Float; public var hi: Float; public var gain_dB: Float }
public struct MojoEQMatch: Codable, Equatable { public var bands: [MojoEQBand] }
"""
    EXT = """import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
}
"""
    PMX = """struct PMXProminent: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .padding(.horizontal, 12).padding(.vertical, 6)
            .background(LinearGradient(colors: [.pink, .purple, .orange],
                                       startPoint: .leading, endPoint: .trailing))
            .foregroundColor(.white)
            .clipShape(Capsule())
            .opacity(configuration.isPressed ? 0.8 : 1.0)
    }
}
"""
    @staticmethod
    def wants(app_logs:str)->bool: return any(k in app_logs for k in SwiftAgent.KEYS)
    @staticmethod
    def run():
        changed = False
        shared = SRC/"SharedTypes.swift"
        changed |= write(shared, SwiftAgent.SHARED)
        # quarantine duplicate types in ProcessorParams.swift
        pp = SRC/"ProcessorParams.swift"
        if pp.exists() and re.search(r'\b(struct|enum)\s+(ProcessorParams|InterpMode)\b', pp.read_text()):
            (SRC/"ProcessorParams_DEPRECATED.swift").write_text("// DEPRECATED\n"+pp.read_text())
            pp.unlink(); changed = True
        # ensure extension file
        changed |= write(SRC/"ProcessorParams+Ext.swift", SwiftAgent.EXT)
        # normalize nested types and wheel enums
        for f in SRC.glob("*.swift"):
            t = f.read_text(errors="ignore")
            t2 = re.sub(r'ProcessorParams\.InterpMode', 'InterpMode', t)
            t2 = re.sub(r'\.app\b', '.appDecides', t2)
            t2 = re.sub(r'\.steal\b', '.stealMacro', t2)
            if t2 != t: f.write_text(t2); changed = True
        # EQ bands fix
        sms = SRC/"StealMojoSwift.swift"
        if sms.exists():
            t = sms.read_text(errors="ignore")
            if "bands.append(.init(" in t:
                t = t.replace("bands.append(.init(", "bands.append(MojoEQBand(")
                if "var bands: [MojoEQBand]" not in t:
                    t = re.sub(r'(bands\.append\(MojoEQBand\()', r'var bands: [MojoEQBand] = []\n\1', t, count=1)
                sms.write_text(t); changed = True
        # macOS 11 style
        panel = SRC/"StealMojoPanel_SwiftOnly.swift"
        if panel.exists():
            t = panel.read_text(errors="ignore")
            if ".buttonStyle(.borderedProminent)" in t:
                t = t.replace(".buttonStyle(.borderedProminent)", ".buttonStyle(PMXProminent())")
                if "struct PMXProminent" not in t:
                    t = t.replace("import AVFoundation", "import AVFoundation\n\n"+SwiftAgent.PMX)
                panel.write_text(t); changed = True
        # make engine observable (best effort)
        ae = SRC/"AudioEngine.swift"
        if ae.exists():
            t = ae.read_text(errors="ignore")
            if "ObservableObject" not in t:
                ae.write_text(t.replace("final class AudioEngine", "final class AudioEngine: ObservableObject")); changed=True
        if changed: stage_all()

class CMakeAgent:
    @staticmethod
    def wants(plugin_logs:str)->bool:
        return "TARGET_BUNDLE_DIR is allowed only for Bundle targets" in plugin_logs or "$<TARGET_BUNDLE_DIR:MoreMojoPlugin>" in plugin_logs
    @staticmethod
    def run():
        cmk = PLUGIN/"CMakeLists.txt"
        cmake_text = f"""cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)
find_package(JUCE CONFIG REQUIRED)
set(MOJO_FORMATS "AU;VST3;Standalone" CACHE STRING "Plugin formats to build")
juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS ${{MOJO_FORMATS}}
    PRODUCT_NAME "More Mojo by Umbo Gumbo"
    COPY_PLUGIN_AFTER_BUILD TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE)
target_sources(MoreMojoPlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h)
target_compile_definitions(MoreMojoPlugin PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(MoreMojoPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp)
"""
        changed = write(cmk, cmake_text)
        if changed:
            shutil.rmtree(PLUGIN/"build", ignore_errors=True)
            stage_all()

def main():
    # collect logs from artifacts using the new read_artifact_logs function
    app_log, plugin_log = read_artifact_logs()

    decisions = {
        "ProjectAgent": ProjectAgent.wants(app_log),
        "SwiftAgent":   SwiftAgent.wants(app_log),
        "CMakeAgent":   CMakeAgent.wants(plugin_log),
    }
    summary_lines = ["# Swarm decisions"]
    summary_lines.append(f"- ProjectAgent: {'YES' if decisions['ProjectAgent'] else 'no'}")
    summary_lines.append(f"- SwiftAgent:   {'YES' if decisions['SwiftAgent'] else 'no'}")
    summary_lines.append(f"- CMakeAgent:   {'YES' if decisions['CMakeAgent'] else 'no'}")
    summary_lines += ["", "## Hints", "- app log bytes: " + str(len(app_log)), "- plugin log bytes: " + str(len(plugin_log)), ""]

    acted = False
    if decisions["ProjectAgent"]: ProjectAgent.run(); summary_lines.append("- Ran ProjectAgent"); acted = True
    if decisions["SwiftAgent"]:   SwiftAgent.run();   summary_lines.append("- Ran SwiftAgent");   acted = True
    if decisions["CMakeAgent"]:   CMakeAgent.run();   summary_lines.append("- Ran CMakeAgent");   acted = True

    summary_lines.append(f"changes_staged = {'YES' if has_changes() else 'no'}; actions_ran = {'YES' if acted else 'no'}")
    write(SUMMARY, "\n".join(summary_lines))
    print("\n".join(summary_lines))
    sys.exit(0)

if __name__ == "__main__":
    main()
