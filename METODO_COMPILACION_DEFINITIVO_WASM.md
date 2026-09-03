# 🌐 MÉTODO DE COMPILACIÓN DEFINITIVO - WEBASSEMBLY (EMSCRIPTEN / WASM)
## 🎮 DEVILUTIONX PARA NAVEGADOR WEB Y GITHUB PAGES

### 🚨 GUÍA OFICIAL Y DEFINITIVA PARA COMPILAR `.wasm` DIRECTAMENTE DESDE C++

---

## 📌 INTRODUCCIÓN

Este documento detalla el procedimiento exacto, probado y reproducible para compilar el código fuente en **C++** de DevilutionX a **WebAssembly (`devilutionx.wasm` y `devilutionx.js`)**.

Al dominar este pipeline, cualquier cambio que hagamos en el código fuente de C++ (por ejemplo, en [`Source/engine/render/scrollrt.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/scrollrt.cpp) para desactivar la lluvia vieja de software, o en [`Source/nightmare/neural/gbuffer.cpp`](file:///c:/Projects/DevilutionX/Source/nightmare/neural/gbuffer.cpp) para exportar buffers G-Buffer hacia WebGPU) se reflejará **directamente en la versión Web / GitHub Pages**.

---

## 🛠️ 1. HERRAMIENTAS Y PRERREQUISITOS

En este sistema ya disponemos de:
- ✅ **CMake**: `C:\Program Files\CMake\bin\cmake.exe`
- ✅ **Ninja**: `C:\Users\xiphos-pc1\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe\ninja.exe`
- ✅ **Git**: `C:\Program Files\Git\cmd\git.exe`
- ⏳ **Emscripten SDK (emsdk)**: Versión recomendada **`3.1.53`** (la misma versión utilizada en el workflow oficial de CI [`.github/workflows/deploy-pages.yml`](file:///c:/Projects/DevilutionX/.github/workflows/deploy-pages.yml)).

---

## 📦 2. INSTALACIÓN DE EMSCRIPTEN (EMSDK)

Para instalar Emscripten de forma limpia y aislada dentro del proyecto:

```bash
# 1. Clonar el repositorio oficial de emsdk en la carpeta deps o en la raíz
git clone https://github.com/emscripten-core/emsdk.git deps/emsdk

# 2. Entrar al directorio
cd deps/emsdk

# 3. Descargar e instalar la versión exacta 3.1.53 (descarga automáticamente Python portátil si no existe)
./emsdk.bat install 3.1.53

# 4. Activar la versión
./emsdk.bat activate 3.1.53
```

> [!TIP]
> Cada vez que abras una nueva consola de PowerShell o CMD para compilar WASM, solo debes ejecutar:
> ```powershell
> & "c:\Projects\DevilutionX\deps\emsdk\emsdk_env.bat"
> ```
> Esto añadirá `emcc`, `emcmake` y `embuilder` temporalmente a tu PATH.

---

## ⚙️ 3. CONFIGURACIÓN Y COMPILACIÓN EXACTA (WASM)

Una vez activado el entorno de Emscripten, la configuración utiliza **Ninja** y los puertos nativos de SDL2 y ZLIB gestionados por Emscripten:

### Paso A: Pre-compilar los Ports del Sistema Emscripten
```bash
embuilder build zlib bzip2 libpng sdl2 sdl2_image sdl2_image_png
```

### Paso B: Configurar CMake con Emscripten (`emcmake`)
```bash
emcmake cmake -S . -B build-web -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DASAN=OFF \
  -DUBSAN=OFF \
  -DTSAN=OFF \
  -DDISABLE_ZERO_TIER=ON \
  -DDISABLE_TCP=ON \
  -DDEVILUTIONX_SYSTEM_SDL_AUDIOLIB=OFF \
  -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF \
  -DDEVILUTIONX_SYSTEM_LIBFMT=OFF \
  -DDEVILUTIONX_SYSTEM_BZIP2=OFF
```

### Paso C: Compilar el Binario WebAssembly
```bash
cmake --build build-web --target devilutionx -j 4
```

---

## 📂 4. DISTRIBUCIÓN Y DESPLIEGUE A `Packaging/emscripten/`

Tras una compilación exitosa, CMake generará los siguientes archivos en `build-web/`:
1. `build-web/devilutionx.wasm` (~6 MB - Código compilado C++ en bytecode WASM).
2. `build-web/devilutionx.js` (~240 KB - Pegamento de enlace JavaScript/Emscripten).
3. `build-web/devilutionx.data` (Si se empaquetan assets por defecto).

### Script de sincronización con el servidor local:
```powershell
Copy-Item build-web/devilutionx.wasm Packaging/emscripten/devilutionx.wasm -Force
Copy-Item build-web/devilutionx.js   Packaging/emscripten/devilutionx.js -Force
if (Test-Path "build-web/devilutionx.data") {
    Copy-Item build-web/devilutionx.data Packaging/emscripten/devilutionx.data -Force
}
Write-Host "✅ Archivos WASM actualizados con éxito en Packaging/emscripten/"
```

---

## 🎯 5. DIFERENCIA CLAVE ENTRE AMBOS ENTORNOS DE COMPILACIÓN

| Característica | Versión Windows Nativa (`.exe`) | Versión WebAssembly (`.wasm`) |
| :--- | :--- | :--- |
| **Documentación** | [`METODO_COMPILACION_DEFINITIVO_FINAL.md`](file:///c:/Projects/DevilutionX/METODO_COMPILACION_DEFINITIVO_FINAL.md) | `METODO_COMPILACION_DEFINITIVO_WASM.md` |
| **Compilador** | MinGW GCC (`C:\mingw64\bin\c++.exe`) | Emscripten Clang/LLVM (`emcc`) |
| **Generador CMake** | `MinGW Makefiles` | `Ninja` |
| **Carpeta de Build** | `build_NOW/` o `build_COMPILE_FRESH/` | `build-web/` |
| **Salida** | `devilutionx.exe` (5.2 MB) | `devilutionx.wasm` (6.0 MB) + `devilutionx.js` |
| **Entorno de Ejecución** | Escritorio Windows directo | Navegador Web (`localhost:8080` / GitHub Pages) |

---

## 🚀 6. SCRIPTS AUTOMATIZADOS DE 1 CLIC

Para no tener que recordar comandos extensos, se proporcionan los siguientes scripts en `scripts/`:
- `scripts/setup_emsdk.bat`: Clona, instala y prepara Emscripten 3.1.53.
- `scripts/build_wasm.bat`: Activa el entorno, compila `build-web` y actualiza automáticamente `Packaging/emscripten/`.
