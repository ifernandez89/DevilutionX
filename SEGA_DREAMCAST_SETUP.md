# 🌀 Guía de Integración: Sega Dreamcast 128-Bit & Redream

Esta guía documenta la implementación completa del ecosistema **Sega Dreamcast** en el proyecto, proporcionando una solución dual:
1. **Entorno Nativo de Alto Rendimiento (Windows x64)**: Optimizado para ejecutar de inmediato a 60 FPS estables y en resoluciones escaladas el título **Ecco the Dolphin: Defender of the Future** (`.chd` / `.cdi`) mediante **Redream x64 oficial** (sustituto oficial y directo de Flycast).
2. **Reproductor WebAssembly (Retro Hub)**: Plataforma web integrada en `Packaging/emscripten/dreamcast/` conectada al catálogo global de consolas retro (`retro-nav.js`).

---

## 🐬 1. Cómo Jugar a *Ecco the Dolphin* (Entorno Nativo Windows x64 con Redream)

### Ventajas Clave de Redream
- **Cero Configuración de BIOS:** Trae implementada una BIOS integrada de alta compatibilidad (HLE). No requiere buscar ni copiar archivos `dc_boot.bin` ni `dc_flash.bin`.
- **Soporte CHD Directo y Ligero:** Carga al instante las imágenes `.chd` comprimidas sin pérdidas del estándar Redump (~550 MB vs 1.1 GB en GDI).
- **Mapeo Automático:** Reconoce mandos de Xbox, PlayStation, Switch Pro y genéricos al conectarlos por USB o Bluetooth.
- **Rendimiento Impecable:** Motor 3D ultra optimizado que mantiene 60 FPS estables sin micro-tirones ni desincronización de audio AICA.

---

### Inicio Rápido en 1 Clic
En la raíz del proyecto encontrarás los lanzadores directos:
- **[`play_ecco_redream.bat`](file:///c:/Projects/DevilutionX/play_ecco_redream.bat)** o **[`play_ecco_dreamcast.bat`](file:///c:/Projects/DevilutionX/play_ecco_dreamcast.bat)**

El script detecta de forma automática la imagen de disco en:
- `D:\Hours of Fun\Roms\Sega Dreamcast\`
- `C:\Users\xiphos-pc1\Downloads\` (incluyendo subcarpetas)
- La raíz del proyecto

Si Redream no estuviera instalado, el script lo descargará e instalará de forma automática desde su repositorio oficial en `./redream-win64/`.

---

### 🌐 Enlace de Preservación y Descarga de ROMs CHD (Redump)
Para obtener la versión oficial en formato comprimido `.chd`:
- **Directorio Completo de Descarga Directa Redump CHD:**  
  [https://archive.org/download/sega-dreamcast-redump-collection](https://archive.org/download/sega-dreamcast-redump-collection)
- **Archivos disponibles de Ecco:**
  - `Ecco the Dolphin - Defender of the Future (USA) (En,Fr,De,Es).chd` (~552 MB)
  - `Ecco the Dolphin - Defender of the Future (Europe) (En,Fr,De,Es).chd` (~551 MB)
  - `Ecco the Dolphin - Defender of the Future (Japan).chd` (~550 MB)

---

### 🎮 Controles Predeterminados de Redream (Ecco the Dolphin)

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

### Configuración General de Redream
Para abrir el emulador de forma independiente, añadir directorios de juegos o configurar mandos:
- Ejecuta: **[`run_redream.bat`](file:///c:/Projects/DevilutionX/run_redream.bat)**

También puedes arrastrar cualquier imagen `.chd`, `.cdi` o `.gdi` sobre `run_redream.bat` para iniciarla directamente.

---

## 🌐 2. Reproductor WebAssembly (Retro Hub)

Se ha integrado el reproductor WebAssembly para **Sega Dreamcast** en:
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
- **Integración con Retro Nav**: Registrado bajo `🌀 SEGA 128-BIT (Motor Redream / Dreamcast WASM)` en el menú desplegable común a Diablo, PSX, N64, SNES, NES, Genesis, DOOM y Quake.

---

## 📦 3. Archivos y Estructura Generada

```
c:\Projects\DevilutionX\
├── redream-win64\
│   └── redream.exe                   # Binario oficial de Redream x64 v1.5.0
├── play_ecco_redream.bat             # Lanzador directo 1-clic para Ecco the Dolphin (Redream)
├── play_ecco_dreamcast.bat           # Lanzador directo para Ecco the Dolphin
├── run_redream.bat                   # Lanzador GUI / Dropzone de Redream
├── SEGA_DREAMCAST_SETUP.md           # Manual técnico de uso, enlaces CHD y controles
└── Packaging\emscripten\
    ├── assets\retro-nav\
    │   └── retro-nav.js              # Actualizado con categoría Sega Dreamcast 128-Bit
    └── dreamcast\
        ├── index.html                # Interfaz web del reproductor Dreamcast
        ├── style.css                 # Estilos visuales Swirl naranja/azul y guía de teclas
        └── app.js                    # Lógica de carga GD-ROM, modal informativo y VMU saves
```
