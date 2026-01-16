# 🚫 DEPENDENCIAS CRÍTICAS - NUNCA BORRAR
**Fecha**: Enero 16, 2026  
**Branch**: `compilacionExitosa`  
**Importancia**: CRÍTICA

---

## ⚠️ ADVERTENCIA MÁXIMA

**ESTAS CARPETAS Y ARCHIVOS SON VITALES PARA COMPILAR**

Si borras cualquiera de estos, la compilación fallará y será difícil/imposible recuperar sin backup externo.

---

## 📁 ESTRUCTURA CRÍTICA

```
DevilutionX/
├── deps/                           ← NUNCA BORRAR ESTA CARPETA
│   ├── SDL2-dev/                   ← CRÍTICO: SDL2 precompilado
│   │   └── SDL2-2.32.10/
│   │       └── x86_64-w64-mingw32/
│   │           ├── bin/
│   │           │   └── SDL2.dll    ← Necesario para ejecutar
│   │           ├── include/
│   │           │   └── SDL2/       ← Headers de SDL2
│   │           └── lib/
│   │               ├── cmake/
│   │               │   └── SDL2/   ← CMake config
│   │               └── *.a         ← Librerías estáticas
│   │
│   ├── zlib-src/                   ← CRÍTICO: Source de zlib
│   │   └── zlib-1.3.1/
│   │       ├── zlib.h
│   │       ├── zconf.h
│   │       └── ...
│   │
│   └── zlib-build/                 ← MUY CRÍTICO: zlib compilado
│       ├── libzlibstatic.a         ← Difícil de regenerar
│       └── zconf.h
│
├── build_SAFE/                     ← MANTENER: Backup funcional
│   └── devilutionx.exe             ← Ejecutable de respaldo
│
├── build_SAFE2/                    ← MANTENER: Backup adicional
│   └── devilutionx.exe
│
├── build_COMPILE_FRESH/            ← MANTENER: Build actual
│   ├── CMakeCache.txt
│   ├── devilutionx.exe
│   └── ...
│
├── DIABDAT.MPQ                     ← CRÍTICO: Datos del juego
│
└── nightmare.exe                   ← Ejecutable principal
```

---

## 🔴 ARCHIVOS CRÍTICOS INDIVIDUALES

### 1. SDL2 (Precompilado)
```
deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2/SDL2Config.cmake
```
**Por qué es crítico**: CMake lo busca para configurar SDL2  
**Si falta**: Compilación falla con "SDL2 not found"  
**Cómo recuperar**: Descargar `SDL2-devel-2.32.10-mingw.tar.gz` y extraer

### 2. zlib Compilado
```
deps/zlib-build/libzlibstatic.a
```
**Por qué es crítico**: Librería compilada de zlib, difícil de regenerar  
**Si falta**: Compilación falla con "zlib not found"  
**Cómo recuperar**: 
- Opción A: Copiar desde backup
- Opción B: Recompilar (ver sección "Recompilar zlib")

### 3. zlib Headers
```
deps/zlib-src/zlib-1.3.1/zlib.h
deps/zlib-src/zlib-1.3.1/zconf.h
```
**Por qué es crítico**: Headers necesarios para compilar  
**Si falta**: Compilación falla con "zlib.h: No such file or directory"  
**Cómo recuperar**: Descargar zlib-1.3.1 desde https://zlib.net/

### 4. DIABDAT.MPQ
```
DIABDAT.MPQ
```
**Por qué es crítico**: Datos del juego original (gráficos, audio, etc.)  
**Si falta**: Juego no inicia  
**Cómo recuperar**: Copiar desde CD de Diablo 1 o backup

### 5. Build Directories
```
build_SAFE/
build_SAFE2/
build_COMPILE_FRESH/
```
**Por qué son críticos**: Contienen builds funcionales y configuración  
**Si faltan**: Hay que reconfigurar CMake desde cero  
**Cómo recuperar**: Mantener siempre al menos uno como backup

---

## 🛡️ ESTRATEGIA DE BACKUP

### Backup Mínimo (Esencial)
```
deps/                    ← Toda la carpeta
build_SAFE/              ← Al menos un build funcional
DIABDAT.MPQ              ← Datos del juego
nightmare.exe            ← Ejecutable funcional
```

**Tamaño total**: ~1.5 GB

### Backup Completo (Recomendado)
```
deps/                    ← Toda la carpeta
build_SAFE/              ← Build funcional 1
build_SAFE2/             ← Build funcional 2
build_COMPILE_FRESH/     ← Build actual
DIABDAT.MPQ              ← Datos del juego
nightmare.exe            ← Ejecutable principal
Source/                  ← Código fuente
CMakeLists.txt           ← Configuración principal
*.bat                    ← Scripts de compilación
*.md                     ← Documentación
```

**Tamaño total**: ~3-4 GB

---

## 🔧 CÓMO RECOMPILAR ZLIB (Si se pierde)

### Paso 1: Descargar zlib
```bash
# Descargar zlib-1.3.1 desde https://zlib.net/
# Extraer en deps/zlib-src/zlib-1.3.1/
```

### Paso 2: Crear build directory
```bash
mkdir deps\zlib-build
cd deps\zlib-build
```

