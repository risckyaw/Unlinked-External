# Getting Started with Unlinked Framework (UR)

**Unlinked Framework** is a lightweight, immediate-mode C++ GUI and rendering library tailored for Windows. It provides multi-backend graphics rendering (Direct3D 11, Direct3D 12, OpenGL, and Vulkan) with a single, unified API.

---

## 1. Quickstart

Include the umbrella header `<ur/ur.hpp>` and initialize the run loop with `ur::app::run`:

```cpp
#include "ur/ur.hpp"

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int ) {
    ur::app::Config Config;
    Config.title = "Unlinked App";
    
    return ur::app::run( Config, [ ] {
        if ( ur::ui::window Window( "Hello Window" ); Window ) {
            ur::ui::heading( "Unlinked Framework" );
            
            if ( ur::ui::button( "Click Me" ) ) {
                ur::ui::notice( "Button was clicked!" );
            }
            
            float& Volume = ur::view::number( "volume", 0.6f );
            ur::ui::slider( "Volume", Volume, 0.0f, 1.0f );
        }
    } );
}
```

In your CMake configuration, use `ur_add_app(myapp Main.cpp)` to link the framework and copy required runtime assets automatically.

---

## 2. Window & Frame Management

`ur::app::run` handles the Win32 message loop, high-DPI scaling, backend presentation, and swapchain synchronization.

### Declarative Window (RAII)
```cpp
if ( ur::ui::window Window( "Window Title" ); Window ) {
    ur::ui::label( "Inside window content" );
}
```

### Manual Window / Frame Scope
```cpp
if ( Frames->Begin( "Window Title" ) ) {
    Widgets->Label( "Inside window content" );
    Widgets->Slider( "Gain", Gain, 0.0f, 1.0f );
}
Frames->End( );
```

Frame flags support `FrameMove`, `FrameResize`, `FrameCollapse`, `FrameClose`, `FrameDock` (`FrameDefault` enables all).

---

## 3. Immediate-Mode ID System

Immediate-mode widgets identify elements by hashing their string label. To prevent ID collisions:
- **Disambiguation**: Append `##identifier` to keep labels visible but unique: `"Save##button1"` vs `"Save##button2"`.
- **Hidden Label**: Prefix with `##` to hide the textual label: `"##volumeSlider"`.
- **Loop Scopes**: Wrap collections in `ur::id_scope Scope(i)`.

---

## 4. Layout Primitives

Elements flow vertically by default. Use layout helpers for complex arrangements:
- `Layout->SameLine()` — Places the next control on the same horizontal row.
- `Layout->PushWidth(w)` / `Layout->PopWidth()` — Constrains control widths.
- `Layout->BeginChild()` / `Layout->EndChild()` — Creates scrollable sub-regions.
- `Layout->BeginTable()` / `Layout->TableRow()` / `Layout->TableColumn()` — Grid layouts.
- `Widgets->BeginVirtual()` / `Widgets->EndVirtual()` — Virtual scrolling for thousands of items.

---

## 5. Widget Catalog

- **Typography**: `Label`, `Faint`, `Heading`, `Section`, `Wrapped`, `Bullet`, `Colored`.
- **Buttons**: `Button`, `Small`, `IconButton`.
- **Toggles & Booleans**: `Check`, `Toggle`, `Radio`.
- **Inputs & Sliders**: `Slider`, `SliderWhole`, `Drag`, `Knob`, `Number`, `Decimal`, `Vector`.
- **Text Entry**: `Field` (single-line), `Area` (multi-line) for `char*` or `std::string`.
- **Selection**: `Choice`, `Segments`, `List`, `FilterList`, `Selectable`.
- **Data & Visualizers**: `Plot`, `Histogram`, `Area`, `Pie`, `Meter`, `Waveform`, `Spectrum`.
- **Utilities**: `ColorPicker`, `Progress`, `Keybind`, `Splitter`, `Tooltip`, `BeginModal`.

---

## 6. Input & Keybinds

```cpp
if ( ur::pressed( ur::Key::F8 ) ) {
    // Direct key press check
}

ur::bind::set( "overlay.toggle", ( int )ur::Key::Insert );
if ( ur::bind::pressed( "overlay.toggle" ) ) {
    // Action bound to named shortcut
}
```

- `Ctrl+K`: Opens the quick Command Palette (populated via `ur::palette::add(...)`).
- `Ctrl+S`: Saves window positions and framework settings when `Config.persist` is enabled.

---

## 7. Styling & Theming

Themes define colors, control roundings, drop shadows, and font scalings:

```cpp
ur::theme::apply( 2 ); // Apply built-in preset
Style->Accent = CColor( 74, 124, 255 );
Style->Rounding = 12.0f;
ur::theme::load_file( "assets/themes/unlinked.theme" );
```

---

## 8. Desk & Studio Modules

Optional integrations configured via `ur::app::Config`:
- **Media Player**: `ur::player::draw_chip` / `draw_compact` / `draw_expanded` with Windows media session integration.
- **Audio Visualizer**: `ur::hear::draw_wave` / `draw_spectrum` for mic or system loopback audio.
- **Discord RPC**: Real-time rich presence updates.
- **3D Orbit**: `ur::orbit::draw` for 3D viewport rendering.
- **Transparent Overlay**: `ur::overlay::Options` for game overlays with click-through and layer glass.
