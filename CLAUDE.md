# CLAUDE.md — Ombro

## Project Overview

Ombro is a visualization plugin DLL for the Cabrio Desktop media player (Winamp-compatible). It renders real-time wireframe math-soundscapes driven by audio — a modern recreation of WhiteCap-style visualizations using Direct3D 11.

Licensed under **AGPL-3.0**.

## Repository Structure

```
ombro/
├── CLAUDE.md              # AI assistant guidelines (this file)
├── LICENSE                # AGPL-3.0 license
├── CMakeLists.txt         # Build configuration (generates VS 2022 solution)
├── ombro.def              # DLL export definitions (undecorated symbols)
├── .gitignore             # Build artifacts exclusions
└── src/
    ├── plugin.h           # Winamp/Cabrio vis plugin interface (structs, IPC, commands)
    ├── plugin.cpp         # DLL entry point, exports, plugin lifecycle, window management
    ├── shaders.h          # Embedded HLSL shaders (wireframe + fullscreen fade)
    ├── renderer.h         # D3D11 renderer class declaration
    ├── renderer.cpp       # D3D11 renderer: device, swap chain, ping-pong afterglow, draw
    ├── visualization.h    # Visualization engine: presets, audio analysis, surface generation
    └── visualization.cpp  # Math surfaces, camera orbit, preset transitions, audio processing
```

## Architecture

### Plugin Interface (`plugin.h`, `plugin.cpp`)
- Exports `winampVisGetHeader` and `cabrioVisGetHeader` via C linkage
- One module: "Ombro" — Init/Render/Quit on the vis thread, Config on the UI thread
- Window procedure handles keyboard (Esc, F11, arrows, space), host commands (WM_COMMAND), and fullscreen toggle

### Renderer (`renderer.h`, `renderer.cpp`)
- Direct3D 11 with feature level 11.0 (fallback to 10.1)
- **Ping-pong afterglow**: two offscreen render targets; each frame fades the previous frame and composites new wireframe on top, then copies to the swap chain back buffer
- Shaders compiled at runtime from embedded HLSL strings via `D3DCompile`
- Dynamic vertex/index buffers that auto-grow
- Wireframe rasterizer state with antialiased lines

### Visualization (`visualization.h`, `visualization.cpp`)
- 80×80 grid mesh (6,400 vertices, 25,280 line indices)
- Surface `z` computed per-frame from layered math functions:
  - **Crossing sine waves** — `sin(x·freq)·cos(y·freq+t)`
  - **Concentric ripples** — `sin(r·freq - t)`
  - **Spiral arms** — `sin(θ·arms + r·tightness - t)`
- Audio modulation: bass scales amplitude, treble adds fine detail, beats pulse the surface
- **8 presets** with smooth morphing transitions (~2.5s via smoothstep interpolation)
- Auto-cycles presets every 15 seconds
- HSV-based coloring: hue from surface height + time, brightness from audio energy
- Camera orbits at preset-defined speed, height modulated by audio

### Shaders (`shaders.h`)
- `g_wireframeVS` / `g_wireframePS` — transform vertices by view-proj matrix, pass-through per-vertex color
- `g_fullscreenVS` / `g_fadePS` — fullscreen triangle from `SV_VertexID`, samples previous frame with fade alpha

## Build

**Requirements:** Visual Studio 2022 (toolset v143), Windows SDK, CMake 3.20+

```powershell
# Generate VS solution
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Output: build/bin/vis_ombro.dll
# Deploy to Cabrio's Plugins/ directory
```

For x86: use `-A Win32` instead of `-A x64`.

**Linked libraries:** `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, `user32.lib`, `gdi32.lib`, `shlwapi.lib`

## Plugin Contract Summary

| Callback | Thread | Purpose |
|----------|--------|---------|
| `Init`   | Vis thread | Create window, init D3D11, set spectrumNch/waveformNch/delayMs |
| `Render` | Vis thread | Read audio data, update surface, draw frame. Return 0 to continue, 1 to stop |
| `Quit`   | Vis thread | Destroy window, release D3D11 resources |
| `Config` | UI thread  | Show configuration dialog |

Audio data: `spectrumData[2][576]` (frequency bins 0–255) and `waveformData[2][576]` (PCM samples), filled by host before each `Render` call.

## Controls

| Key | Action |
|-----|--------|
| Left/Right Arrow | Previous/next preset |
| Space | Random preset |
| F11 / Double-click | Toggle fullscreen |
| Escape | Exit fullscreen, or close visualization |

Unhandled keystrokes are forwarded to the host window for media playback control.

## License

AGPL-3.0 — all source files must be compatible with this license. Any new dependencies must have AGPL-3.0-compatible licenses (MIT, BSD, Apache-2.0, etc. are fine).

## Development Guidelines

### Git Conventions
- Default branch: `main`
- Clear, concise commit messages describing the "why"
- One logical change per commit

### Code Style
- C++17 with Win32/COM idioms for D3D11 code
- RAII where practical; `SafeRelease` helper for COM pointers
- Namespace `ombro` for all non-plugin-interface code
- Keep surface math readable — each term on its own line with a comment
- Prefer embedded shader strings over separate `.hlsl` files (simpler deployment)

### AI Assistant Notes
- Always read files before editing
- This is a Windows-only project (Win32 DLL, D3D11) — code will not compile on Linux
- The plugin interface structs in `plugin.h` must match the Winamp/Cabrio ABI exactly — do not modify field order, sizes, or alignment
- Do not add unnecessary abstractions; the renderer and visualization are intentionally simple classes
- Respect the AGPL-3.0 license when suggesting dependencies
