# 📚 ÍNDICE MAESTRO - COMPILACIÓN NIGHTMARE EDITION
**Fecha**: Enero 16, 2026  
**Branch**: `compilacionExitosa`  
**Propósito**: Referencia rápida a toda la documentación de compilación

---

## 🎯 INICIO RÁPIDO

### ¿Primera vez compilando?
1. Lee: `COMPILACION_DEFINITIVA_GUIA_ETERNA.md`
2. Ejecuta: `VERIFICAR_DEPENDENCIAS.bat`
3. Compila: `COMPILE_NIGHTMARE_DEFINITIVO.bat`

### ¿Ya compilaste antes?
1. Ejecuta: `COMPILE_NIGHTMARE_DEFINITIVO.bat`
2. Si falla: Lee `DEPENDENCIAS_CRITICAS_NO_BORRAR.md`

### ¿Problemas de compilación?
1. Lee: `DIAGNOSTICO_PROBLEMAS_COMPILACION.md` (si existe)
2. Ejecuta: `VERIFICAR_DEPENDENCIAS.bat`
3. Revisa: Sección "Diagnóstico de Problemas" en guía principal

---

## 📖 DOCUMENTACIÓN PRINCIPAL

### 1. Guía de Compilación
**Archivo**: `COMPILACION_DEFINITIVA_GUIA_ETERNA.md`  
**Propósito**: Guía completa paso a paso para compilar  
**Cuándo leer**: Primera vez o cuando hay problemas

**Contenido**:
- Reglas de oro (qué nunca hacer)
- Estructura del proyecto
- Método de compilación definitivo
- Configuración CMake explicada
- Diagnóstico de problemas
- Workflow recomendado

### 2. Dependencias Críticas
**Archivo**: `DEPENDENCIAS_CRITICAS_NO_BORRAR.md`  
**Propósito**: Lista de archivos/carpetas que NUNCA se deben borrar  
**Cuándo leer**: Antes de hacer limpieza o si borraste algo por accidente

**Contenido**:
- Estructura crítica del proyecto
- Archivos críticos individuales
- Estrategia de backup
- Cómo recompilar zlib
- Recuperación de emergencia
- Comandos peligrosos vs seguros

### 3. Índice Maestro (este archivo)
**Archivo**: `INDICE_MAESTRO_COMPILACION.md`  
**Propósito**: Referencia rápida a toda la documentación  
**Cuándo leer**: Para encontrar información específica

---

## 🔧 SCRIPTS DE COMPILACIÓN

### 1. Script Principal
**Archivo**: `COMPILE_NIGHTMARE_DEFINITIVO.bat`  
**Propósito**: Compilar el proyecto completo  
**Cuándo usar**: Siempre que quieras compilar

**Características**:
- Verifica dependencias automáticamente
- Detecta si es build limpio o incremental
- Hace backup del ejecutable anterior
- Maneja errores con mensajes claros

**Uso**:
```bash
COMPILE_NIGHTMARE_DEFINITIVO.bat
```

### 2. Script de Verificación
**Archivo**: `VERIFICAR_DEPENDENCIAS.bat`  
**Propósito**: Verificar que todas las dependencias están presentes  
**Cuándo usar**: Antes de compilar o cuando hay errores

**Características**:
- Verifica SDL2, zlib, DIABDAT.MPQ
- Verifica herramientas (GCC, CMake)
- Verifica espacio en disco
- Reporta errores y advertencias

**Uso**:
```bash
VERIFICAR_DEPENDENCIAS.bat
```

### 3. Scripts Adicionales (Existentes)
**Archivos**:
- `COMPILE_DEVILUTIONX_DEFINITIVO.bat` - Script anterior (funcional)
- `QUICK_START_TESTING.bat` - Testing rápido
- `compile_test_oracle.bat` - Testing del sistema Oracle
- `TEST_OLLAMA_STATUS.bat` - Verificar Ollama

---

## 📁 ESTRUCTURA DEL PROYECTO

### Carpetas Críticas (NO BORRAR)
```
deps/                    - Dependencias compiladas
├── SDL2-dev/           - SDL2 precompilado
├── zlib-src/           - Source de zlib
└── zlib-build/         - zlib compilado (CRÍTICO)

build_SAFE/             - Build funcional de backup
build_SAFE2/            - Build funcional adicional
build_COMPILE_FRESH/    - Build actual
```

