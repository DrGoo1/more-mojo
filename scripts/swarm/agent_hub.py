#!/usr/bin/env python3
import os, re, subprocess, sys, json, pathlib, shutil

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC  = ROOT / "app" / "Sources"
PLUGIN = ROOT / "plugin"
LOGS = ROOT / "failed_artifacts"  # when run by the failure workflow
CI_LOGS = ROOT / "ci_logs"        # when run inline

def sh(cmd, check=True):
    print(f"$ {cmd}")
    p = subprocess.run(cmd, shell=True, text=True)
    if check and p.returncode != 0:
        raise SystemExit(p.returncode)
    return p.returncode

def read_text(path):
    try:
        return pathlib.Path(path).read_text(errors="ignore")
    except FileNotFoundError:
        return ""

def write(path, text):
    p = pathlib.Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)
    if p.exists() and p.read_text() == text:
        return False
    p.write_text(text)
    print(f"wrote {p}")
    return True

def stage_all():
    sh("git add -A", check=False)

def has_changes():
    return subprocess.run("git diff --quiet", shell=True).returncode != 0

# ------------------------ Agents ------------------------

class ProjectAgent:
    """Fix Xcode project format mismatch via XcodeGen; ensure scheme exists."""
    @staticmethod
    def wants(app_logs:str)->bool:
        return "future Xcode project file format" in app_logs or "Unable to read project" in app_logs

    @staticmethod
    def run():
        # ensure XcodeGen available
        sh("which xcodegen || (brew update || true; brew install xcodegen)", check=False)
        # attempt regeneration if project.yml exists
        projy = ROOT/"app"/"project.yml"
        if projy.exists():
            sh(f"(cd {ROOT/'app'} && xcodegen generate)", check=False)
        # make build script robust to codesign issues
        # (build script already handled in existing workflow; no-op here)

class SwiftAgent:
    """Unify SharedTypes + fix common Swift compile errors."""
    SHARED_TYPES = """import Foundation
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
    EXT_TYPES = """import Foundation
