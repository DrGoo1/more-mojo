# ProGUI UI Scaffold — Progress Snapshot

Date: 2025-09-21 (Local)

## What’s Implemented

- **Common Shell**
  - Left/Right columns at 120 px each (temporary sizing)
  - **LED meters** (vector fallback) with 56 segments
  - **Input/Output level controls** (knob + slider)
  - **WaveformView** in the center with timeline grid and moving playhead
  - **TransportBar** centered under waveform (Play/Pause/Stop; UI-only)
  - **Master Bypass** toggle (top-left; UI-only dim overlay)
  - **Timeline Zoom**
    - Buttons: “- / +” centered in transport strip
    - Mouse wheel/trackpad zoom within `WaveformView`
    - Range: 1× → 16× (no panning yet; anchored to start)

- **Skin & Assets**
  - `SkinManager` detects skins and provides filmstrip frames if available
  - Auto-generation of button/switch frames when assets are missing
  - LED sprite optional; vector rendering is the fallback

- **Components Added**
  - `shared/ui_core/LedBarMeter.{h,cpp}` — Vector LED bar with peak/RMS + ballistics
  - `shared/ui_core/WaveformView.{h,cpp}` — Waveform, timeline, playhead, zoom
  - `shared/ui_core/TransportBar.h` — Play/Pause/Stop (UI-only)
  - `shared/ui_core/AmountKnobWithRing.h` — Medium knob with circumferential color ring
  - `Source/Components/ProMasterComponent.h` — Pro master with per-process rows
  - `Source/Components/StealMojoComponent.h` — Upload, instrument dropdown, amount, apply (UI-only)

## Professional GUI — Current Plan

- **Master Window**
  - Large **Amount** knob with circumferential level meter (centerpiece)
  - **Process list (short labels, in order):**
    - ISP, SRC, Quant/Dither, Align, Transient, De-esser, MLAR, Transformer
  - Each process row has a medium **Amount** knob + **Open** button (to subwindow)

- **Per-Process Subwindows**
  - Follow a shared template (Header, Body, Meter, Footer)
  - Controls and meters per `README_PRO_GUI.md` (Milestone 1 + MLAR/Transformer)

## Consumer GUI — Current Plan

- Main window with a large **More Mojo** knob
- Buttons: **More Like A Record** and **Steal The Mojo**
  - “Steal The Mojo” opens `StealMojoComponent` (upload file, instrument, amount, apply)

## Acceptance / Verification

- `ProGUI/scripts/acceptance.sh` builds TestApp and JUCE Runner, then captures screenshots and logs to `/tmp`
- Diagnostic overlays and logs enabled in `UIDemoComponent` to aid visibility during build issues

## Open Tasks (Next)

- Integrate **ProMasterComponent** into a Pro master window (UI-only)
- Integrate **Consumer** main window and wire **Steal The Mojo** button to open `StealMojoComponent`
- Add optional panning for `WaveformView` when zoomed > 1×
- Later: Audio engine integration for transport and shell controls

## Notes

- Windows appeared blank during some runs; acceptance build and screenshots completed successfully. Diagnostic guides/logs are present to aid validation regardless of window z-order.
- Sizing is not final; a responsive pass will follow once the full scaffold is visible.
