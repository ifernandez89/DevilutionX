# 🌙 DIABLO NIGHTMARE EDITION - DOCUMENTACIÓN MAESTRA DE FEATURES ESTABLES Y FUNCIONALES

Esta es la documentación oficial consolidada para **Diablo Nightmare Edition**. Contiene **exclusivamente las características probadas, estables y 100% funcionales** validadas en el compilado final (`PROGRESO_FEATURES_FINAL_ACTUALIZADO.md`), excluyendo intentos fallidos pasados para asegurar máxima estabilidad.

---

## 🏆 CÁTALOGO OFICIAL DE FEATURES ACTIVAS Y ESTABLES (14/14)

### 1. 👁️ Focus Tracking Post-Diablo Fix
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: La cámara del juego continúa siguiendo al personaje tras derrotar a Diablo en Single Player. Los dungeons y portales se mantienen abiertos y accesibles para continuar la partida.
- **Archivos**: [`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp), [`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp)

### 2. 📜 Guaranteed Quests
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Todas las misiones del juego original están garantizadas en cada partida en modo Single Player.
- **Archivo**: [`Source/quests.cpp`](file:///c:/Projects/DevilutionX/Source/quests.cpp)

### 3. 💡 Visibilidad e Iluminación Mejorada (Base Light Radius)
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Radio de iluminación base del personaje incrementado de 10 a 12 casillas para mayor visibilidad y confort visual.
- **Archivo**: [`Source/player.cpp`](file:///c:/Projects/DevilutionX/Source/player.cpp)

### 4. 👹 Monster Depth Scaling (Monster Variants by Depth)
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Escalado progresivo de monstruos según la profundidad de la mazmorra (+5% HP, +3% daño cada 2 niveles).
- **Archivo**: [`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp)

### 5. 📦 Decorative Density
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Mayor densidad de elementos decorativos (barriles, urnas, vasijas) en los niveles de mazmorra.
- **Archivo**: [`Source/objects.cpp`](file:///c:/Projects/DevilutionX/Source/objects.cpp)

### 6. 🗿 Visual Elite Monsters
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: 0.5% de probabilidad de generar monstruos élite con efecto visual de piedra (`TRN`), prefijo `"Corrupted"` en su nombre y barra de salud roja.
- **Archivos**: [`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp), [`Source/qol/monhealthbar.cpp`](file:///c:/Projects/DevilutionX/Source/qol/monhealthbar.cpp), [`Source/control/control_infobox.cpp`](file:///c:/Projects/DevilutionX/Source/control/control_infobox.cpp)

### 7. 🏰 Thematic Monster Packs
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Identidad temática por profundidades (60% probabilidad de favorecer familias temáticas: No-Muertos L1-4, Demonios L5-8, Bestias L9-12, Infernales L13-15).
- **Archivo**: [`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp)

### 8. 🚶 Complete NPC Facing & Idle Micro-Wandering
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Los NPCs de Tristram giran suavemente hacia el jugador al interactuar usando secuencias estables de frame único (`UpdateTownerFacing`), y cuando están desocupados deambulan sutilmente 1-2 casillas alrededor de su posición origen.
- **Archivos**: [`Source/towners.cpp`](file:///c:/Projects/DevilutionX/Source/towners.cpp), [`Source/towners.h`](file:///c:/Projects/DevilutionX/Source/towners.h)

### 9. ✝️ Guaranteed Wounded Townsman (Muerto en la Iglesia)
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: `IsTownerPresent(TOWN_DEADGUY)` devuelve siempre `true`, garantizando que el aldeano herido aparezca a la entrada de la iglesia en todas las sesiones.
- **Archivo**: [`Source/towners.cpp`](file:///c:/Projects/DevilutionX/Source/towners.cpp)

### 10. 🔴 Red Portal Always Active in Town
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Todos los portales del pueblo utilizan la animación del **Red Portal** (`MissileID::RedPortal`), y el portal del jugador local en Tristram está activado por defecto apuntando a la Catedral.
- **Archivo**: [`Source/portal.cpp`](file:///c:/Projects/DevilutionX/Source/portal.cpp)

### 11. 🎛️ HUD Control Panel "LOAD" Button
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Botón dedicado de Cargar Partida en el panel inferior (coordenadas `560, 75`, lado derecho) con etiqueta de texto dorada `"LOAD"`, ejecutando `gamemenu_load_game(false)`.
- **Archivos**: [`Source/control/control_panel.cpp`](file:///c:/Projects/DevilutionX/Source/control/control_panel.cpp), [`Source/control/control_panel.hpp`](file:///c:/Projects/DevilutionX/Source/control/control_panel.hpp), [`Source/control/control.hpp`](file:///c:/Projects/DevilutionX/Source/control/control.hpp)

### 12. 🌧️ Dynamic Gothic Weather & Town Water Waves
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Sistema de lluvia gótica dinámica (`MAX_RAIN = 220`) renderizado en dos capas de profundidad (`DrawRainBack` y `DrawRainFront`) en Tristram.
- **Archivo**: [`Source/engine/render/scrollrt.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/scrollrt.cpp)

### 13. 🛡️ Fail-Soft Audio & Missile Capacity Protection
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Protección contra desbordamiento en `AddMissile()` (`ActiveMissileCount >= MAXMISSILES - 5`) y firmas de audio seguras en `effects_stubs.cpp` previniendo cuelgues ante hechizos intensos (Apocalypse, Inferno).
- **Archivos**: [`Source/missiles.cpp`](file:///c:/Projects/DevilutionX/Source/missiles.cpp), [`Source/effects_stubs.cpp`](file:///c:/Projects/DevilutionX/Source/effects_stubs.cpp)

### 14. 🎨 Nightmare Main Menu Branding
- **Estado**: ✅ IMPLEMENTADO Y FUNCIONAL
- **Descripción**: Cadena de título del menú principal actualizada a `"Diablo Nightmare Edition"`.
- **Archivo**: [`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp)

---

## ❌ CARACTERÍSTICAS ABORTADAS Y EXCLUIDAS (POR ESTABILIDAD)

- ❌ **Farnham Upright Posture**: Abortado por la excesiva complejidad del engine de animaciones de towners que causaba parpadeos visuales.
- ❌ **Decorative House Removal**: Revertido para conservar la estética clásica intacta de Tristram.

---

## 🔨 GUÍA DE RECOMPILACIÓN EN ENTORNO WEB (WEBASSEMBLY)

```powershell
# Compilación mediante Ninja en WebAssembly (Emscripten)
ninja -C build-em
```

---
*Documentación oficial consolidada para Diablo Nightmare Edition - Agosto 2026.*