extension ProcessorParams {
    public var outputNormalized: Float { (output + 12) / 24 }
}
"""
    PMX_STYLE = """struct PMXProminent: ButtonStyle {
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
    def wants(app_logs:str)->bool:
        keys = [
          "cannot find type 'ProcessorParams' in scope",
          "type 'MojoMacroMode' has no member 'app'",
          "buttonStyle(.borderedProminent)",
          "cannot infer contextual base in reference to member 'init'",
          "could not build Objective-C module 'CoreData'",
          "'Foundation/Foundation.h' file not found"
        ]
        return any(k in app_logs for k in keys)

    @staticmethod
    def run():
        changed = False
        # Canonical SharedTypes.swift
        changed |= write(SRC/"SharedTypes.swift", SwiftAgent.SHARED_TYPES)
        # Ensure extension-only file exists
        changed |= write(SRC/"ProcessorParams+Ext.swift", SwiftAgent.EXT_TYPES)
        # If duplicate definitions exist in ProcessorParams.swift, quarantine
        pp = SRC/"ProcessorParams.swift"
        if pp.exists() and re.search(r'\b(struct|enum)\s+(ProcessorParams|InterpMode)\b', pp.read_text()):
            (SRC/"ProcessorParams_DEPRECATED.swift").write_text(
                "// DEPRECATED duplicate removed\n" + pp.read_text()
            )
            pp.unlink()
            changed = True
        # Normalize nested references
        for f in SRC.glob("*.swift"):
            t = f.read_text()
            t2 = re.sub(r'ProcessorParams\.InterpMode', 'InterpMode', t)
            t2 = re.sub(r'\.app\b', '.appDecides', t2)
            t2 = re.sub(r'\.steal\b', '.stealMacro', t2)
            if t2 != t:
                f.write_text(t2); changed = True

        # Fix StealMojoSwift.swift bands
        sms = SRC/"StealMojoSwift.swift"
        if sms.exists():
            t = sms.read_text()
            if "bands.append(.init(" in t or "bands.append(MojoEQBand(" not in t:
                t = t.replace("bands.append(.init(", "bands.append(MojoEQBand(")
                if "var bands: [MojoEQBand]" not in t:
                    # insert explicit declaration before first append
                    t = re.sub(r'(bands\.append\(MojoEQBand\()', r'var bands: [MojoEQBand] = []\n\1', t, count=1)
                sms.write_text(t); changed = True

        # macOS 11 button style
        panel = SRC/"StealMojoPanel_SwiftOnly.swift"
        if panel.exists():
            t = panel.read_text()
            if ".buttonStyle(.borderedProminent)" in t:
                t = t.replace(".buttonStyle(.borderedProminent)", ".buttonStyle(PMXProminent())")
                if "struct PMXProminent" not in t:
                    t = t.replace("import AVFoundation", "import AVFoundation\n\n"+SwiftAgent.PMX_STYLE)
                panel.write_text(t); changed = True

        # Ensure AudioEngine is ObservableObject? (best-effort)
        ae = SRC/"AudioEngine.swift"
        if ae.exists():
            t = ae.read_text()
            if "ObservableObject" not in t:
                t = t.replace("final class AudioEngine", "final class AudioEngine: ObservableObject")
                ae.write_text(t); changed = True

        if changed:
            stage_all()

class CMakeAgent:
    """Clean CMakeLists to Option-A, wipe cache."""
    CMAKE_OPT_A = """cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
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

    @staticmethod
    def wants(plugin_logs:str)->bool:
        return "$<TARGET_BUNDLE_DIR:MoreMojoPlugin>" in plugin_logs or "TARGET_BUNDLE_DIR is allowed only for Bundle targets" in plugin_logs

    @staticmethod
    def run():
        changed = write(PLUGIN/"CMakeLists.txt", CMakeAgent.CMAKE_OPT_A)
        if changed:
            # remove cached build to purge stale post-build commands
            shutil.rmtree(PLUGIN/"build", ignore_errors=True)
            stage_all()

class GuardAgent:
    """Inject or verify guardrails in workflow to stop regressions."""
    @staticmethod
    def run():
        wf = ROOT/".github"/"workflows"/"build_with_logs.yml"
        txt = read_text(wf)
        if not txt: return
        changed = False
        if "Guard against raw swiftc usage" not in txt:
            # minimal insertion hint: add a grep guard before app build
            txt = txt.replace("steps:\n      - uses: actions/checkout@v4",
                              "steps:\n      - uses: actions/checkout@v4\n\n      - name: Guard against raw swiftc usage\n        run: |\n          set -euo pipefail\n          if [ -f build_part4_swift.sh ]; then echo \"❌ remove build_part4_swift.sh\"; exit 1; fi\n          BAD=$(grep -RIn --exclude-dir=.git --exclude-dir=plugin --include=\"*.sh\" -E '^[[:space:]]*swiftc\\b' || true)\n          if [ -n \"$BAD\" ]; then echo \"❌ Found swiftc: $BAD\"; exit 1; fi")
            changed = True
        if "Guard CMake for forbidden commands" not in txt:
            txt = txt.replace("steps:\n      - uses: actions/checkout@v4",
                              "steps:\n      - uses: actions/checkout@v4\n\n      - name: Guard CMake for forbidden commands\n        run: |\n          set -euo pipefail\n          if grep -RIn '\\$<TARGET_BUNDLE_DIR:MoreMojoPlugin>' plugin/CMakeLists.txt; then echo \"❌ forbidden generator expression\"; exit 1; fi",
                              1)
            changed = True
        if changed:
            write(wf, txt); stage_all()

# ------------------------ Router ------------------------

def main():
    # ingest logs (either inline ci_logs or downloaded failed_artifacts)
    app_log = ""
    if (CI_LOGS/"xcodebuild_app_stdout.log").exists():
        app_log = (CI_LOGS/"xcodebuild_app_stdout.log").read_text(errors="ignore")
    elif (LOGS).exists():
        # look for app-build-logs zip unpack; best effort
        for p in LOGS.glob("**/app_preflight.txt"):
            app_log += p.read_text(errors="ignore")
        for p in LOGS.glob("**/xcodebuild_app_stdout.log"):
            app_log += p.read_text(errors="ignore")

    plugin_log = ""
    if (CI_LOGS/"cmake_configure.log").exists():
        plugin_log = (CI_LOGS/"cmake_configure.log").read_text(errors="ignore")
    elif LOGS.exists():
        for p in LOGS.glob("**/cmake_configure.log"):
            plugin_log += p.read_text(errors="ignore")
        for p in LOGS.glob("**/cmake_build.log"):
            plugin_log += p.read_text(errors="ignore")

    ran = False
    if ProjectAgent.wants(app_log): ProjectAgent.run(); ran = True
    if SwiftAgent.wants(app_log):   SwiftAgent.run();   ran = True
    if CMakeAgent.wants(plugin_log):CMakeAgent.run();   ran = True
    GuardAgent.run()

    print(f"AgentHub finished. changes_staged={has_changes()} actions_ran={ran}")
    sys.exit(0)

if __name__ == "__main__":
    main()
