# Changelog - Diablo Nightmare Edition

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

### Diablo Nightmare Edition (Fork Release & Mod Features)

#### Render Loop, CPU Optimization & Focus Management
- **Inactive CPU Throttle**: Introduced active idle sleep (`SDL_Delay(20)`) when the game window is unfocused, minimized, or hidden (`!gbActive`). Prevents 100% CPU core spinning, high machine strain, fan noise, and application freezing when switching tabs or unfocusing windows.
- **Frame Rate Limiter**: Fixed `LimitFrameRate()` calculation to compute microsecond deadlines precisely (`frameDeadline - tc`) and apply exact frame delays instead of modulo arithmetic.

#### Hellfire Expansion & Game Mode Switching
- **UI Mode Switcher**: Exposed the `Game Mode` setting directly in the UI Settings menu (`Settings -> Game Mode`), allowing seamless switching between **Diablo** and **Hellfire** without editing `diablo.ini` or passing CLI parameters.
- **Dynamic Mod Reloading**: Registered `OptionGameModeChanged()` handler that automatically toggles Hellfire mod status (`SetHellfireEnabled`) and reloads Lua mod archives and game assets dynamically (`LuaReloadActiveMods`).
- **Legacy Override Fix**: Removed legacy startup migration in `DiabloInit()` that previously forced `StartUpGameMode::Hellfire` back to `Diablo` on startup.

#### Save File Management (.sv & .hsv)
- **Dual-Extension Fallback**: Implemented primary and alternate save file fallback in `OpenSaveArchive()`. Hellfire mode can now open Diablo (`.sv`) saves and Diablo mode can open Hellfire (`.hsv`) saves seamlessly.
- **Transparent Character Conversion**: Automatic save game conversion (`pfile_convert_levels()`) when loading a character created in the opposite mode.
- **Synchronized Character Deletion**: Updated `pfile_delete_save()` to delete both `.sv` and `.hsv` files associated with a character slot.
- **File Manager UI**: Web and desktop UI options to import, manage, export, and download `.sv`, `.dsv`, and `.hsv` save files.

#### WebAssembly & Browser Port (Emscripten / GitHub Pages)
- **GitHub Pages CI/CD**: Automated WebAssembly build and deployment workflow via GitHub Actions (`.github/workflows/deploy.yml`).
- **Pure Web Port Architecture**: Single-threaded ASYNCIFY execution model without COOP/COEP / `SharedArrayBuffer` requirements, enabling instant playback on any modern desktop or mobile browser.
- **In-Browser File Manager**: Drag-and-drop MPQ loader supporting both `DIABDAT.MPQ` (Full Retail) and `spawn.mpq` (Shareware), with auto-persistence via IndexedDB (`IDBFS`).
- **Immersive Dark & Fullscreen UI**: Black background layout, dynamic aspect-ratio viewport scaling, and 1-click Fullscreen toggle.
- **Memory & Performance Optimizations**: Configured 256MB initial heap with dynamic growth (up to 2GB) and disabled AddressSanitizer/UndefinedBehaviorSanitizer for Emscripten to prevent WebAssembly OOM crashes when loading large retail archives.

#### Graphics Remaster & Dynamic Visuals
- **Subtle Graphics Remaster GPU Pipeline**: Real-time WebGL/GPU post-processing pipeline enhancing sprite rendering and color depth.
- **Real-Time Remaster Comparison Toggle (F2)**: Pressing **F2** in-game instantly toggles between classic original graphics and remastered graphics mode.
- **Dynamic 2D Lighting & Dungeon Embers**: Dynamic light falloff, realistic torch flickering, and floating ember overlays in dungeons.
- **Hell Lighting & Visual Polish**: Smooth falloff lighting curve in Hell levels, base player light radius increased to 12 tiles, and removed red screen tint during pause in dungeons.

#### Features & Gameplay Enhancements
- **Portal Rojo**: Red portal transition support in Tristram.
- **Cinemática Espada**: Restored initial sequence cinematic.
- **Chat Single Player**: Enabled in-game chat in single-player mode.
- **Ollama AI Integration**: Dynamic text variation support via AI integration hooks.
- **Nightmare Balance & Items**: Custom item affixes and Hellfire balancing.
- **Environmental & Ambient Effects**: Rain and contemplative UI in town.
- **Lua Engine & Mod System**: Expanded Lua bindings for items, system, render, floating damage numbers (`Floating Numbers - Damage`), and experience bar (`Floating Numbers - XP`).

### Bug Fixes

#### Graphics / Audio

- Music doesn't unmute when focus is lost on level transition with Auto Pause On Focus Lost disabled
- Image ghosting visible on border of map in higher resolutions

#### Stability / Performance / System

- Crashes related to player graphics rendering in death state
- Fix CPU spin and application freeze when window loses focus or tabs are switched
- WebAssembly: Fix out-of-memory (OOM) crash when loading large retail `DIABDAT.MPQ` (~500MB) by configuring 256MB initial heap and 2GB maximum memory cap, and disabling sanitizers for Emscripten builds