### Carpetas de Código
```
Source/                 - Código fuente C++
├── oracle/            - Sistema Oráculo
├── controls/          - Controles
├── engine/            - Motor gráfico
└── ...

CMake/                  - Scripts CMake
Translations/           - Traducciones
assets/                 - Assets del juego
```

### Archivos de Configuración
```
CMakeLists.txt          - Configuración principal CMake
Source/CMakeLists.txt   - Configuración de Source
vcpkg.json              - Dependencias vcpkg (no usado)
```

### Ejecutables
```
nightmare.exe           - Ejecutable principal
DIABDAT.MPQ            - Datos del juego (CRÍTICO)
```

---

## 🔍 DIAGNÓSTICO RÁPIDO

### Problema: "SDL2 not found"
**Solución**: Ver `DEPENDENCIAS_CRITICAS_NO_BORRAR.md` sección "SDL2"  
**Script**: `VERIFICAR_DEPENDENCIAS.bat`

### Problema: "zlib not found"
**Solución**: Ver `DEPENDENCIAS_CRITICAS_NO_BORRAR.md` sección "zlib"  
**Script**: `VERIFICAR_DEPENDENCIAS.bat`

### Problema: "Linking failed"
**Solución**: Ver `COMPILACION_DEFINITIVA_GUIA_ETERNA.md` sección "Diagnóstico"  
**Causa común**: Falta `winhttp` en CMakeLists.txt

### Problema: "Build directory corrupto"
**Solución**: Crear nuevo build directory limpio  
**Script**: `COMPILE_NIGHTMARE_DEFINITIVO.bat` (lo hace automáticamente)

### Problema: "Compilación muy lenta"
**Solución**: Reducir jobs paralelos (`-j 2` en lugar de `-j 4`)  
**Causa**: Disco lento o poca RAM

---

## 📊 DOCUMENTACIÓN POR TEMA

### Compilación
- `COMPILACION_DEFINITIVA_GUIA_ETERNA.md` - Guía completa
- `METODO_COMPILACION_DEFINITIVO_FINAL.md` - Método detallado (anterior)
- `BUILD_PROCESS_DOCUMENTATION.md` - Proceso completo (anterior)
- `COMPILACION_EXITOSA_METODO.md` - Método exitoso (anterior)

### Dependencias
- `DEPENDENCIAS_CRITICAS_NO_BORRAR.md` - Lista crítica
- `SDL2-devel-2.32.10-mingw.tar.gz` - SDL2 precompilado (archivo)

### Scripts
- `COMPILE_NIGHTMARE_DEFINITIVO.bat` - Script principal
- `VERIFICAR_DEPENDENCIAS.bat` - Verificación
- `COMPILE_DEVILUTIONX_DEFINITIVO.bat` - Script anterior
- `QUICK_START_TESTING.bat` - Testing

### Features Implementadas
- `NPC_MICRO_MOVEMENTS_IMPLEMENTATION_ENERO_16_2026.md` - NPC Micro-Movements
- `RESUMEN_COMPLETO_ORACLE_ENERO_16_2026.md` - Sistema Oráculo
- `GUIA_VERIFICAR_OLLAMA_ENERO_16_2026.md` - Verificar Ollama

### Sesiones de Trabajo
- `RESUMEN_FINAL_SESION_COMPLETA_ENERO_16_2026.md` - Sesión actual
- `RESUMEN_SESION_NPC_MICRO_MOVEMENTS_ENERO_16_2026.md` - Sesión NPCs
- `RESUMEN_SESION_ORACLE_UI_ENERO_16_2026.md` - Sesión Oracle UI

---

## 🎯 FLUJOS DE TRABAJO COMUNES

### Compilar por Primera Vez
```
1. VERIFICAR_DEPENDENCIAS.bat
2. Leer COMPILACION_DEFINITIVA_GUIA_ETERNA.md
3. COMPILE_NIGHTMARE_DEFINITIVO.bat
4. Testear nightmare.exe
```

### Compilar Después de Cambios
```
1. COMPILE_NIGHTMARE_DEFINITIVO.bat
2. Testear nightmare.exe
```

