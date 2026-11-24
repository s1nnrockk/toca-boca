# Pocket Places — C++ cross-platform game

An unofficial, original social-life sandbox prototype inspired by the broad ideas visible in the supplied Toca Life World location showcase. This project is not affiliated with or endorsed by Toca Boca.

## Targets
- Windows x64 desktop build (C++17 + raylib)
- Android ARM64 APK (same C++ game source)
- GitHub Actions workflows build both targets and upload artifacts

## Current playable foundation
- Illustrated world map with six selectable regions
- Region scenes with a movable character
- Mouse and touch input
- Location switching and a simple character/item interaction loop
- Responsive UI for desktop and mobile aspect ratios

## Build locally (Windows)
1. Install CMake 3.24+, Visual Studio 2022 C++ workload, and Git.
2. Run:
   ```powershell
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
3. The executable is produced in `build/Release/PocketPlaces.exe` (or `build/PocketPlaces.exe`, depending on the generator).

## Build Android
Open the project in Android Studio or run the Android workflow. The Android Gradle project uses the same `src/main.cpp` and CMake target.

## GitHub Actions
- **Windows EXE:** Actions → Build Windows EXE
- **Android APK:** Actions → Build Android APK

Artifacts are attached to the completed workflow run.

## Assets and licensing
The current art is drawn procedurally in C++ and is original placeholder artwork. No proprietary Toca Boca images, sounds, animations, maps, or game files are included. Add only assets that you own or are licensed to redistribute.

## Controls
- Desktop: WASD / arrow keys to move; click map markers to travel; click the map button to return.
- Android: touch a map marker; use the on-screen directional pad to move.
