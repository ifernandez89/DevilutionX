# Corrección Crítica del Sistema de Guardado (Sega Genesis, NES y SNES WebAssembly)

## 📌 Contexto y Diagnóstico del Error
Al guardar y cargar partidas en el emulador de **Sega Genesis / Mega Drive** (por ejemplo en títulos como *Road Rash 3*), la partida restauraba correctamente el fotograma en pantalla, pero al intentar comenzar la siguiente carrera, nivel o mapa, el juego quedaba completamente tildado/congelado.

### 🔍 Causas Raíz Detectadas:
1. **Ausencia de persistencia de Batería SRAM (Cartridge RAM):**
   * Juegos de 16 bits y 8 bits (*Road Rash 3*, *Sonic 3*, *Phantasy Star*, *The Legend of Zelda*, *Super Mario World*, *Chrono Trigger*) dependen de la memoria no volátil SRAM del cartucho para registrar avance entre carreras/mapas.
   * El sistema previo solo almacenaba capturas de memoria (SaveStates) y no cargaba ni sincronizaba la memoria SRAM en el arranque ni durante la partida.
   * Al restaurar un SaveState desincronizado con una SRAM vacía, la CPU 68000 entraba en un bucle infinito (*spin-lock*) esperando la respuesta del coprocesador de sonido Z80 y la confirmación de escritura en SRAM.

2. **Excepciones de 68k Address Error en Juegos de Electronic Arts (EA):**
   * Los motores de EA (*Road Rash*, *Desert Strike*, etc.) realizan lecturas y escrituras desalineadas de 16 bits.
   * En `genesis_plus_gx`, si la emulación estricta de errores de dirección no está desactivada (`genesis_plus_gx_addr_error = disabled`) y no se asegura el handshake DTACK (`genesis_plus_gx_force_dtack = enabled`), el cambio de mapa tras un guardado provocaba una excepción de hardware congelando la emulación.

3. **Condiciones de Carrera por Restauración en Caliente:**
   * Al restaurar estados mientras el bucle de audio y video de RetroArch corría a 60 FPS sin pausa, los registros del APU/SPC700/YM2612 quedaban desalineados con respecto al búfer de WebAudio.
   * La opción `video_threaded: true` causaba desincronización en el hilo único de WebAssembly.

---

## 🛠️ Solución Implementada

### 1. Sistema Bidireccional de Batería SRAM (IndexedDB)
* **Arranque Inteligente:** Al iniciar cualquier ROM en `gens`, `nes` o `snes`, se consulta IndexedDB (`gens_sram_<rom>`, `nes_sram_<rom>`, `snes_sram_<rom>`) e inyecta la memoria SRAM al core antes del booteo del cartucho mediante la opción `sram` de Nostalgist.
* **Autoguardado Periódico y al Salir:** Cada 20 segundos y en los eventos `beforeunload`/`exitToHub`, se ejecuta `saveSRAM()` y `saveState()` para garantizar que ningún avance se pierda.

### 2. Configuración de Núcleos y Estabilidad de WebAssembly
* **Sega Genesis (`genesis_plus_gx`):**
  * `genesis_plus_gx_addr_error: 'disabled'`
  * `genesis_plus_gx_force_dtack: 'enabled'`
  * `genesis_plus_gx_bram: 'per_game'`
  * `genesis_plus_gx_ym2612: 'mame'`
* **NES (`nestopia` / `fceumm`):**
  * `nestopia_ram_power_state: '0x00'`
  * `fceumm_nospritelimit: 'disabled'`
* **SNES (`snes9x`):**
  * `snes9x_block_invalid_vram_access: 'enabled'`
  * `snes9x_overclock_cycles: 'disabled'`
* **Video & Audio Global:**
  * `video_threaded: 'false'` (evita desincronizaciones en WebAssembly)
  * `audio_latency: '64'`
  * `autosave_interval: '10'`

### 3. Protocolo Limpio de Restauración de Estados (`loadGameStateCleanly`)
* Implementación de una secuencia segura de carga:
  1. Pausa de la emulación (`currentEmulator.pause()`) para silenciar DMA e interrupciones.
  2. Restauración de memoria WebAssembly (`currentEmulator.loadState()`).
  3. Micro-pausa de 60ms para estabilización de buses de sonido y registros.
  4. Reanudación suave (`currentEmulator.resume()`).

---

## 📁 Archivos Modificados
* `Packaging/emscripten/gens/app.js`
* `Packaging/emscripten/nes/app.js`
* `Packaging/emscripten/snes/app.js`
* `RETRO_EMULATORS_SRAM_FIX.md` (este documento)
