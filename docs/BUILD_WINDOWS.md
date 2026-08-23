# 🏆 Guía Maestra de Compilación Funcional en Windows (DevilutionX)

Esta guía documenta la configuración **100% probada y funcional** para compilar y ejecutar **DevilutionX** (incluyendo Nightmare Edition / mods) en Windows utilizando **MinGW-w64** y **CMake / Ninja**.

---

## 1. Requisitos y Herramientas

1. **CMake** (`>= 3.22`): Generador de proyectos.
2. **Ninja**: Motor de compilación rápido en paralelo (o `MinGW Makefiles`).
3. **MinGW-w64** (x86_64 UCRT / POSIX): Compilador `gcc`, `g++`, `c++`.
4. **Git**: Control de versiones.

---

## 2. Configuración Exacta de CMake

Para garantizar que el juego abra correctamente, lea los archivos `.MPQ` y no sufra de fallos de reubicación de memoria en Windows 64-bit, se deben pasar los siguientes parámetros:

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

### 🎯 Explicación de los Parámetros Críticos:

| Parámetro | Valor | Motivo |
| :--- | :--- | :--- |
| `-DDEVILUTIONX_STATIC_BZIP2=ON` | `ON` | **CRÍTICO**: Embebe `bzip2` estáticamente en el binario para descomprimir y leer `DIABDAT.MPQ` sin errores de DLL. |
| `-DDEVILUTIONX_SYSTEM_BZIP2=OFF`| `OFF` | Evita enlazar versiones externas incompatibles de bzip2 del sistema. |
| `-DNOSOUND=OFF` | `OFF` | Habilita el subsistema de audio nativo mediante `SDL_audiolib`. |
| `-DUNPACKED_MPQS=OFF` | `OFF` | Obliga al juego a cargar los datos desde los archivos MPQ empaquetados (`DIABDAT.MPQ`, `hellfire.mpq`, etc.). |
| `-DNONET=ON` | `ON` | Deshabilita dependencias de red externas complejas para builds locales rápidos y estables. |
| `-DBUILD_TESTING=OFF` | `OFF` | Acelera la compilación al no generar binarios de prueba innecesarios. |

---

## 3. Enlazado en MinGW (Prevención de Pseudo-Relocation Errors)

En toolchains modernos de 64-bit (GCC 14+ / 16+), Windows ASLR puede cargar librerías fuera del rango de 32-bit (±2GB), provocando el error:
> `Mingw-w64 runtime failure: 32 bit pseudo relocation ... out of range`

Para prevenir esto permanentemente, [`CMake/platforms/windows.cmake`](file:///c:/Projects/DevilutionX/CMake/platforms/windows.cmake) incluye:
```cmake
list(APPEND DEVILUTIONX_PLATFORM_LINK_LIBRARIES "-Wl,--disable-dynamicbase" "-Wl,--disable-high-entropy-va")
```

---

## 4. Compilación del Ejecutable

Una vez configurado CMake, compila con:

```powershell
cmake --build build_COMPILE_FRESH -j 4
```

El ejecutable resultante se generará en:
`build_COMPILE_FRESH/devilutionx.exe`

---

## 5. Archivos Necesarios en la Carpeta de Ejecución

Para que el juego inicie correctamente al hacer doble clic o ejecutarlo desde la terminal, la carpeta `build_COMPILE_FRESH/` debe contener:

1. **`devilutionx.exe`** (El binario compilado)
2. **`DIABDAT.MPQ`** (517 MB, datos del juego original)
3. **Carpeta `assets/`** (Generada automáticamente por el build)
4. **DLLs de Runtime**:
   - `SDL2.dll`
   - `SDL2_image.dll`
   - `libzlib.dll` (o `zlib1.dll`)
   - `libstdc++-6.dll`
   - `libgcc_s_seh-1.dll`
   - `libwinpthread-1.dll`

---

## 6. Recompilación Rápida tras Cambios de Código

Si editas archivos en `Source/`:
```powershell
# Solo requiere ejecutar el build, no es necesario reconfigurar CMake
cmake --build build_COMPILE_FRESH -j 4
```
