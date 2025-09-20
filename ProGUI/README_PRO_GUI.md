# MoreMojo Professional GUI — Spec (Milestone 1)

This README captures the Professional GUI scope starting with lower‑level processes and scalability (CPU/GPU, realtime vs. offline). It also defines presets, bakeability, and global UI requirements.

## Architecture
- shared/ui_core/: theme, primitives, 3D controls (header‑only)
- ui_pro/: professional subwindows (to be added), using ui_core
- Each subwindow uses one template: Header (title/preset/amount), Body (controls), Meter column, Footer (waveform/transport/IO/Bake)

## Global UI (every subwindow)
- Waveform strip (scroll/zoom, selection) + Transport (Play/Pause, Stop, Loop, A/B)
- IO trims + IO meters (L/R)
- Preset dropdown (Instrument, Buss, Master, Other)
- Explanation text (what the process does)
- Engine selector: Auto | CPU | GPU
- Quality selector: Realtime (low‑latency) | Offline (high‑quality)
- Bake It In: render to NEW audio file (selection/all; quality; format/bit‑depth)

## Bakeability
- Bakeable: all DSP processes below
- Not bakeable: meters/analysis only

## Milestone 1 — Lower‑Level Processes (with CPU/GPU + Realtime/Offline)
1) Intersample Interpolation / True‑Peak (ISP) — INCLUDED
   - Controls: OS Factor (2×/4×/8×), Filter (linear/min/polyphase), Passband Rolloff, Stopband Atten, TP Ceiling, Lookahead (ms)
   - Meters: True‑Peak, ISP count, Ceiling margin
   - Engine: CPU (vDSP/polyphase, vector TP); GPU (Metal OS/TP scan for high OS)
   - Quality: Realtime (2×–4×, short FIR) vs Offline (8×, long FIR)
2) Resampler / SRC
   - Controls: Target SR, Quality (Fast/HQ/Extreme), Passband Ripple, Stopband Atten, Phase (lin/min/mixed)
   - Meters: Aliasing residual, Latency
   - Engine: CPU (vDSP/libsamplerate‑like); GPU (Metal polyphase at high OS)
   - Quality: Realtime (shorter FIR) vs Offline (long FIR)
3) Jitter & Accumulation (Quant/Dither)
   - Controls: Jitter RMS, Jitter Spectrum, Accum Bit‑Depth, Quant Mode, Dither Type, Dither Level, Noise Floor Target
   - Meters: Noise spectrum, THD+N, Time‑variance
   - Engine: CPU (SIMD), GPU not required
4) Align (Phase/Time)
   - Controls: Delay (samples/ms), Polarity, Phase Rotate, Tilt AP, Crossover, Auto‑Align, Link
   - Meters: Correlation, Phase scope, IACC timeline, Group delay
   - Engine: CPU FFT/AP; GPU optional for big FFTs
5) Transient Detect / Shaper
   - Controls: Sensitivity, Attack/Hold/Release (ms), Boost/Soften, HF Focus, Band Mode (full/split)
   - Meters: Transient timeline, Before/After waveform, Spectral emphasis
   - Engine: CPU; SIMD friendly
6) De‑esser / HF Control
   - Controls: Freq, Q/BW, Threshold, Amount, Mode (split/full), Listen
   - Meters: Sibilance activity, HF spectrum
   - Engine: CPU

## Presets (per process)
- Categories: Instrument, Buss, Master, Other
- Preset applies a full parameter set; user presets supported; saved with sessions

## Process Amount (global overview)
- Each process reports 0–100% amount (semantics per process, e.g., TP proximity/GR for ISP, H2/H3 energy for Transformer, etc.)
- Overview panel shows per‑process amount bars + bypass toggles

## Realtime vs Offline — Guidance
- ISP: Realtime 2×–4×, short FIR; Offline 8×, longer FIR + precise TP
- SRC: Realtime shorter FIR; Offline extreme FIR/low ripple
- Jitter/Dither: same both modes (avoid jitter sim in realtime unless requested)
- Align: Realtime minimal AP; Offline full correlation sweep

## CPU/GPU Scalability
- CPU: Accelerate/vDSP (FFT, convolution, vector ops) + hand‑tuned SIMD
- GPU: Metal compute for high‑order polyphase/oversampling and heavy visualizers
- Auto mode picks GPU when beneficial

## AudioGridder (extra processing power)
- Option A (recommended, immediate): DAW hosts our plugin on AudioGridder Server (no app changes)
- Option B (later): embed AG Client in app’s processing graph for offload

## Next Steps
- Add ui_pro/ scaffolding (template subwindow) and implement the 6 Milestone‑1 processes
- Expose Engine/Quality selectors, presets, explanation, global footer UI
- Wire Bake It In rendering path
