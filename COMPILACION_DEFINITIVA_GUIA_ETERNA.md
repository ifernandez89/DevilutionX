# 🔨 GUÍA DEFINITIVA DE COMPILACIÓN - NIGHTMARE EDITION
**Fecha**: Enero 16, 2026  
**Branch**: `compilacionExitosa`  
**Status**: ✅ MÉTODO PROBADO Y FUNCIONAL

---

## ⚠️ REGLAS DE ORO - NUNCA OLVIDAR

### 🚫 NUNCA HACER ESTO:
1. ❌ **NUNCA borrar carpetas `deps/`** - Contiene dependencias críticas compiladas
2. ❌ **NUNCA borrar todas las carpetas `build_*`** - Siempre mantener al menos una funcional
3. ❌ **NUNCA borrar `deps/zlib-build/`** - zlib compilado, difícil de regenerar
4. ❌ **NUNCA borrar `deps/SDL2-dev/`** - SDL2 precompilado
5. ❌ **NUNCA hacer `git clean -fdx`** sin backup - Borra TODO incluyendo dependencias

### ✅ SIEMPRE HACER ESTO:
1. ✅ **Mantener carpeta `build_SAFE` o `build_SAFE2`** como backup funcional
2. ✅ **Crear nueva carpeta build** en lugar de limpiar existente
3. ✅ **Verificar que `deps/` existe** antes de compilar
4. ✅ **Usar scripts de compilación** en lugar de comandos manuales
5. ✅ **Hacer backup del ejecutable** antes de recompilar

---

## 📁 ESTRUCTURA CRÍTICA DEL PROYECTO

### Dependencias Vitales (NO BORRAR)
```
deps/
├── SDL2-dev/
│   └── SDL2-2.32.10/
│       └── x86_64-w64-mingw32/
│           ├── bin/
│           ├── include/
│           └── lib/
│               └── cmake/SDL2/
├── zlib-src/
│   └── zlib-1.3.1/
└── zlib-build/
    └── libzlibstatic.a  ← CRÍTICO: No regenerar fácilmente
```

### Build Directories (Mantener al menos uno)
```
build_COMPILE_FRESH/  ← Último build exitoso
build_SAFE/           ← Backup funcional
build_SAFE2/          ← Backup adicional
```

### Scripts de Compilación
```
COMPILE_DEVILUTIONX_DEFINITIVO.bat  ← Script principal
QUICK_START_TESTING.bat             ← Testing rápido
compile_test_oracle.bat             ← Testing Oracle
```

### Ejecutables
```
nightmare.exe         ← Ejecutable principal
devilutionx.exe       ← Alias (opcional)
```

---

## 🔧 MÉTODO DE COMPILACIÓN DEFINITIVO

### Opción A: Script Automático (RECOMENDADO)
```bash
# Usar el script definitivo
COMPILE_DEVILUTIONX_DEFINITIVO.bat
```

### Opción B: Comandos Manuales (Si script falla)

#### Paso 1: Verificar Dependencias
```bash
# Verificar que deps/ existe
dir deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\lib\cmake\SDL2
dir deps\zlib-build\libzlibstatic.a
dir deps\zlib-src\zlib-1.3.1
```

**Si falta algo**: ¡DETENER! Restaurar desde backup o rama funcional.

#### Paso 2: Crear Build Directory Limpio
```bash
# NO borrar build existente, crear uno nuevo
mkdir build_NEW
cd build_NEW
```

#### Paso 3: Configurar CMake
```bash
cmake -S.. -B. -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DNONET=ON ^
  -DNOSOUND=OFF ^
  -DBUILD_TESTING=OFF ^
  -DDISABLE_ZERO_TIER=ON ^
  -DUNPACKED_MPQS=OFF ^
  -DUNPACKED_SAVES=OFF ^
  -DDEVILUTIONX_STATIC_BZIP2=ON ^
  -DDEVILUTIONX_SYSTEM_BZIP2=OFF ^
  -DSDL2_DIR="C:/c - c++/DevilutionX/deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2" ^
  -DZLIB_LIBRARY="C:/c - c++/DevilutionX/deps/zlib-build/libzlibstatic.a" ^
  -DZLIB_INCLUDE_DIR="C:/c - c++/DevilutionX/deps/zlib-src/zlib-1.3.1" ^
  -DCMAKE_PREFIX_PATH="C:/c - c++/DevilutionX/deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32"
```

**IMPORTANTE**: Ajustar rutas si tu proyecto está en otra ubicación.

#### Paso 4: Compilar
```bash
cmake --build . -j 4
```

**Tiempo esperado**: 5-10 minutos

#### Paso 5: Copiar Ejecutable
```bash
cd ..
copy build_NEW\devilutionx.exe nightmare.exe
```

---

## 🛠️ CONFIGURACIÓN CMAKE EXPLICADA

