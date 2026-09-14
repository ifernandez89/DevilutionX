# 🌀 Guía de Integración: Sega Dreamcast 128-Bit & Ecco the Dolphin

Esta guía documenta la implementación completa del ecosistema **Sega Dreamcast** en el proyecto, proporcionando una solución dual:
1. **Entorno Nativo de Alto Rendimiento (Windows x64)**: Optimizado para ejecutar de inmediato a 60 FPS y en resoluciones HD/4K el título **Ecco the Dolphin: Defender of the Future** (`.cdi`) mediante **Flycast x64 oficial**.
2. **Reproductor WebAssembly (Retro Hub)**: Plataforma web integrada en `Packaging/emscripten/dreamcast/` conectada al catálogo global de consolas retro (`retro-nav.js`).

---

## 🐬 1. Cómo Jugar a *Ecco the Dolphin* (Entorno Nativo Windows x64)

### Inicio Rápido en 1 Clic
En la raíz del proyecto encontrarás el script lanzador:
- Ejecuta con doble clic: **[`play_ecco_dreamcast.bat`](file:///c:/Projects/DevilutionX/play_ecco_dreamcast.bat)**

El script detecta de forma automática la imagen de disco en:
```
C:\Users\xiphos-pc1\Downloads\Ecco the Dolphin - Defender of the Future\Ecco the Dolphin - Defender of the Future.cdi
```
E inicia el motor **Flycast x64** con aceleración de hardware activa.

---

### Configuración General de Flycast
Para abrir el emulador de forma independiente, configurar gráficos o agregar más juegos:
- Ejecuta: **[`run_flycast.bat`](file:///c:/Projects/DevilutionX/run_flycast.bat)**

También puedes arrastrar cualquier imagen `.cdi`, `.gdi` o `.chd` sobre `run_flycast.bat` para iniciarla directamente.

---

### Controles Predeterminados (Teclado y Mandos)

| Botón Dreamcast | Tecla (Teclado) | Mando Xbox / XInput | Mando PlayStation |
| :--- | :--- | :--- | :--- |
| **D-Pad / Stick** | Flechas del Teclado | D-Pad / Stick Izquierdo | Cruceta / Stick Izquierdo |
| **Botón A** | `X` | `A` (Verde) | `✖` (Cruz) |
| **Botón B** | `Z` | `B` (Rojo) | `●` (Círculo) |
| **Botón X** | `S` | `X` (Azul) | `■` (Cuadrado) |
| **Botón Y** | `A` | `Y` (Amarillo) | `▲` (Triángulo) |
| **Gatillo L** | `Q` | `LT` / Gatillo Izquierdo | `L2` |
| **Gatillo R** | `W` / `E` | `RT` / Gatillo Derecho | `R2` |
| **Start** | `Enter` | `Start` | `Options` |

#### Atajos de Teclado del Emulador
- **Alt + Enter**: Alternar Pantalla Completa.
- **Escape / Tab**: Abrir Menú de Configuración de Flycast (Video, Audio, Controles).
- **F5**: Guardado Rápido (Save State).
- **F7**: Carga Rápida (Load State).

---

## 🌐 2. Reproductor WebAssembly (Retro Hub)

Se ha integrado un nuevo reproductor WebAssembly para **Sega Dreamcast** en:
- **Ruta**: [`Packaging/emscripten/dreamcast/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/dreamcast/index.html)
- **Hoja de Estilos**: [`Packaging/emscripten/dreamcast/style.css`](file:///c:/Projects/DevilutionX/Packaging/emscripten/dreamcast/style.css)
- **Controlador**: [`Packaging/emscripten/dreamcast/app.js`](file:///c:/Projects/DevilutionX/Packaging/emscripten/dreamcast/app.js)

### Características del Reproductor Web
- **Formatos Soportados**: `.CDI`, `.GDI`, `.CHD`, `.ISO`, `.CUE`.
- **Selector de Pantalla Dinámica**:
  - `Normal (800p)`: Resolución nítida y ligera.
  - `Grande (980p)`: Tamaño óptimo por defecto para monitores 1080p.
  - `Cinema (1180p)`: Experiencia panorámica extendida.
  - `Pantalla Completa`: Modo inmersivo sin bordes.
- **Visual Memory Unit (VMU)**: Panel LCD interactivo con simulación de VMU y botón para exportar respaldos `.bin` de tus partidas.
- **Integración con Retro Nav**: Registrado bajo `🌀 SEGA 128-BIT (Motor Flycast / Dreamcast WASM)` en el menú desplegable común a Diablo, PSX, N64, SNES, NES, Genesis, DOOM y Quake.

---

## 📦 3. Archivos y Estructura Generada

```
c:\Projects\DevilutionX\
├── flycast-win64\
│   └── flycast.exe                   # Binario oficial de Flycast x64 v2.7
├── play_ecco_dreamcast.bat           # Lanzador directo 1-clic para Ecco the Dolphin
├── run_flycast.bat                   # Lanzador GUI / Dropzone de Flycast
├── SEGA_DREAMCAST_SETUP.md           # Este manual técnico de uso y controles
└── Packaging\emscripten\
    ├── assets\retro-nav\
    │   └── retro-nav.js              # Actualizado con categoría Sega Dreamcast 128-Bit
    └── dreamcast\
        ├── index.html                # Interfaz web del reproductor Dreamcast
        ├── style.css                 # Estilos visuales Swirl naranja/azul
        └── app.js                    # Lógica de carga GD-ROM y VMU saves
```
