# Plan de Implementación: NIGHTMARE — Restoration Layer (Ajuste Limpio)

Este documento establece el plan de integración quirúrgico y no invasivo para la **Capa de Restauración (Restoration Layer)** de `DevilutionX`. El objetivo es reactivar el contenido original dormido en los archivos del juego (sprites, audios WAV, cinemáticas SMK y diálogos recortados) sin alterar la arquitectura base, sin modificar mapas ni crear zonas artificiales.

---

## 🎯 Respuestas Clave de Diseño y Flujo Condicional

### 1. ¿En qué punto se activará cada elemento?
- **Tremain (El Sacerdote):**
  - **Tristram:** Aparece en las ruinas de la iglesia utilizando la geometría original existente.
  - **Disparador:** Se activa cuando el jugador entra al juego y alcanza la etapa adecuada (o nivel 5 de las Catacumbas).
  - **Quest & Diálogo:** Reproduce `sfx/towners/priest05.wav` (76 segundos originales) al hablar con él, desbloqueando el spawn de *Fleshdoom*.
- **Fleshdoom & Shadowfang (Catacumbas Niveles 5–7):**
  - Se genera de forma **procedural como un monstruo único (Unique Boss)** dentro de las Catacumbas estándar (sin subniveles ni mapas nuevos).
  - Al morir, entrega la espada única dormida `Shadowfang`.
- **Resolución de Tremain:**
  - Al regresar a Tristram con `Shadowfang`, Tremain reacciona (`priest07.wav`), desencadenando su desenlace final. En esa partida, el sacerdote no vuelve a aparecer.
- **Cinemática del Carnicero (`fbutch3.smk`):**
  - Se engancha directamente al **trigger de entrada a la sala del Carnicero (Nivel 2)**. Se reproduce una sola vez cuando el jugador abre la puerta por primera vez.
- **Wirt Invertido (`pegboy21.wav` / `pegboy22.wav`):**
  - Probabilidad baja (5%–10%) de activarse al hablar con Wirt **después** de haber visitado el nivel del Carnicero.
- **Atmósfera del Pueblo (`sfx/animals/*`):**
  - Sonidos de animales encolados en el mezclador de audio de Tristram mediante un temporizador aleatorio (cada 45 a 120 segundos).

### 2. ¿Estarán siempre habilitadas y disponibles todo el tiempo?
- **NO.** Ningún evento está habilitado permanentemente. Todo se rige por un **modelo dinámico guiado por el estado de la partida (Game State Driven)** para preservar la inmersión, evitar colisiones con las misiones originales y mantener la estabilidad del motor.

---

## 📐 Arquitectura de Capas de Restauración

```
┌────────────────────────────────────────────────────────┐
│             NIGHTMARE RESTORATION REGISTRY             │
├────────────────────────────────────────────────────────┤
│ 1. AUDIO HOOKS (MPQ originals):                        │
│    - sfx/towners/priest00.wav .. priest07.wav           │
│    - sfx/towners/pegboy21.wav, pegboy22.wav            │
│    - sfx/animals/*.wav (cow, dog, pig, sheep)          │
│                                                        │
│ 2. SPRITES & ASSETS:                                   │
│    - towners/priest/priest8.cel (Tremain)              │
│    - gendata/fbutch3.smk (Butcher Cinematic)           │
│                                                        │
│ 3. PROCEDURAL GAMEPLAY:                                │
│    - Catacombs 5-7: Fleshdoom Unique Boss + Shadowfang │
└────────────────────────────────────────────────────────┘
```

---

## Proposed Changes

### Layer 1: Core Restoration Registry & State Manager

#### [NEW] [Source/nightmare/restoration/registry.hpp](file:///c:/Projects/DevilutionX/Source/nightmare/restoration/registry.hpp)
#### [NEW] [Source/nightmare/restoration/registry.cpp](file:///c:/Projects/DevilutionX/Source/nightmare/restoration/registry.cpp)
- Declarar e implementar `LostContentRegistry` para estructurar la activación segura de diálogos, WAVs y cinemáticas dormidas.

