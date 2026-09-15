# 🌀 Guía de Integración: Sega Dreamcast 128-Bit & Redream

Esta guía documenta la implementación completa del ecosistema **Sega Dreamcast** en el proyecto, proporcionando una solución dual:
1. **Entorno Nativo de Alto Rendimiento (Windows x64)**: Optimizado para ejecutar de inmediato a 60 FPS estables y en resoluciones escaladas el título **Ecco the Dolphin: Defender of the Future** (`.chd` / `.cdi`) mediante **Redream x64 oficial** (sustituto oficial y directo de Flycast).
2. **Reproductor WebAssembly (Retro Hub)**: Plataforma web integrada en `Packaging/emscripten/dreamcast/` conectada al catálogo global de consolas retro (`retro-nav.js`).

---

## 🐬 1. Cómo Jugar a *Ecco the Dolphin* (Entorno Nativo Windows x64 con Redream)

### Ventajas Clave de Redream frente a Flycast
- **Cero Configuración de BIOS:** Trae implementada una BIOS integrada de alta compatibilidad (HLE). No requiere buscar ni copiar archivos `dc_boot.bin` ni `dc_flash.bin`.
- **Soporte CHD Directo y Ligero:** Carga al instante las imágenes `.chd` comprimidas sin pérdidas del estándar Redump (~550 MB vs 1.1 GB en GDI).
- **Mapeo Automático:** Reconoce mandos de Xbox, PlayStation, Switch Pro y genéricos al conectarlos por USB o Bluetooth.
- **Rendimiento Impecable:** Motor 3D ultra optimizado que mantiene 60 FPS estables sin micro-tirones ni desincronización de audio AICA.

---

### Inicio Rápido en 1 Clic
En la raíz del proyecto encontrarás el script lanzador:
- Ejecuta con doble clic: **[`play_ecco_dreamcast.bat`](file:///c:/Projects/DevilutionX/play_ecco_dreamcast.bat)**

El script detecta de forma automática la imagen de disco en:
- `D:\Hours of Fun\Roms\Sega Dreamcast\`
- `C:\Users\xiphos-pc1\Downloads\` (incluyendo subcarpetas)
- La raíz del proyecto

Si Redream no estuviera instalado, el script lo descargará e instalará de forma automática desde su repositorio oficial en `./redream-win64/`.

---

### Enlace de Preservación y Descarga de ROMs CHD (Redump)
Para obtener la versión oficial en formato comprimido `.chd`:
- **Directorio Completo de Descarga Directa Redump CHD:**  
  [https://archive.org/download/sega-dreamcast-redump-collection](https://archive.org/download/sega-dreamcast-redump-collection)
- **Archivos disponibles de Ecco:**
  - `Ecco the Dolphin - Defender of the Future (USA) (En,Fr,De,Es).chd` (~552 MB)
  - `Ecco the Dolphin - Defender of the Future (Europe) (En,Fr,De,Es).chd` (~551 MB)
  - `Ecco the Dolphin - Defender of the Future (Japan).chd` (~550 MB)

---

### Configuración General de Redream
Para abrir el emulador de forma independiente, añadir directorios de juegos o configurar mandos:
- Ejecuta: **[`run_redream.bat`](file:///c:/Projects/DevilutionX/run_redream.bat)**

También puedes arrastrar cualquier imagen `.chd`, `.cdi` o `.gdi` sobre `run_redream.bat` para iniciarla directamente.

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
- **Escape**: Abrir Menú Rápido de Redream (Ajustes, Controles, Salir).
- **F5 / F7**: Guardado / Carga rápida de estados.

---

## 🌐 2. Reproductor WebAssembly (Retro Hub)

Se ha integrado el reproductor WebAssembly para **Sega Dreamcast** en:
- **Ruta**: [`Packaging/emscripten/dreamcast/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/dreamcast/index.html)
- **Hoja de Estilos**: [`Packaging/emscripten/dreamcast/style.css`](file:///c:/Projects/DevilutionX/Packaging/emscripten/dreamcast/style.css)
- **Controlador**: [`Packaging/emscripten/dreamcast/app.js`](file:///c:/Projects/DevilutionX/Packaging/emscripten/dreamcast/app.js)

---

## 📦 3. Archivos y Estructura del Proyecto

```
c:\Projects\DevilutionX\
├── redream-win64\
│   └── redream.exe                   # Binario oficial de Redream x64 v1.5.0
├── play_ecco_dreamcast.bat           # Lanzador directo 1-clic para Ecco the Dolphin (Redream)
├── run_redream.bat                   # Lanzador GUI / Dropzone oficial de Redream
├── run_flycast.bat                   # Reenvío de compatibilidad hacia run_redream.bat
├── SEGA_DREAMCAST_SETUP.md           # Este manual técnico de uso, enlaces CHD y controles
└── Packaging\emscripten\
    ├── assets\retro-nav\
    │   └── retro-nav.js              # Actualizado con categoría Sega Dreamcast 128-Bit
    └── dreamcast\
        ├── index.html                # Interfaz web del reproductor Dreamcast
        ├── style.css                 # Estilos visuales Swirl naranja/azul
        └── app.js                    # Lógica de carga GD-ROM y VMU saves
```
