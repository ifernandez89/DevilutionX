# Changelog - Diablo Nightmare Edition

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

### Diablo Nightmare Edition (Gothic Visual Remaster & QoL Features)

#### Unified UI Branding & Mode Synchronization
- **Unforced Product Title**: Standardized `PROJECT_NAME` and `gszProductName` as `"Diablo Nightmare Edition"` across both Diablo and Hellfire game modes, ensuring consistent main menu, dialog, and window captions across all platforms.

#### Combat QoL & Visual Feedback
- **Monster Health Bar & Overhead Floating Bars**: Configured `enemyHealthBar` option to `true` by default and added dynamic overhead health bars (`DrawMonsterOverheadHealthBar`) rendered directly above living monsters in combat, featuring real-time health scaling, critical health indicators, and gold borders for unique champions/bosses.
- **Floating Damage Numbers**: Integrated dynamic floating damage numbers (`AddFloatingNumber`) in `ApplyMonsterDamage`, displaying red damage popups floating above monsters during combat.

#### NIGHTMARE — Restoration Layer (Dormant Content Activation)
- **Tremain the Priest & Shadowfang Quest**: Reactivated dormant towner Tremain (`priest8.cel`) in Tristram church ruins with full original voice lines (`priest00.wav` - `priest07.wav`), spawning procedural unique boss *Fleshdoom* in Catacombs (levels 5-7), rewarding `Lightforge` mace upon resolution.
- **Butcher Chamber Video Cinematic**: Hooked `gendata/fbutch3.smk` video trigger directly to the first entrance of the Butcher's Chamber (Level 2).
- **Wirt Reversed Speech Riddle (50% Chance)**: Integrated conditional 50% probability trigger for Wirt's reversed audio lines (`pegboy21.wav` / `pegboy22.wav`) upon interaction after visiting the Butcher's level.
- **Tristram Ambient Animal Sound Queue**: Enqueued dormant animal audio effects (`sfx/animals/*`) in Tristram mixer with a non-intrusive random timer (45s to 120s).

#### Multiplatform Gothic Visual Enhancement & Weather System
- **Nightmare UI Pass (Gothic Slate Stone Architecture)**: Consolidated zero-risk visual skinning strategy over existing UI geometry (`implementation_plan.md`), unifying Inventory, Stash, Character Panel, Spellbook, and HUD buttons with `GetGoldToStoneTRN()` while preserving 100% of C++ game logic, drag-and-drop, and slot hit-testing for PC, Mobile, and WebAssembly.
- **Slate Stone Buttons & Window Trims (Angel/Demon Palette)**: Integrated `GetGoldToStoneTRN()` translation table converting golden button bevels, button text, and window borders (Inventory, Stash, Character Panel, Spellbook, and Quest Log) into the matching slate stone gray of the HUD angel and demon statues, delivering a consistent gothic aesthetic without red font artifacts.
- **Fixed Spell Icon Box & 6-Button Singleplayer HUD**: Corrected `TotalSpMainPanelButtons` to 6 and eliminated the singleplayer 'LOAD' button overlay over the quick spell icon box at bottom-right.
- **Embedded Web Favicon & Page Title**: Added 32x32 Diablo skull favicon (embedded base64) and updated web title to "Diablo Nightmare Edition" in `Packaging/emscripten/index.html`.
- **Screen-Space Weather System in Tristram**: Preserved 2D particle rain engine (`weather_overlay.cpp`), rendering 350 translucent falling drops with wind drift in town under UI panels.
- **Hellfire Save & Character Class Persistence**: Set default game mode to `StartUpGameMode::Hellfire` and updated character unpacker (`pack.cpp`) to preserve Hellfire hero classes (`HeroClass::LAST`), ensuring `.hsv` saves load seamlessly even after resetting `diablo.ini`.
- **Multiplatform Architecture Plan**: Established zero-risk visual skinning strategy over existing UI geometry (`implementation_plan.md`), preserving touch controls and slot hit-testing for PC, Mobile, and WebAssembly.

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
