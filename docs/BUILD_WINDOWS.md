# Guía de Compilación Local en Windows (DevilutionX)

Esta guía documenta todos los requisitos, herramientas y comandos necesarios para recrear desde cero el entorno de compilación de **DevilutionX** en Windows y compilar el binario nativo `devilutionx.exe`.

---

## 1. Requisitos y Herramientas

Para compilar en Windows de forma nativa con **MinGW-w64** y **CMake / Ninja**, se requieren las siguientes herramientas:

1. **CMake** (Generador de sistemas de construcción): `>= 3.13`
2. **Ninja** (Sistema de construcción rápido en paralelo): Opcional pero altamente recomendado.
3. **MinGW-w64 (WinLibs / GCC + UCRT / POSIX threads)**: Compilador `gcc`, `g++`, `c++`.
4. **Git**: Control de versiones.

---

## 2. Instalación Rápida con `winget`

Abre una terminal de PowerShell como administrador o usuario estándar y ejecuta:

```powershell
# 1. Instalar CMake
winget install --id Kitware.CMake -e --accept-source-agreements --accept-package-agreements --silent

# 2. Instalar Ninja
winget install --id Ninja-build.Ninja -e --accept-source-agreements --accept-package-agreements --silent

# 3. Instalar MinGW-w64 (WinLibs POSIX UCRT)
winget install --id BrechtSanders.WinLibs.POSIX.UCRT -e --accept-source-agreements --accept-package-agreements --silent
```

> **Nota:** Tras la instalación, reinicia la terminal o actualiza la variable `PATH` de tu sesión para que los comandos `cmake`, `ninja`, y `g++` estén disponibles.

---

## 3. Configurar y Compilar

### Opción A: Usando Ninja (Más Rápido)

```powershell
# 1. Posicionarse en la raíz del repositorio
cd C:\Projects\DevilutionX

# 2. Configurar el proyecto con CMake y Ninja
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# 3. Compilar el ejecutable
cmake --build build -j
```

### Opción B: Usando MinGW Makefiles

```powershell
# 1. Configurar
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 2. Compilar
cmake --build build -j
```

---

## 4. Estructura de Salida y Ejecución

Una vez finalizada la compilación:
- El ejecutable generado estará en: `build/devilutionx.exe` (o `build_COMPILE_FRESH/devilutionx.exe`).
- Para ejecutar el juego, asegúrate de tener el archivo de datos original `DIABDAT.MPQ` en la misma carpeta o en el directorio configurado de DevilutionX (o `devilutionx.mpq`).

---

## 5. Regla del Proyecto para Modificaciones

Cada vez que se modifique código fuente en `Source/` o archivos de configuración:
1. Recompilar inmediatamente ejecutando:
   ```powershell
   cmake --build build -j
   ```
2. Registrar los cambios en [`docs/CHANGELOG.md`](file:///c:/Projects/DevilutionX/docs/CHANGELOG.md).