### Resolver Problemas de Compilación
```
1. VERIFICAR_DEPENDENCIAS.bat
2. Leer errores en consola
3. Buscar error en COMPILACION_DEFINITIVA_GUIA_ETERNA.md
4. Aplicar solución
5. COMPILE_NIGHTMARE_DEFINITIVO.bat
```

### Recuperar Dependencias Perdidas
```
1. Leer DEPENDENCIAS_CRITICAS_NO_BORRAR.md
2. Identificar qué falta
3. Restaurar desde backup o descargar
4. VERIFICAR_DEPENDENCIAS.bat
5. COMPILE_NIGHTMARE_DEFINITIVO.bat
```

### Crear Build Limpio
```
1. mkdir build_NEW
2. COMPILE_NIGHTMARE_DEFINITIVO.bat
   (detectará que es build limpio y configurará CMake)
```

---

## 🔗 REFERENCIAS EXTERNAS

### Dependencias
- **SDL2**: https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10
- **zlib**: https://zlib.net/
- **MinGW-w64**: https://www.mingw-w64.org/
- **CMake**: https://cmake.org/download/

### Proyecto Original
- **DevilutionX**: https://github.com/diasurgical/devilutionx
- **Diablo 1**: Blizzard Entertainment

---

## 📝 NOTAS IMPORTANTES

### Versiones de Dependencias
```
SDL2:      2.32.10
zlib:      1.3.1
MinGW:     GCC 14.2.0
CMake:     4.2.1+
```

### Tamaños Esperados
```
deps/:                 ~200 MB
build_*/:              ~500 MB cada uno
nightmare.exe:         ~15-20 MB
DIABDAT.MPQ:           ~400-500 MB
```

### Tiempos de Compilación
```
Primera vez:           10-15 minutos
Incremental:           1-3 minutos
Build limpio:          5-10 minutos
```

---

## ✅ CHECKLIST DE COMPILACIÓN

### Antes de Compilar
- [ ] `deps/` existe y está completo
- [ ] `DIABDAT.MPQ` existe
- [ ] GCC y CMake en PATH
- [ ] Al menos 2GB de espacio libre
- [ ] Al menos un `build_SAFE*` como backup

### Durante Compilación
- [ ] No hay errores de "not found"
- [ ] No hay errores de linking
- [ ] Compilación progresa sin detenerse

### Después de Compilar
- [ ] `nightmare.exe` existe
- [ ] Tamaño >10 MB
- [ ] Juego inicia sin errores
- [ ] Menú principal funciona

---

## 🆘 AYUDA RÁPIDA

### ¿Qué archivo leer?
- **Compilar**: `COMPILACION_DEFINITIVA_GUIA_ETERNA.md`
- **Dependencias**: `DEPENDENCIAS_CRITICAS_NO_BORRAR.md`
- **Problemas**: Sección "Diagnóstico" en guía principal
- **Features**: Archivos específicos de cada feature

### ¿Qué script ejecutar?
- **Compilar**: `COMPILE_NIGHTMARE_DEFINITIVO.bat`
- **Verificar**: `VERIFICAR_DEPENDENCIAS.bat`
- **Testear**: `QUICK_START_TESTING.bat`

### ¿Dónde buscar ayuda?
1. Este índice (para encontrar documentación)
2. `COMPILACION_DEFINITIVA_GUIA_ETERNA.md` (guía completa)
3. `DEPENDENCIAS_CRITICAS_NO_BORRAR.md` (si borraste algo)
4. Scripts `.bat` (tienen mensajes de error descriptivos)

---

## 🎓 FILOSOFÍA DEL PROYECTO

### Reglas de Oro
1. **NUNCA borrar `deps/`** - Difícil de recuperar
2. **Mantener backups** - Al menos un `build_SAFE`
3. **Crear nuevo, no limpiar** - Más seguro
4. **Verificar antes de compilar** - Evita errores
5. **Documentar todo** - Para el futuro

### Principios
- **Seguridad primero**: Backups antes de cambios
- **Claridad**: Documentación detallada
- **Automatización**: Scripts para tareas comunes
- **Recuperabilidad**: Siempre hay forma de volver atrás

---

**Creado por**: Kiro AI  
**Fecha**: Enero 16, 2026  
**Branch**: `compilacionExitosa`  
**Versión**: 1.0

**Este archivo es tu punto de entrada a toda la documentación de compilación** 📚