### Flags Críticos
```cmake
-DCMAKE_BUILD_TYPE=Release
# Compilación optimizada para producción

-DNONET=ON
# Desactiva networking (no necesario para single player)

-DNOSOUND=OFF
# Mantiene audio habilitado

-DBUILD_TESTING=OFF
# No compila tests (más rápido)

-DDISABLE_ZERO_TIER=ON
# Desactiva ZeroTier networking

-DDEVILUTIONX_STATIC_BZIP2=ON
# Usa bzip2 estático (no requiere DLL externa)

-DDEVILUTIONX_SYSTEM_BZIP2=OFF
# No usa bzip2 del sistema
```

### Rutas de Dependencias
```cmake
-DSDL2_DIR="..."
# Ruta a SDL2Config.cmake

-DZLIB_LIBRARY="..."
# Ruta a libzlibstatic.a (archivo compilado)

-DZLIB_INCLUDE_DIR="..."
# Ruta a headers de zlib

-DCMAKE_PREFIX_PATH="..."
# Ruta base de SDL2 (para encontrar DLLs)
```

---

## 🔍 DIAGNÓSTICO DE PROBLEMAS

### Problema 1: "SDL2 not found"
```bash
# Verificar que existe:
dir deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\lib\cmake\SDL2\SDL2Config.cmake
```

**Solución**: Si no existe, descargar SDL2-devel-2.32.10-mingw.tar.gz y extraer en `deps/SDL2-dev/`

### Problema 2: "zlib not found"
```bash
# Verificar que existe:
dir deps\zlib-build\libzlibstatic.a
```

**Solución**: Si no existe, copiar desde `build_SAFE/` o recompilar zlib:
```bash
cd deps\zlib-src\zlib-1.3.1
mkdir ..\..\zlib-build
cd ..\..\zlib-build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..\zlib-src\zlib-1.3.1
cmake --build . --target zlibstatic
```

### Problema 3: "Linking failed - undefined reference"
```bash
# Verificar que CMakeLists.txt tiene:
target_link_libraries(libdevilutionx PUBLIC winhttp)
```

**Solución**: Añadir `winhttp` a `Source/CMakeLists.txt` si falta.

### Problema 4: "Cannot open dependency file"
```bash
# Build directory corrupto
```

**Solución**: Crear nuevo build directory limpio (NO limpiar el existente).

### Problema 5: "Compilation takes forever"
```bash
# Demasiados jobs paralelos o disco lento
```

**Solución**: Reducir jobs: `cmake --build . -j 2` en lugar de `-j 4`

---

## 📦 DEPENDENCIAS EXTERNAS

### MinGW-w64 GCC 14.2.0
**Ubicación**: `C:\mingw64\`  
**Verificar**:
```bash
gcc --version
# Debe mostrar: gcc (GCC) 14.2.0
```

**Si no está instalado**: Descargar desde https://www.mingw-w64.org/

### CMake 4.2.1+
**Verificar**:
```bash
cmake --version
# Debe mostrar: cmake version 4.2.1 o superior
```

**Si no está instalado**: Descargar desde https://cmake.org/download/

### SDL2 2.32.10
**Ubicación**: `deps/SDL2-dev/SDL2-2.32.10/`  
**Archivo**: `SDL2-devel-2.32.10-mingw.tar.gz`

**Si falta**: Descargar desde https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10

### zlib 1.3.1
**Ubicación**: `deps/zlib-src/zlib-1.3.1/`  
**Compilado**: `deps/zlib-build/libzlibstatic.a`

**Si falta**: Descargar desde https://zlib.net/

---

## 🎯 CHECKLIST PRE-COMPILACIÓN

Antes de compilar, verificar:

- [ ] `deps/SDL2-dev/` existe y contiene SDL2 2.32.10
- [ ] `deps/zlib-build/libzlibstatic.a` existe
- [ ] `deps/zlib-src/zlib-1.3.1/` existe
- [ ] MinGW-w64 GCC está en PATH
- [ ] CMake está en PATH
- [ ] Al menos una carpeta `build_SAFE*` existe como backup
- [ ] Espacio en disco suficiente (>2GB libre)

---

## 🚀 COMPILACIÓN RÁPIDA (Ya configurado)

Si ya tienes un build directory configurado:

```bash
# Opción 1: Recompilar en build existente
cd build_COMPILE_FRESH
cmake --build . -j 4
cd ..
copy build_COMPILE_FRESH\devilutionx.exe nightmare.exe
```

```bash
# Opción 2: Usar script
COMPILE_DEVILUTIONX_DEFINITIVO.bat
```

---

## 📝 NOTAS IMPORTANTES

### Tiempo de Compilación
- **Primera vez**: 10-15 minutos (compila todo)
- **Incremental**: 1-3 minutos (solo cambios)
- **Limpia**: 5-10 minutos (build directory limpio)

### Espacio en Disco
- **deps/**: ~200 MB
- **build_*/**: ~500 MB por carpeta
- **Total recomendado**: 2-3 GB libres

### Warnings Normales (Ignorar)
```
warning: visibility attribute not supported
warning: argument 1 value exceeds maximum object size
```

Estos warnings son normales y no afectan la compilación.

### Errores Críticos (NO ignorar)
```
fatal error: SDL2/SDL.h: No such file or directory
fatal error: zlib.h: No such file or directory
undefined reference to `WinHttpOpen'
```

