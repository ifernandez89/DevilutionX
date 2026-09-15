# 🌀 Guía de Integración: Sega Dreamcast 128-Bit & Ecco the Dolphin

Esta guía documenta la implementación completa del ecosistema **Sega Dreamcast** en el proyecto, proporcionando una solución dual:
1. **Entorno Nativo de Alto Rendimiento (Windows x64)**: Optimizado para ejecutar de inmediato a 60 FPS y en resoluciones HD/4K el título **Ecco the Dolphin: Defender of the Future** (`.cdi`) mediante **Flycast x64 oficial**.
2. **Reproductor WebAssembly (Retro Hub)**: Plataforma web integrada en `Packaging/emscripten/dreamcast/` conectada al catálogo global de consolas retro (`retro-nav.js`).

---

## 🐬 1. Cómo Jugar a *Ecco the Dolphin* en PC (Redream & Flycast)

Tienes a tu disposición dos emuladores nativos de alto rendimiento en Windows x64:

### Opción A (Recomendada): Redream x64
- **Lanzador directo**: Haz doble clic en **[`play_ecco_redream.bat`](file:///c:/Projects/DevilutionX/play_ecco_redream.bat)**.
- Detecta automáticamente tu imagen `.chd` de 552 MB:
  `C:\Users\xiphos-pc1\Downloads\Ecco the Dolphin - Defender of the Future (USA) (En,Fr,De,Es).chd`
- **Lanzador general**: **[`run_redream.bat`](file:///c:/Projects/DevilutionX/run_redream.bat)** (puedes arrastrar cualquier juego sobre él).

#### 🎮 Controles por Defecto de Redream (Ecco the Dolphin)
| Función en Ecco | Tecla (Teclado) | Botón Dreamcast | Mando Xbox | Mando PlayStation |
| :--- | :--- | :--- | :--- | :--- |
| **Nadar / Mover a Ecco** | `W` `A` `S` `D` | **Palanca Analógica** | Stick Izquierdo | Stick Izquierdo |
| **Nadar rápido / Acelerar / Seleccionar** | `K` | **Botón A** | `A` (Verde) | `✖` (Cruz) |
| **Embestida / Ataque con el morro / Dash** | `L` | **Botón B** | `B` (Rojo) | `●` (Círculo) |
| **Sonar / Ecolocalización / Cantar** | `J` | **Botón X** | `X` (Azul) | `■` (Cuadrado) |
| **Cámara / Vista en 3ª persona** | `P` o `I` | **Botón Y** | `Y` (Amarillo) | `▲` (Triángulo) |
| **Pausa / Menú del juego** | **Barra Espaciadora** | **START** | `Start` | `Options` |
| **Giro acrobático izquierdo** | `Q` | **Gatillo L** | `LT` | `L2` |
| **Giro acrobático derecho** | `E` | **Gatillo R** | `RT` | `R2` |
| **Cruceta de menús** | Flechas del teclado | **D-Pad** | Cruceta | Cruceta |
| **Menú de Configuración de Redream** | **`ESC`** | Menú OSD | Guía | PS |

#### ⚙️ ¿Cómo ver o cambiar los controles en Redream?
1. Durante el juego, presiona la tecla **`ESC`** (Escape). Aparecerá el menú interactivo de Redream.
2. En la barra superior, haz clic en la pestaña **INPUT**.
3. En **Port A (Controller 0)**, haz clic en **CUSTOMIZE BINDS**.
4. Haz clic sobre cualquier botón ilustrado en pantalla y presiona la tecla o botón de tu mando que desees asignar.
5. Para volver a jugar, presiona **`ESC`** nuevamente o selecciona **RESUME**.

---

### Opción B: Flycast x64
- **Lanzador directo**: **[`play_ecco_dreamcast.bat`](file:///c:/Projects/DevilutionX/play_ecco_dreamcast.bat)**
- **Lanzador general**: **[`run_flycast.bat`](file:///c:/Projects/DevilutionX/run_flycast.bat)**
- Menú de opciones de Flycast: tecla **`Tab`** o **`Escape`**.

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
