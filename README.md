# Pocket Places — C++ cross-platform game

An unofficial, original social-life sandbox prototype inspired by the broad ideas visible in the supplied Toca Life World location showcase. This project is not affiliated with or endorsed by Toca Boca.

## Targets
- Windows x64 desktop build (C++17 + raylib)
- Android ARM64 APK (same C++ game source)
- GitHub Actions workflows build both targets and upload artifacts

## Current playable foundation
- Illustrated, responsive world map with six prototype regions
- Connected roads, terrain, coast, trees, clouds, and distinct region markers
- Select a region, review its description, then enter it
- Six distinct handcrafted region scenes: city, valley, autumn forest, snowy town, seaside, and park
- Region scenes with a movable character, animated walking, and facing direction
- Three themed, walk-up props in every region (18 interactive props total)
- Enterable building interiors with distinct shop, cafe, bakery, and home details
- Discovery journal that records all 18 interactive objects and shows progress in each region
- Four character outfit color styles, cycled with `C`
- Interactions show location-specific messages and mark objects in the discovery journal; use `E` on desktop or the on-screen button on touch devices
- Mouse, keyboard, and touch-oriented directional-pad controls
- Camera scaling for different desktop and mobile aspect ratios

The six region names and environments are original prototype content, not a verified inventory of every Toca Life World location. The supplied YouTube page could not be fetched in this session, so its exact location list still needs visual reference screenshots.

## Build locally (Windows)
1. Install CMake 3.22.1+, Visual Studio 2022 C++ workload, and Git.
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
- Desktop: WASD / arrow keys to move, `E` or the on-screen Interact button to use nearby props / enter buildings, `C` to cycle outfit colors, click map markers to travel, and use `Esc` to leave interiors or return to the map.
- Android: touch a map marker; drag from the on-screen directional pad to move and tap Interact when near a prop or building.
