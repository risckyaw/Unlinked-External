# Building Unlinked Framework (UR)

This document provides instructions for compiling the framework library, demos, and integrating with external applications.

---

## Prerequisites

- **Operating System**: Windows 10 or Windows 11 (x64)
- **Compiler**: MSVC (Visual Studio 2022 recommended) with the **Desktop development with C++** workload
- **Build Tools**: CMake 3.20 or newer, Ninja build system
- **Windows SDK**: Windows 10 SDK (version 10.0.19041.0 or newer)
- **Vulkan SDK** *(optional)*: Required only if compiling with `-DUR_VULKAN=ON`

---

## Build Targets & Presets

The repository includes pre-configured CMake presets for fast builds:

### Release Build (Recommended)
```bash
cmake --preset windows-release
cmake --build --preset windows-release
```

### Debug Build
```bash
cmake --preset windows-debug
cmake --build --preset windows-debug
```

---

## Build Outputs

Upon successful compilation, the output binaries are located in the `build/` directory:

| Binary | Description |
| :--- | :--- |
| `build/windows-release/Showcase.exe` | Complete feature desk demonstrating all widgets, themes, and audio visualizers. |
| `build/windows-release/Hello.exe` | Minimal quickstart application example. |

> **Note**: The application runtime expects the `assets/` directory (fonts, icons, themes) to be present in the working directory or alongside the executable.

---

## Enabling Optional Features

- **Vulkan Backend**:
  ```bash
  cmake -B build -DUR_VULKAN=ON
  ```
- **Custom Application Integration**:
  To add your own application target in CMake:
  ```cmake
  ur_add_app( my_app src/Main.cpp )
  ```
