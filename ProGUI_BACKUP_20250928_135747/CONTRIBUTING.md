# Contributing Guide (MoreMojo ProGUI)

This guide defines how multiple agents (and humans) collaborate safely and productively.

## Ownership & Directory Map

- `ProGUI/Tests/TestApp/` — Swift host (SwiftUI) and standalone Cocoa window
  - Owner: swift-host
- `ProGUI/SwiftBridge/` — Swift <-> C API bridge
  - Owner: swift-bridge
- `ProGUI/Source/` — JUCE C++ wrapper and demo component
  - Owner: juce-ui
- `ProGUI/shared/ui_core/` — UI primitives, theme, SkinManager
  - Owner: ui-core
- `ProGUI/Tools/JuceRunner/` — Pure-JUCE runner (CMake)
  - Owner: juce-runner
- `.github/workflows/` — CI workflows
  - Owner: ci
- `ProGUI/scripts/` — helper scripts (screenshots, acceptance, etc.)
  - Owner: tooling

## Branching Strategy

- Create a branch per feature: `feature/<short-description>`
- Keep PRs small and focused (<= 300 LOC ideally)
- Rebase on `main` and resolve conflicts locally before opening PR

## Required CI Checks (PR Gates)

- Build JUCE Runner (CMake) and Swift TestApp
- Run acceptance checks:
  - Launch apps
  - Capture screenshots
  - Ensure screenshots exceed a minimal byte-size threshold (non-blank)
- Lint/format (future extension)

## Commit Messages

- Use descriptive messages; prefix with area when relevant, e.g. `bridge:`, `ui:`, `ci:`, `runner:`
- Example: `ui: add filmic knob and glow slider; wire to demo`

## Coding Guidelines

- Swift: prefer explicit logging via `NSLog` and local file `/tmp/progui_demo_swift.log`
- C++: use `/tmp/progui_demo.log` for component/wrapper logs
- Avoid heavy allocations in `paint()`; batch paths when possible; throttle timers to ~30 Hz for meters
- Keep attach/resize semantics deterministic: `addToDesktop(0, parentNSView)`, `setVisible(true)`, `toFront(true)`, `setBounds(getLocalBounds())`

## Review Checklist

- App launches without crash
- Visual header appears: `[Demo] MoreMojo Pro GUI — UIDemoComponent`
- Minimal control set visible (knobs, slider, toggle, meters)
- No excessive warnings; no new TODOs without corresponding issues

## Reporting Issues

Use the GitHub issue templates (`Bug report`, `Feature request`). Include logs and screenshots:
- `/tmp/progui_demo_swift.log`, `/tmp/progui_demo.log`
- OS version, device, steps to reproduce
