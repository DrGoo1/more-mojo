#!/usr/bin/env python3
import os, re, subprocess, sys, pathlib, shutil

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC  = ROOT / "app" / "Sources"
PLUGIN = ROOT / "plugin"
LOGS_DL = ROOT / "failed_artifacts"   # downloaded artifacts
LOGS_CI = ROOT / "ci_logs"            # inline logs from build job
SUMMARY = ROOT / "swarm_summary.md"

def sh(cmd, check=True):
    print(f"$ {cmd}")
    p = subprocess.run(cmd, shell=True, text=True)
    if check and p.returncode != 0:
        raise SystemExit(p.returncode)
    return p.returncode

def write(p: pathlib.Path, s: str) -> bool:
    p.parent.mkdir(parents=True, exist_ok=True)
    old = p.read_text(errors="ignore") if p.exists() else ""
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

def first_match_excerpt(text:str, pattern:str, lines=20):
    try:
        m = re.search(pattern, text, flags=re.IGNORECASE|re.MULTILINE)
        if not m: return ""
        start = max(0, text.rfind("\n", 0, m.start() - 1))
        end   = text.find("\n", m.end())
        start = 0 if start < 0 else start
        end   = len(text) if end < 0 else end
        # expand to N lines around
        before = text[:m.start()].splitlines()[-lines:]
        after  = text[m.end():].splitlines()[:lines]
        return "\n".join(before + [text[m.start():m.end()]] + after)
    except Exception:
        return ""

# ------------------------ Agents ------------------------

class ProjectAgent:
    """Fix future project format via XcodeGen."""
    PATTERNS = [
        "future Xcode project file format",
        "Unable to read project",
        "cannot be opened because it is in a future Xcode project file format",
    ]
    @staticmethod
    def wants(app_logs:str)->bool:
        return any(pat in app_logs for pat in ProjectAgent.PATTERNS)
    @staticmethod
    def run():
        print("ProjectAgent: XcodeGen regeneration (if app/project.yml exists)")
        sh("which xcodegen || (brew update || true; brew install xcodegen)", check=False)
        projy = ROOT/"app"/"project.yml"
        if projy.exists():
            sh(f"(cd {ROOT/'app'} && xcodegen generate)", check=False)

class SchemeAgent:
    """Fix missing/unshared scheme by regenerating the project with XcodeGen."""
    PATTERNS = [
        r"Scheme .+ is not currently configured for the build action",
        r"No shared schemes found",
        r"xcodebuild: error:",
    ]
    @staticmethod
    def wants(app_logs:str)->bool:
        return any(re.search(p, app_logs, flags=re.IGNORECASE) for p in SchemeAgent.PATTERNS)
    @staticmethod
    def run():
        print("SchemeAgent: ensuring shared scheme via XcodeGen (if project.yml exists)")
        sh("which xcodegen || (brew update || true; brew install xcodegen)", check=False)
        projy = ROOT/"app"/"project.yml"
        if projy.exists():
            sh(f"(cd {ROOT/'app'} && xcodegen generate)", check=False)
        # no-op if project.yml absent (workflow step already chooses first available scheme)

