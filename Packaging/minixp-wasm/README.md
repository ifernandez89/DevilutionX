# Tiny Core Linux — WebAssembly x86 PC (v86)

Entorno de ejecución y virtualización de **Tiny Core Linux (GUI LiveCD)** directamente en el navegador web mediante emulación de hardware PC x86 sobre **WebAssembly** usando el motor [v86](https://github.com/copy/v86).

---

## 📦 Archivos del Módulo

- **`index.html`**: Interfaz web moderna con barra de estado, telemetría, controles de captura de ratón, pantalla completa, gestión de snapshots de memoria y selector de imágenes ISO.
- **`app.js`**: Controlador de ejecución de WebAssembly, manejo de eventos no bloqueantes con `requestAnimationFrame`, telemetría de CPU y throttling de actividad de disco.
- **`style.css`**: Hoja de estilos con diseño oscuro, soporte responsivo y adaptaciones para el canvas de video VESA.
- **`tinycore.iso`**: Imagen oficial de Tiny Core Linux (~24 MB) con servidor gráfico X11 (`Xvesa`) y gestor de ventanas liviano `FLWM`.
- **`v86.wasm`**: Motor de CPU x86 de 32 bits, MMU y periféricos de PC compilado a WebAssembly.
- **`libv86.js`**: Biblioteca JavaScript oficial de interacción con el runtime WASM.
- **`seabios.bin`**: BIOS x86 open source compatible con SeaBIOS.
- **`vgabios.bin`**: BIOS de video compatible con extensiones VESA/VBE.
- **`server.js`**: Servidor Node.js local con soporte de streaming HTTP Range (`206 Partial Content`) y cabeceras COOP/COEP.
- **`server.py`**: Servidor Python alternativo para pruebas locales rápidas.

---

## ⚙️ Especificaciones de la Máquina Virtual

| Componente | Configuración |
| :--- | :--- |
| **Arquitectura CPU** | Intel Pentium II (x86 32-bit emulado) |
| **Memoria RAM** | 256 MB (Asignación en heap WebAssembly) |
| **Memoria VRAM** | 8 MB VESA / VBE |
| **Almacenamiento** | CD-ROM IDE (Streaming por bloques de 2 KB) |
| **Entrada** | Teclado PS/2 + Ratón PS/2 (Pointer Lock API) |
| **Audio** | Sound Blaster 16 emulada con Web Audio API |

---

## 🚀 Cómo ejecutar localmente

### Opción 1: Con Node.js (Recomendada)
```bash
node server.js
```
Accede desde tu navegador a:
👉 `http://localhost:8000`

### Opción 2: Con Python 3
```bash
python server.py
```
Accede desde tu navegador a:
👉 `http://localhost:8000`

---

## 🌐 Despliegue en GitHub Pages

Este módulo es **100% estático** y funciona de forma nativa en **GitHub Pages**:

1. Sube la carpeta del proyecto a tu repositorio de GitHub Pages.
2. Todas las rutas a los archivos (`tinycore.iso`, `v86.wasm`, `seabios.bin`, `vgabios.bin`, `libv86.js`) están configuradas con rutas relativas (`./`), por lo que funcionan en cualquier subdirectorio o dominio raíz.
3. GitHub Pages soporta de forma nativa peticiones HTTP Range (`Range: bytes=...`), permitiendo que el navegador descargue solo los sectores de la ISO que el kernel de Tiny Core solicita durante el arranque.

---

## 💾 Guardado y Restauración de Estados (Snapshots)

- **Guardar Estado (`💾 Guardar Estado`)**: Vuelca el estado completo de los 256 MB de RAM y registros de CPU a un archivo binario (`.bin`).
- **Restaurar Estado (`📂 Cargar Estado`)**: Permite reanudar la sesión exactamente en el milisegundo en que se guardó, sin necesidad de volver a bootear la ISO.
