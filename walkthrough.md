# Walkthrough - NIGHTMARE — Restoration Layer (Ajuste Limpio)

Se ha implementado y verificado con éxito el módulo **NIGHTMARE — Restoration Layer** en `DevilutionX`.

## 🛠️ Cambios Implementados y Verificados

1. **LostContentRegistry & Módulo de Restauración**:
   - [Source/nightmare/restoration/registry.hpp](file:///c:/Projects/DevilutionX/Source/nightmare/restoration/registry.hpp) & [registry.cpp](file:///c:/Projects/DevilutionX/Source/nightmare/restoration/registry.cpp): Declaración formal del registro de contenido dormido y utilidades para disparar audios y cinemáticas.

2. **Tremain el Sacerdote & Misión de Fleshdoom/Shadowfang**:
   - [Source/nightmare/npcs/tremain.hpp](file:///c:/Projects/DevilutionX/Source/nightmare/npcs/tremain.hpp) & [tremain.cpp](file:///c:/Projects/DevilutionX/Source/nightmare/npcs/tremain.cpp):
     - Ubicación inicial en las ruinas de la iglesia de Tristram (`priest8.cel`).
     - Activación de líneas de voz originales (`priest00.wav` a `priest07.wav`).
     - Aparición procedural de *Fleshdoom* en Catacumbas (Niveles 5–7) entregando la espada *Shadowfang*.
     - Resolución y recompensa con la maza *Lightforge*.

3. **Cinemática del Carnicero (`fbutch3.smk`)**:
   - En [Source/quests.cpp](file:///c:/Projects/DevilutionX/Source/quests.cpp#L539-L541), la cinemática `gendata/fbutch3.smk` se engancha al primer ingreso a la sala del Carnicero (Nivel 2).

4. **Wirt Invertido (50% Probabilidad)**:
   - En [Source/towners.cpp](file:///c:/Projects/DevilutionX/Source/towners.cpp#L453-L457), al hablar con Wirt tras descubrir la sala del Carnicero, existe un **50% de probabilidad** de reproducir sus audios invertidos (`pegboy21.wav` / `pegboy22.wav`).

5. **Audio Ambiental de Tristram**:
   - [Source/nightmare/world/ambient_animals.hpp](file:///c:/Projects/DevilutionX/Source/nightmare/world/ambient_animals.hpp) & [ambient_animals.cpp](file:///c:/Projects/DevilutionX/Source/nightmare/world/ambient_animals.cpp): Cola sutil de audios distantes de animales (`sfx/animals/*`) cada 45s a 120s en Tristram.

---

## 🧪 Resultados de Verificación

- **Compilación Exitosa (WASM & Nativo):**
  - Generados correctamente en `build_COMPILE_FRESH/`:
    - `devilutionx.js` (220 KB)
    - `devilutionx.wasm` (4.45 MB)
- **Cero Regresiones:** Juego estable en PC y WebAssembly.
