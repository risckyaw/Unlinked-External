# Contributing to Unlinked

Thank you for your interest in contributing to **Unlinked**! Contributions from the community help make this project more stable, efficient, and feature-rich.

---

## 📋 Prerequisites

Before you start developing, ensure your environment meets the following requirements:

- **Operating System**: Windows 10 or Windows 11 (64-bit).
- **Toolchain**:
  - [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (Community, Professional, or Enterprise) or Visual Studio Build Tools.
  - Required workload: **Desktop development with C++** (which includes MSVC, Windows SDK, and C++ CMake tools).
  - C++ standard: **C++20**.
- **Roblox**: `RobloxPlayerBeta.exe` (LIVE channel; see README for offset synchronization notes).

---

## 🛠️ Building the Project

The repository includes everything needed to compile without downloading extra repositories.

### One-Click Build

Use the bundled batch script:

```batch
# Build Release (output: build\windows-release\Unlinked.exe)
build.bat

# Build Debug (output: build\windows-debug\Unlinked.exe)
build.bat --debug
```

### Manual CMake Build

You can also use CMake Presets directly from any terminal configured with MSVC (`VsDevCmd.bat`):

```batch
# Release
cmake --preset windows-release
cmake --build --preset windows-release

# Debug
cmake --preset windows-debug
cmake --build --preset windows-debug
```

Assets (`assets/`) are automatically copied beside the resulting executable after build.

---

## 📂 Project Structure

```
build.bat                   One-click build script
CMakeLists.txt              Primary CMake project definition
CMakePresets.json           Preset configurations (windows-release, windows-debug)
assets/                     Fonts, icons, and themes copied to target directory
src/                        Core source code
  ├── Main.cpp              Application entry point, overlay loop, and UI composition
  ├── world.hpp             Memory reader, DataModel traversal, math & matrix transforms
  ├── sense.hpp             ESP features and rendering logic
  ├── move.hpp              Movement & physics modifiers (jump power, noclip)
  ├── silent.hpp            Silent aim hooks and shellcode generation
  ├── store.hpp             Config serialization and preset management
  ├── browse.hpp            DataModel explorer hierarchy and property cache
  ├── explorer.hpp          Explorer tree UI components & icon helpers
  ├── catalog.hpp           Themes, colors, and shaders
  ├── weather.hpp           Particle system (snow, rain) and visual effects
  └── offsets.hpp           Dynamic offset sync with offsets API & local cache
third_party/
  ├── custom-framework/     Bundled immediate-mode UI & rendering library
  └── fonts/                Font Awesome 6 and typography assets
```

---

## 🌿 Branching & Git Workflow

1. **Fork the Repository**:
   Create a fork of `viltzn/Unlinked-External` under your GitHub account.

2. **Clone your Fork**:
   ```bash
   git clone https://github.com/<your-username>/Unlinked-External.git
   cd Unlinked-External
   git remote add upstream https://github.com/viltzn/Unlinked-External.git
   ```

3. **Create a Feature Branch**:
   Create a branch with a descriptive name prefixed with its category:
   ```bash
   # For bug fixes
   git checkout -b fix/issue-description

   # For new features
   git checkout -b feature/feature-name

   # For documentation / tooling
   git checkout -b chore/tooling-update
   ```

4. **Make and Verify Changes**:
   - Ensure the project builds cleanly without warnings (`build.bat`).
   - Keep diffs focused on the specific change — avoid broad, unrelated reformatting.

5. **Commit with Conventional Messages**:
   Use structured commit messages:
   - `fix: resolve issue in ...`
   - `feat: add new feature for ...`
   - `refactor: clean up ...`
   - `docs: update documentation ...`

6. **Submit a Pull Request**:
   Push your branch to your fork and open a Pull Request against `main`. Fill in the PR template with summary, rationale, and verification steps.

---

## 📝 Coding Standards

- **Standard**: C++20.
- **Compiler Warnings**: Code should compile with MSVC `/W4` without introducing new warnings. Use `if constexpr` for compile-time template branching.
- **Resource Management**: Always ensure Windows resources (handles, GDI font resources, files) are freed or unregistered when no longer needed.
- **Memory Safety**: Validate pointers and heap boundaries before reading (`world::Heap(Addr)`).