class SwiftAgent:
    """Unify SharedTypes / fix wheel cases / macOS 11 style / EQ bands."""
    KEYS = [
        "cannot find type 'ProcessorParams' in scope",
        "type 'MojoMacroMode' has no member 'app'",
        "buttonStyle(.borderedProminent)",
        "cannot infer contextual base in reference to member 'init'",
        "could not build Objective-C module 'CoreData'",
        "'Foundation/Foundation.h' file not found",
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
    EXT   = "import Foundation\nextension ProcessorParams { public var outputNormalized: Float { (output + 12) / 24 } }\n"
    PMX   = "struct PMXProminent: ButtonStyle { func makeBody(configuration: Configuration) -> some View { configuration.label.padding(.horizontal, 12).padding(.vertical, 6).background(LinearGradient(colors: [.pink, .purple, .orange], startPoint: .leading, endPoint: .trailing)).foregroundColor(.white).clipShape(Capsule()).opacity(configuration.isPressed ? 0.8 : 1.0) } }\n"
    @staticmethod
    def wants(app_logs:str)->bool: return any(k in app_logs for k in SwiftAgent.KEYS)
    @staticmethod
    def run():
        changed = False
        changed |= write(SRC/"SharedTypes.swift", SwiftAgent.SHARED)
        # quarantine duplicates
        pp = SRC/"ProcessorParams.swift"
        if pp.exists() and re.search(r'\b(struct|enum)\s+(ProcessorParams|InterpMode)\b', pp.read_text(errors="ignore")):
            (SRC/"ProcessorParams_DEPRECATED.swift").write_text("// DEPRECATED\n"+pp.read_text(errors="ignore"))
            pp.unlink(); changed = True
        changed |= write(SRC/"ProcessorParams+Ext.swift", SwiftAgent.EXT)
        # normalize nested refs & wheel enums
        for f in SRC.glob("*.swift"):
            t = f.read_text(errors="ignore"); t2 = t
            t2 = re.sub(r'ProcessorParams\.InterpMode', 'InterpMode', t2)
            t2 = re.sub(r'\.app\b', '.appDecides', t2)
            t2 = re.sub(r'\.steal\b', '.stealMacro', t2)
            if t2 != t: f.write_text(t2); changed = True
        # EQ bands
        sms = SRC/"StealMojoSwift.swift"
        if sms.exists():
            t = sms.read_text(errors="ignore"); t2 = t
            t2 = t2.replace("bands.append(.init(", "bands.append(MojoEQBand(")
            if "bands.append(MojoEQBand(" in t2 and "var bands: [MojoEQBand]" not in t2:
                t2 = re.sub(r'(bands\.append\(MojoEQBand\()', r'var bands: [MojoEQBand] = []\n\1', t2, count=1)
            if t2 != t: sms.write_text(t2); changed=True
        # macOS 11 style
        panel = SRC/"StealMojoPanel_SwiftOnly.swift"
        if panel.exists():
            t = panel.read_text(errors="ignore"); t2 = t
            t2 = t2.replace(".buttonStyle(.borderedProminent)", ".buttonStyle(PMXProminent())")
            if ".buttonStyle(PMXProminent())" in t2 and "struct PMXProminent" not in t2:
                t2 = t2.replace("import AVFoundation", "import AVFoundation\n\n"+SwiftAgent.PMX)
            if t2 != t: panel.write_text(t2); changed=True
        if changed: stage_all()

class CMakeAgent:
    @staticmethod
    def wants(plugin_logs:str)->bool:
        return "TARGET_BUNDLE_DIR is allowed only for Bundle targets" in plugin_logs or "$<TARGET_BUNDLE_DIR:MoreMojoPlugin>" in plugin_logs
    @staticmethod
    def run():
        cmk = PLUGIN/"CMakeLists.txt"
        cmake_text = """cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(MoreMojoPlugin VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)
find_package(JUCE CONFIG REQUIRED)
set(MOJO_FORMATS "AU;VST3;Standalone" CACHE STRING "Plugin formats to build")
juce_add_plugin(MoreMojoPlugin
    COMPANY_NAME "Umbo Gumbo"
    FORMATS ${MOJO_FORMATS}
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
    # read logs from artifacts and inline
    app_log = read_globs("ci_logs/xcodebuild_app_stdout.log",
                         "ci_logs/app_preflight.txt",
                         "failed_artifacts/app-build-logs/**/xcodebuild_app_stdout.log",
                         "failed_artifacts/app-build-logs/**/app_preflight.txt",
                         "failed_artifacts/app-build-logs/**/*.log")
    plugin_log = read_globs("ci_logs/cmake_configure.log",
                            "ci_logs/cmake_build.log",
                            "failed_artifacts/plugin-build-logs/**/cmake_configure.log",
                            "failed_artifacts/plugin-build-logs/**/cmake_build.log",
                            "failed_artifacts/plugin-build-logs/**/CMake*.log")

    # decisions
    decisions = {
        "ProjectAgent": ProjectAgent.wants(app_log),
        "SchemeAgent":  SchemeAgent.wants(app_log),
        "SwiftAgent":   SwiftAgent.wants(app_log),
        "CMakeAgent":   CMakeAgent.wants(plugin_log),
    }
    summary = ["# Swarm decisions"]
    for k,v in decisions.items(): summary.append(f"- {k}: {'YES' if v else 'no'}")
    summary += ["", "## Hints",
                f"- app log bytes: {len(app_log)}",
                f"- plugin log bytes: {len(plugin_log)}", ""]

    acted = False
    if decisions["ProjectAgent"]: ProjectAgent.run(); summary.append("- Ran ProjectAgent"); acted=True
    if decisions["SchemeAgent"]:  SchemeAgent.run();  summary.append("- Ran SchemeAgent");  acted=True
    if decisions["SwiftAgent"]:   SwiftAgent.run();   summary.append("- Ran SwiftAgent");   acted=True
    if decisions["CMakeAgent"]:   CMakeAgent.run();   summary.append("- Ran CMakeAgent");   acted=True

    # small excerpt to aid debugging
    if decisions["ProjectAgent"] or decisions["SchemeAgent"]:
        exc = first_match_excerpt(app_log, r"(future Xcode project file format|Scheme .+ not currently configured|No shared schemes found|xcodebuild: error:)")
        if exc:
            summary += ["", "## Excerpt", "```", exc, "```"]

    summary.append(f"\nchanges_staged = {'YES' if has_changes() else 'no'}; actions_ran = {'YES' if acted else 'no'}")
    write(SUMMARY, "\n".join(summary))
    print("\n".join(summary))
    sys.exit(0)

if __name__ == "__main__":
    main()
