# 🏆 GUÍA DEFINITIVA Y REPRODUCIBLE DE COMPILACIÓN - DEVILUTIONX (2026)

Esta guía explica paso a paso y con total claridad el método exacto y verificado para compilar y ejecutar **DevilutionX** en Windows de forma 100% sana, funcional y reproducible.

---

## 📋 COMANDO MAESTRO DE CONFIGURACIÓN CMAKE

Ejecutar desde PowerShell en la raíz del repositorio (`C:\Projects\DevilutionX`):

```powershell
cmake -S . -B build_COMPILE_FRESH -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DNONET=ON `
  -DNOSOUND=OFF `
  -DBUILD_TESTING=OFF `
  -DDISABLE_ZERO_TIER=ON `
  -DUNPACKED_MPQS=OFF `
  -DUNPACKED_SAVES=OFF `
  -DDEVILUTIONX_STATIC_BZIP2=ON `
  -DDEVILUTIONX_SYSTEM_BZIP2=OFF
```

---

## 🎯 DESGLOSE DE OPCIONES CRÍTICAS (POR QUÉ CADA UNA ES VITAL)

1. **`-DDEVILUTIONX_STATIC_BZIP2=ON` & `-DDEVILUTIONX_SYSTEM_BZIP2=OFF`**
   - **Función**: Compila la librería `bzip2` de forma estática dentro del ejecutable.
   - **Por qué es obligatoria**: DevilutionX necesita `bzip2` para descompresión interna de los bloques en los archivos MPQ (`DIABDAT.MPQ`, `hellfire.mpq`). Si está en `OFF`, intentará usar DLLs dinámicas externas que provocan fallos de arranque o "DIABDAT.MPQ no reconocido".

2. **`-DNOSOUND=OFF`**
   - **Función**: Compila y enlaza `SDL_audiolib`.
   - **Por qué es obligatoria**: Permite que los efectos de sonido, música ambiental y voces de los NPCs funcionen a la perfección.

3. **`-DUNPACKED_MPQS=OFF` & `-DUNPACKED_SAVES=OFF`**
   - **Función**: Activa el cargador estándar de archivos empaquetados `.MPQ`.
   - **Por qué es obligatoria**: Si se pone en `ON`, el motor ignora los archivos `.MPQ` y busca assets sueltos en disco, fallando al iniciar.

4. **`-Wl,--disable-dynamicbase -Wl,--disable-high-entropy-va`** (En `CMake/platforms/windows.cmake`)
   - **Función**: Desactiva la aleatorización agresiva de memoria de ASLR en 64 bits para MinGW.
   - **Por qué es obligatoria**: Previene el error `Mingw-w64 runtime failure: 32 bit pseudo relocation out of range` en compiladores modernos GCC (14/15/16).

---

## 🔨 COMANDO DE COMPILACIÓN

```powershell
cmake --build build_COMPILE_FRESH -j 4
```

---

## 📁 ESTRUCTURA Y ARCHIVOS REQUERIDOS PARA JUGAR

En la carpeta de salida (`build_COMPILE_FRESH/`):

1. **`devilutionx.exe`**: Binario generado por la compilación.
2. **`DIABDAT.MPQ`**: Archivo de datos original del juego (517 MB).
3. **Carpeta `assets/`**: Generada automáticamente con fuentes, UI y scripts Lua.
4. **DLLs de Runtime de MinGW/SDL2**:
   - `SDL2.dll`
   - `SDL2_image.dll`
   - `libzlib.dll` (o `zlib1.dll`)
   - `libstdc++-6.dll`
   - `libgcc_s_seh-1.dll`
   - `libwinpthread-1.dll`

---

## ⚡ RECOMPILACIÓN RÁPIDA

Cuando hagas cambios en código C++ (`Source/`):
```powershell
cmake --build build_COMPILE_FRESH -j 4
```
*No es necesario volver a ejecutar el comando `cmake -S ...` a menos que agregues o quites archivos `.cpp` o dependencias de CMake.*