---

### Layer 2: NPC Tremain & Fleshdoom Boss Integration

#### [NEW] [Source/nightmare/npcs/tremain.hpp](file:///c:/Projects/DevilutionX/Source/nightmare/npcs/tremain.hpp)
#### [NEW] [Source/nightmare/npcs/tremain.cpp](file:///c:/Projects/DevilutionX/Source/nightmare/npcs/tremain.cpp)
- Gestionar la aparición condicional del sprite `priest8.cel` en la iglesia de Tristram.
- Enlazar la interacción de voz de Tremain (`priest00.wav` - `priest07.wav`) y entregar la recompensa `Lightforge` tras el desenlace.

#### [MODIFY] [Source/monsters.cpp](file:///c:/Projects/DevilutionX/Source/monsters.cpp)
#### [MODIFY] [Source/quests.cpp](file:///c:/Projects/DevilutionX/Source/quests.cpp)
- Registrar el spawn procedural del jefe único `Fleshdoom` en las Catacumbas (niveles 5 a 7) con la caída garantizada de `Shadowfang`.

---

### Layer 3: Audio, Video & Tristram Atmosphere Triggers

#### [MODIFY] [Source/movie.cpp](file:///c:/Projects/DevilutionX/Source/movie.cpp)
#### [MODIFY] [Source/quests.cpp](file:///c:/Projects/DevilutionX/Source/quests.cpp)
- Enganchar la reproducción del video `gendata/fbutch3.smk` al trigger de primera apertura de la puerta de la sala del Carnicero.

#### [MODIFY] [Source/towners.cpp](file:///c:/Projects/DevilutionX/Source/towners.cpp)
- Incorporar la variante con 5%–10% de probabilidad de voz invertida en Wirt (`pegboy21.wav` / `pegboy22.wav`) tras descubrir la sala del Carnicero.

#### [MODIFY] [Source/engine/render/weather_overlay.cpp](file:///c:/Projects/DevilutionX/Source/engine/render/weather_overlay.cpp)
#### [MODIFY] [Source/sound.cpp](file:///c:/Projects/DevilutionX/Source/sound.cpp)
- Encolar efectos de audio distantes de animales (`sfx/animals/*`) en Tristram con un temporizador aleatorio no invasivo (45s–120s).

---

## Verification Plan

### Automated Tests / Compilación
- Compilar la versión nativa en Windows con Ninja:
  ```powershell
  cmake -S . -B build_COMPILE_FRESH -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DNONET=ON -DNOSOUND=OFF -DBUILD_TESTING=OFF -DDISABLE_ZERO_TIER=ON -DUNPACKED_MPQS=OFF -DUNPACKED_SAVES=OFF -DDEVILUTIONX_STATIC_BZIP2=ON -DDEVILUTIONX_SYSTEM_BZIP2=OFF
  cmake --build build_COMPILE_FRESH -j 4
  ```

### Manual Verification
1. **Verificación de Tremain & Shadowfang:**
   - Iniciar una partida, verificar la presencia de Tremain en la iglesia de Tristram con el sprite `priest8.cel` y su audio de diálogo.
   - Descender a las Catacumbas (niveles 5–7), comprobar el spawn de *Fleshdoom* y la entrega de *Shadowfang*.
   - Volver a Tristram con *Shadowfang* y validar la secuencia de desenlace (`priest07.wav`).
2. **Verificación de Cinemática del Carnicero:**
   - Abrir la puerta de la habitación del Carnicero en el nivel 2 y confirmar la reproducción de `fbutch3.smk`.
3. **Verificación de Wirt y Audio Ambiental:**
   - Visitar a Wirt tras el Carnicero y probar la activación condicional de la línea invertida.
   - Permanecer en Tristram y comprobar la reproducción ambiental sutil de los audios de animales sin interrupciones ni lag.
