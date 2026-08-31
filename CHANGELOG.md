# Changelog - Diablo Nightmare Edition

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

### WebAssembly Virtualization Platform & Mini Windows XP Integration

#### Hardware & Virtualization Architecture
- **Direct NTLDR Real-Mode El Torito Bootloader (`tools/generate_boot_sector.js`)**: Replaced ISOLINUX/Syslinux with a custom 16-bit real-mode El Torito bootloader (`minixp_boot.bin`). When SeaBIOS boots, the sector displays `Booting Mini Windows XP...`, streams `XP.BIN` (370 KB NTLDR) directly into RAM (`0x2000:0000`) using BIOS INT 13h Extensions (AH=42h) across 64 KB segment boundaries, and transfers execution to NTLDR with `DL` set to the active boot drive. This bypasses all Syslinux filesystem parsing hangs (`ETCD`), booting Mini Windows XP directly.
- **Snapshot Persistence Subsystem**: Added instant state serialization (`save_state` / `restore_state`) allowing the entire 256 MB RAM state to be downloaded as a `.bin` snapshot file or reloaded in ~1 second, eliminating boot times on repeat sessions.

#### Packaging, Server & CI/CD Pipeline
- **Node.js & Python ISO Packaging Engines (`tools/build_minixp_iso.js` / `tools/build_minixp_iso.py`)**: Developed a standalone zero-dependency Node.js ISO-9660 + Joliet Level 3 + El Torito builder alongside the Python script, generating compact 62.1 MB bootable images (`minixp.iso`) with full Type L/M Path Tables, automatic Boot-Info-Table patching, and multi-path configuration mirroring (`/ISOLINUX.CFG`, `/ISOLINUX/ISOLINUX.CFG`, `/BOOT/ISOLINUX/ISOLINUX.CFG`, `/HBCD/ISOLINUX.CFG`) with standard `COM32` / `APPEND` parameter passing.
- **Node.js Range HTTP Server (`Packaging/minixp-wasm/server.js`)**: Added a dedicated Node.js HTTP server supporting HTTP Range requests (206 Partial Content) and COOP/COEP headers for high-performance WebAssembly execution without external Python dependencies.
- **Same-Origin GitHub Pages CI/CD Pipeline (`.github/workflows/deploy-pages.yml`)**: Automated the build and deployment pipeline in GitHub Actions to build `minixp.iso` from the repository and publish it into `dist/minixp/minixp.iso`. This serves the OS image from the exact same domain origin, enabling native HTTP Range (206 Partial Content) streaming with zero CORS restrictions.

#### Modern Telemetry UI & Diablo Integration
- **Clean Hardware Status Bar**: Redesigned the emulator dashboard to replace text-heavy specification cards with a minimalist, modern status strip (`Pentium II x86 • 256 MB RAM • VGA VBE • CD-ROM IDE`).
- **Real-Time Disk I/O Activity LED & Transfer Meter**: Integrated a live green activity LED that pulses on ATAPI IDE read interrupts (`ide-read-start` / `ide-read-end`), accompanied by a real-time transfer counter displaying cumulative megabytes streamed into RAM.
- **Live Diagnostic & Serial Event Console**: Embedded a collapsible dark terminal drawer logging timestamped emulator lifecycle events, BIOS execution, video mode transitions, network asset download progress, and live serial output (`serial0-output-char`) from BIOS/Kernel.
- **Aggressive Cache-Busting Subsystem**: Implemented cache-control HTTP meta tags (`no-cache, no-store, must-revalidate`) and asset versioning query parameters (`app.js?v=3`, `style.css?v=3`) ensuring clients immediately run updated scripts without stale browser caching.
- **Web Diablo Quick Launchers**: Integrated a top toolbar launcher button (`🪟 Mini XP`) and an interactive card in the Web File Manager modal (`Packaging/emscripten/index.html`) to launch the virtual machine in a dedicated tab.

### Diablo Nightmare Edition (Gothic Visual Remaster & QoL Features)

#### Unified UI Branding & Mode Synchronization
- **Unforced Product Title**: Standardized `PROJECT_NAME` and `gszProductName` as `"Diablo Nightmare Edition"` across both Diablo and Hellfire game modes, ensuring consistent main menu, dialog, and window captions across all platforms.