Estos errores indican dependencias faltantes o mal configuradas.

---

## 🔄 WORKFLOW RECOMENDADO

### Para Desarrollo Normal
```bash
# 1. Hacer cambios en código
# 2. Compilar incrementalmente
cd build_COMPILE_FRESH
cmake --build . -j 4

# 3. Copiar ejecutable
cd ..
copy build_COMPILE_FRESH\devilutionx.exe nightmare.exe

# 4. Testear
nightmare.exe
```

### Para Cambios Grandes (Nueva Feature)
```bash
# 1. Crear rama
git checkout -b nueva-feature

# 2. Hacer cambios
# ...

# 3. Crear build limpio
mkdir build_nueva_feature
cd build_nueva_feature
cmake -S.. -B. -G "MinGW Makefiles" [... flags ...]
cmake --build . -j 4

# 4. Testear
cd ..
copy build_nueva_feature\devilutionx.exe nightmare.exe
nightmare.exe

# 5. Si funciona, mergear
git checkout master
git merge nueva-feature
```

### Para Rollback Seguro
```bash
# Opción 1: Usar build anterior
copy build_SAFE\devilutionx.exe nightmare.exe

# Opción 2: Recompilar rama anterior
git checkout master
cmake --build build_COMPILE_FRESH -j 4
copy build_COMPILE_FRESH\devilutionx.exe nightmare.exe
```

---

## 🎮 POST-COMPILACIÓN

### Verificar Ejecutable
```bash
# Verificar que existe
dir nightmare.exe

# Verificar tamaño (debe ser ~15-20 MB)
# Si es <1 MB, algo salió mal
```

### Testing Básico
```bash
# Iniciar juego
nightmare.exe

# Verificar:
# - Menú principal carga
# - Pueblo carga
# - NPCs se mueven (si feature habilitada)
# - Oracle responde (si Ollama activo)
```

### Logs de Debug
```bash
# Buscar en consola o archivo de log:
# - "NPC Micro-Movement: Enabled for..."
# - "Oracle System: Initialized"
# - Errores o warnings críticos
```

---

## 📚 ARCHIVOS DE REFERENCIA

### Scripts
- `COMPILE_DEVILUTIONX_DEFINITIVO.bat` - Script principal
- `QUICK_START_TESTING.bat` - Testing rápido
- `compile_test_oracle.bat` - Testing Oracle

### Documentación
- `METODO_COMPILACION_DEFINITIVO_FINAL.md` - Método detallado
- `BUILD_PROCESS_DOCUMENTATION.md` - Proceso completo
- `COMPILACION_EXITOSA_METODO.md` - Método exitoso

### Configuración
- `CMakeLists.txt` - Configuración principal
- `Source/CMakeLists.txt` - Configuración de Source
- `vcpkg.json` - Dependencias (no usado actualmente)

---

## 🆘 AYUDA RÁPIDA

### Compilación Falla
1. Verificar `deps/` existe
2. Crear build directory limpio
3. Verificar rutas en comando cmake
4. Verificar GCC y CMake en PATH

### Ejecutable No Funciona
1. Verificar que SDL2.dll está en PATH o mismo directorio
2. Verificar que DIABDAT.MPQ existe
3. Verificar logs de error

### Build Directory Corrupto
1. NO intentar limpiar
2. Crear nuevo build directory
3. Reconfigurar con cmake
4. Compilar desde cero

### Dependencias Perdidas
1. Restaurar desde `build_SAFE/`
2. O descargar desde fuentes oficiales
3. O copiar desde otra máquina/backup

---

## ✅ VERIFICACIÓN FINAL

Antes de considerar compilación exitosa:

- [ ] `nightmare.exe` existe y es >10 MB
- [ ] Juego inicia sin errores
- [ ] Menú principal funciona
- [ ] Pueblo carga correctamente
- [ ] NPCs visibles y funcionales
- [ ] Audio funciona (si habilitado)
- [ ] No hay crashes inmediatos

---

**Creado por**: Kiro AI  
**Fecha**: Enero 16, 2026  
**Branch**: `compilacionExitosa`  
**Versión**: 1.0 - Método Definitivo

**¡NUNCA BORRAR DEPENDENCIAS CRÍTICAS!** 🚫