### Paso 3: Configurar CMake
```bash
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..\zlib-src\zlib-1.3.1
```

### Paso 4: Compilar
```bash
cmake --build . --target zlibstatic
```

### Paso 5: Verificar
```bash
dir libzlibstatic.a
# Debe existir y ser ~100-200 KB
```

---

## 🔍 VERIFICACIÓN DE INTEGRIDAD

### Script de Verificación
```bash
@echo off
echo Verificando dependencias criticas...
echo.

if exist "deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\lib\cmake\SDL2" (
    echo [OK] SDL2 encontrado
) else (
    echo [ERROR] SDL2 NO encontrado
)

if exist "deps\zlib-build\libzlibstatic.a" (
    echo [OK] zlib compilado encontrado
) else (
    echo [ERROR] zlib compilado NO encontrado
)

if exist "deps\zlib-src\zlib-1.3.1" (
    echo [OK] zlib source encontrado
) else (
    echo [ERROR] zlib source NO encontrado
)

if exist "DIABDAT.MPQ" (
    echo [OK] DIABDAT.MPQ encontrado
) else (
    echo [ERROR] DIABDAT.MPQ NO encontrado
)

if exist "build_SAFE" (
    echo [OK] build_SAFE encontrado
) else (
    echo [WARN] build_SAFE NO encontrado (recomendado)
)

echo.
pause
```

Guardar como `VERIFICAR_DEPENDENCIAS.bat`

---

## 📋 CHECKLIST DE SEGURIDAD

Antes de hacer cualquier operación de limpieza:

- [ ] Verificar que `deps/` existe y está completo
- [ ] Verificar que `deps/zlib-build/libzlibstatic.a` existe
- [ ] Verificar que al menos un `build_SAFE*` existe
- [ ] Hacer backup externo de `deps/` (opcional pero recomendado)
- [ ] Verificar que `DIABDAT.MPQ` existe
- [ ] Verificar que `nightmare.exe` funciona

---

## 🚫 COMANDOS PELIGROSOS

### NUNCA ejecutar estos comandos:
```bash
# ❌ PELIGRO: Borra TODO incluyendo dependencias
git clean -fdx

# ❌ PELIGRO: Borra deps/
rmdir /s /q deps

# ❌ PELIGRO: Borra todos los builds
for /d %i in (build_*) do rmdir /s /q %i

# ❌ PELIGRO: Borra zlib compilado
del deps\zlib-build\libzlibstatic.a
```

### Comandos seguros:
```bash
# ✅ SEGURO: Borra solo un build específico
rmdir /s /q build_TEST

# ✅ SEGURO: Limpia build actual (pero mantiene configuración)
cmake --build build_COMPILE_FRESH --target clean

# ✅ SEGURO: Crea nuevo build sin tocar existentes
mkdir build_NEW
```

---

## 🆘 RECUPERACIÓN DE EMERGENCIA

### Si borraste `deps/` por accidente:

1. **Restaurar desde backup externo** (si existe)
2. **O recompilar desde cero**:
   ```bash
   # Descargar SDL2-devel-2.32.10-mingw.tar.gz
   # Extraer en deps/SDL2-dev/
   
   # Descargar zlib-1.3.1
   # Extraer en deps/zlib-src/
   
   # Recompilar zlib (ver sección anterior)
   ```

### Si borraste todos los `build_*`:

1. **Crear nuevo build desde cero**:
   ```bash
   mkdir build_COMPILE_FRESH
   COMPILE_NIGHTMARE_DEFINITIVO.bat
   ```

### Si borraste `DIABDAT.MPQ`:

1. **Copiar desde CD de Diablo 1**
2. **O copiar desde backup**
3. **O copiar desde otra instalación**

---

## 📚 DOCUMENTACIÓN RELACIONADA

- `COMPILACION_DEFINITIVA_GUIA_ETERNA.md` - Guía completa de compilación
- `COMPILE_NIGHTMARE_DEFINITIVO.bat` - Script de compilación
- `BUILD_PROCESS_DOCUMENTATION.md` - Proceso detallado

---

## ✅ RESUMEN EJECUTIVO

### Lo que NUNCA debes borrar:
1. ❌ `deps/` - Dependencias compiladas
2. ❌ `deps/zlib-build/libzlibstatic.a` - zlib compilado
3. ❌ `deps/SDL2-dev/` - SDL2 precompilado
4. ❌ Todos los `build_*` a la vez - Mantener al menos uno
5. ❌ `DIABDAT.MPQ` - Datos del juego

### Lo que puedes borrar con seguridad:
1. ✅ `build_TEST/` - Builds de prueba específicos
2. ✅ `*.exe.backup` - Backups de ejecutables
3. ✅ Archivos `.md` de documentación (pero no recomendado)
4. ✅ Logs y archivos temporales

### Regla de oro:
> **"Cuando tengas dudas, NO borres. Crea nuevo en lugar de limpiar existente."**

---

**Creado por**: Kiro AI  
**Fecha**: Enero 16, 2026  
**Branch**: `compilacionExitosa`  
**Versión**: 1.0

**¡MANTENER ESTE ARCHIVO SIEMPRE VISIBLE!** 🚨