#### Combat QoL & Visual Feedback
- **Monster Health Bar & Overhead Floating Bars**: Configured `enemyHealthBar` option to `true` by default and added dynamic overhead health bars (`DrawMonsterOverheadHealthBar`) rendered directly above living monsters in combat, featuring real-time health scaling, critical health indicators, and gold borders for unique champions/bosses.
- **Floating Damage Numbers**: Integrated dynamic floating damage numbers (`AddFloatingNumber`) in `ApplyMonsterDamage`, displaying red damage popups floating above monsters during combat.

#### NIGHTMARE — Arcane Runes (+2 Spell Level Consumables)
- **Empowered Spell Runic Inscriptions**: Completely overhauled the dormant 1997 Hellfire single-use ground trap rune mechanic into valuable ancient runic inscriptions (`Source/items.cpp`, `Source/inv.cpp`, `Source/qol/stash.cpp`). Right-clicking a rune now directly and permanently consumes it from the inventory (usable in both town and dungeons with book-reading audio and visual feedback), increasing the associated spell level by **+2** up to the maximum spell cap (`MaxSpellLevel = 15`).
  - **Rune of Fire** (`IMISC_RUNEF`): Permanently increases *Fireball* by +2 spell levels.
  - **Rune of Lightning** (`IMISC_RUNEL`): Permanently increases *Lightning* by +2 spell levels.
  - **Greater Rune of Lightning** (`IMISC_GR_RUNEL`): Permanently increases *Chain Lightning* by +2 spell levels.
  - **Greater Rune of Fire** (`IMISC_GR_RUNEF`): Permanently increases *Flame Wave* by +2 spell levels.
  - **Rune of Stone** (`IMISC_RUNES`): Permanently increases *Stone Curse* by +2 spell levels.
- **Rune Tooltips & Info Box**: Updated item descriptions (`PrintItemOil` in `Source/items.cpp`) to clearly state the exact +2 spell level benefits, eliminating legacy trap text while keeping original code commented for reference.

#### NIGHTMARE — Atmospheric Floor Reconfiguration (24 Dungeon Levels)
- **Dynamic Floor Light Radii**: Implemented `GetNightmareBaseLightRadius(currlevel)` in `Source/nightmare/world/level_atmosphere.cpp` and `Source/items.cpp` replacing static base radius (12) with per-floor atmospheric depths (ranging from claustrophobic darkness of 3-4 tiles in deep Hell and bone crypts to 10 in open cavern and chamber floors).
- **Procedural Floor Music Orchestration**: Integrated `GetNightmareLevelMusic(dungeonType, currlevel, setlevel)` in `Source/engine/sound.cpp` breaking biome auditory predictability (e.g. Cathedral Threshold playing haunting Crypt tracks, Catacombs Lords playing Hell music, and Crypt Treasure Chamber playing Cathedral solemn themes).
- **Floor Economy & Chest Density Overrides**: Added `GetNightmareChestCounts(currlevel)` in `Source/objects.cpp` tuning chest distributions per floor archetype (extreme scarcity in Hunger/Silence levels, high rewards in Lords/Treasure chambers).
- **Monster Depth Distribution Overrides**: Integrated `ApplyNightmareMonsterLevelOverrides()` in `Source/tables/monstdat.cpp` allowing early shock spawns (Hidden/Sneaks and Flesh Clan in lower Cathedral floors, earlier Acid beasts, and deep dungeon Succubi/Casters) while keeping all original engine tables documented and commented.

#### NIGHTMARE — Restoration Layer (Dormant Content Activation)
- **Restoration Layer 2 (Prophecy of the Stars, Cut Gossip Library & Monster Restorations)**:
  - **Map of the Stars Prophecy**: Integrated Deckard Cain's dormant prophecy dialogue (`Cain22.wav` / `TEXT_DOOM1` - `TEXT_DOOM10`) with defensive sound loading (`PlayLayer2WavDefensive`) and text banner rendering when inspecting the Map of the Stars.
  - **Extended Tristram Gossip & Lore**: Reactivated sleeping town gossip pools (`TEXT_FARNHAM16-22`, `TEXT_GILLIAN11-26`, `TEXT_PEPIN12-30`, `TEXT_GRISWOLD14-37`) with 30% random selection probability (`GetLayer2GossipDefensive`) when speaking to Tristram NPCs.
  - **Incinerator (`MT_INCIN`) AI & Unique Bosses**: Implemented `FireManAi` handling ranged fireball attacks and rolling fireball transformation (`Special` animation), spawning in Caves (levels 11-12) and Hell (levels 13-15) alongside unique bosses *Wrathfire the Doomed* and *Warpfire Hellspawn*.
  - **Arch-Lich Malignus (`MT_DARKMAGE`) AI & Level 15/16 Spawns**: Configured Arch-Lich Malignus with `CounselorAi` (evasive teleportation and arcane conjuration) in Hell levels 15 and 16.
  - **Anti-Crash VFS/MPQ Asset Guards**: Added runtime asset checks (`IsIncineratorAssetAvailable`, `IsDarkMageAssetAvailable`, `IsLayer2AssetAvailable`) in `IsMonsterAvailable` and audio playback to prevent `app_fatal` crashes when custom or vanilla MPQs lack Layer 2 assets.
