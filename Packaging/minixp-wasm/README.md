# Mini Windows XP - WebAssembly x86 (v86)

Entorno de ejecución de Mini Windows XP (LiveCD / WinPE) en el navegador mediante emulación WebAssembly x86 con [v86](https://github.com/copy/v86).

## Archivos del Módulo

- **`index.html`**: Interfaz de emulación en el navegador, controles de pantalla completa, guardado/restauración de snapshots `.bin` y selector de ISO.
- **`v86.wasm`**: Motor de emulación de CPU x86 y periféricos compilado a WebAssembly.
- **`libv86.js`**: Controlador JavaScript para v86.
- **`seabios.bin`**: BIOS x86 para arranque de hardware virtual.
- **`vgabios.bin`**: BIOS de video compatible con VESA/VBE.
- **`server.py`**: Servidor local con soporte de HTTP Range Requests (`206 Partial Content`) para lectura por bloques de la ISO.
- **`download_assets.py`**: Script de utilidad para descargar o actualizar los binarios de v86.

## Cómo compilar y ejecutar localmente

1. **Instalar dependencias de Python:**
   ```bash
   pip install pycdlib
   ```

2. **Generar la ISO desde tu carpeta de Windows XP:**
   ```bash
   python tools/build_minixp_iso.py "C:\Ruta\A\Tu\Carpeta_MiniXP" "Packaging/minixp-wasm/minixp.iso"
   ```

3. **Iniciar el servidor local:**
   ```bash
   python Packaging/minixp-wasm/server.py
   ```

4. **Abrir en el navegador:**
   Accede a `http://localhost:8000` o `http://localhost:8000/Packaging/minixp-wasm/index.html`.
