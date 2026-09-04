# Custom Framework (UR)

A high-performance, immediate-mode C++ UI framework designed for Windows desktop applications and overlays. It provides a modern declarative API, multi-backend hardware rendering (Direct3D 11, Direct3D 12, OpenGL, Vulkan), built-in animation systems, custom styling, and audio/media studio integrations.

![Showcase](docs/preview.png)

---

## Key Features

- **Immediate-Mode UI**: Simple, declarative widget workflow with minimal boilerplate.
- **Multi-Backend Rendering**: Seamless support for Direct3D 11, Direct3D 12, OpenGL, and optional Vulkan.
- **Glass & Overlay Ready**: Layered transparent windows, click-through support, and high-DPI scaling out of the box.
- **Comprehensive Widget Library**: Buttons, sliders, drag controls, color pickers, text inputs, segmented controls, tree views, tables, plots, histograms, and audio visualizers.
- **Modular Architecture**: Built-in desk modules including Windows media session tracking, real-time audio analysis (mic and loopback), 3D orbit visuals, Discord RPC, and command palette (`Ctrl+K`).
- **Smooth Motion & Theming**: Built-in spring and lerp animations, customizable rounding, drop shadows, and extensible theme palettes.

---

## Graphics Backends

Select your rendering backend during startup or switch dynamically at runtime:

| Backend | API | Description & Platform Notes |
| :--- | :--- | :--- |
| **Direct3D 11** | `D3D11 + DXGI` | **Default path.** Highly stable and optimized for Windows 10/11. |
| **Direct3D 12** | `D3D12 + DXGI` | Modern low-overhead graphics pipeline. |
| **OpenGL** | `WGL / Desktop GL` | Fallback cross-compatible desktop pipeline. |
| **Vulkan** | `Vulkan SDK` | High-performance optional backend enabled via `-DUR_VULKAN=ON`. |

The framework manages the Win32 window lifetime, DPI awareness, swapchain resizing, and input routing across all backends automatically.

---

## Getting Started

### Quick Example

```cpp
#include "ur/ur.hpp"

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int ) {
    ur::app::Config Config;
    Config.title = "Application";
    Config.backend = ur::Backend::DX11;
    Config.width = 1280;
    Config.height = 720;

    return ur::app::run( Config, [ ] {
        if ( ur::ui::window Window( "Main Window" ); Window ) {
            ur::ui::heading( "Custom Framework" );
            ur::ui::label( "Direct3D 11, Direct3D 12, OpenGL, or Vulkan." );

            static float Volume = 0.75f;
            ur::ui::slider( "Master Volume", Volume, 0.0f, 1.0f );

            if ( ur::ui::button( "Show Toast" ) ) {
                ur::ui::notice( "Action completed successfully!" );
            }

            if ( ur::ui::button( "Quit" ) ) {
                ur::app::quit( );
            }
        }
    } );
}
```

---

## Project Structure

```
custom-framework/
├── include/ur/         # Public C++ headers (app, ui, widgets, overlay, theme, etc.)
├── src/
│   ├── app/            # Window lifecycle, DPI handling, settings, theme persistence
│   ├── engine/         # Immediate-mode widget rendering, layout engine, canvas primitives
│   ├── host/           # D3D11, D3D12, OpenGL, and Vulkan rendering backends
│   ├── ui/             # Command palette, toast notifications, motion & spring physics
│   ├── widgets/        # Specialized widgets (media player, 3D orbit, desk tools)
│   └── audio/          # WASAPI audio capture, waveform, and spectrum analyzer
├── demos/
│   ├── hello/          # Minimal 20-line introductory demo
│   └── showcase/       # Complete feature showcase and widget gallery
└── docs/               # Detailed guides (start.md, build.md)
```

---

## Building

### Requirements
- Windows 10 / 11 (64-bit)
- MSVC (Visual Studio 2022 recommended with "Desktop development with C++")
- CMake 3.20+ and Ninja

### Build Commands
```bash
cmake --preset windows-release
cmake --build --preset windows-release
```

| Built Target | Description |
| :--- | :--- |
| `build/windows-release/Hello.exe` | Minimal quickstart application |
| `build/windows-release/Showcase.exe` | Full desk showcase application |

---

## Documentation

- [Getting Started Guide](docs/start.md) — Comprehensive overview of UI widgets, ID stack, themes, and desk modules.
- [Build Instructions](docs/build.md) — Detailed compiler configurations, CMake options, and backend prerequisites.
