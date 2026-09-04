# Bundled Fonts

This directory contains typography assets used by the UI framework and overlay system.

---

## Included Fonts

### 1. Poppins Font Family (`.ttf`)
- **`Poppins-Regular.ttf`**: Default body text, descriptions, and UI controls.
- **`Poppins-Medium.ttf`**: Labels, secondary headers, and list items.
- **`Poppins-SemiBold.ttf`**: Section headers, window titles, and emphasized highlights.

### 2. Font Awesome Icons (`.woff2`)
- **`fa-light-300.woff2`**: Light icon variant.
- **`fa-regular-400.woff2`**: Standard regular icon glyphs.
- **`fa-solid-900.woff2`**: Solid high-contrast icon set.

---

## Usage in Custom Framework
Fonts are automatically loaded during application initialization via `ur::app::Config` or glyph ranges defined in `include/ur/glyphs.hpp`.