- **Tremain the Priest & Shadowfang Quest**: Integrated dormant towner Tremain (`TOWN_PRIEST`, `priest8.cel`) into Tristram church courtyard (`{65, 24}`) via `towners.tsv` (Diablo & Hellfire) and `Source/towners.cpp`, with full original voice lines (`priest00.wav` - `priest07.wav`), procedural unique boss *Fleshdoom* in Catacombs (levels 5-7), dropping *Shadowfang*, and rewarding `Lightforge` mace upon resolution.
- **Tremain Audio & Quest Log Bug Fixes**:
  - **Single Dialogue Playback**: Fixed dialogue audio looping continuously across town and dungeons by updating sound player invocation (`PlayNightmareRestorationWav`) to stop prior samples and issue single-instance playback (`numIterations = 1`).
  - **Quest Log Integration**: Expanded `MAXQUESTS` to 25 and added `Q_TREMAIN` ("Tremain the Priest") to `questdat.tsv` and `quest_id` table; speaking with Tremain or defeating Fleshdoom dynamically registers and tracks the quest in the Quest Log (`_qlog = true`).
  - **Non-Colliding Coordinates & Hellfire Availability**: Relocated Tremain's town position to `{65, 24}` in the church courtyard to prevent overlaps with Complete Nut (`{61, 22}`) and Lester (`{62, 16}`), and registered `TOWN_PRIEST` in `mods/Hellfire/txtdata/towners/towners.tsv` for full Hellfire compatibility.
- **Safe Asset Loading & Crash Prevention for Tremain**: Updated Tremain NPC initialization (`tremain.cpp` & `towners.cpp`) to use `LoadOptionalCel` with on-the-fly `.cel` to `.clx` conversion (`towners/priest/priest8`) and safe `std::nullopt` rendering checks (`scrollrt.cpp`, `towners.h`). Prevents `app_fatal` crashes and error popups when priest graphic assets are missing from MPQ or custom installs.
- **Butcher Chamber Video Cinematic**: Hooked `gendata/fbutch3.smk` video trigger directly to the first entrance of the Butcher's Chamber (Level 2).
- **Wirt Reversed Speech Riddle (50% Chance)**: Integrated conditional 50% probability trigger for Wirt's reversed audio lines (`pegboy21.wav` / `pegboy22.wav`) upon interaction after visiting the Butcher's level.
- **Guaranteed Hellfire Quests & NPC Coexistence**: Enabled full concurrent availability for all Hellfire quests (`bTheoQuest` & `bCowQuest` set to active by default in Hellfire mode). Lester the Farmer (`TOWN_FARMER`, `{62, 16}`), Complete Nut (`TOWN_COWFARM`, `{61, 22}`), Celia the Little Girl (`TOWN_GIRL`, `{77, 43}`), and Tremain the Priest (`TOWN_PRIEST`, `{62, 23}`) now coexist simultaneously in Tristram at non-overlapping coordinates, allowing all Hive, Cow Suit (Bovine Plate), Theodore bear, and Restoration quests to be active and completed in a single playthrough.
- **Tristram Ambient Animal Sound Queue**: Hooked `InitTristramAmbientAnimals()` to town map creation (`CreateTown`) and tuned playback intervals (first trigger at 4-8s, recurring every 15-30s) with active vector retention for dormant animal sound effects (`sfx/animals/*`).

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
- **File Manager UI & Download Button**: Added dedicated download buttons (`💾 Descargar`) alongside delete controls for all character save files (`.sv`, `.hsv`, `.dsv`, `.ini`) and MPQ archives in the File Manager, with toast notification feedback for backing up character progress locally.

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
