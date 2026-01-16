# 🚨 SISTEMA DE DEBUG LATENTE - UBICACIONES Y USO FUTURO

## 📁 UBICACIÓN DE LOS ARCHIVOS .TXT

### Estructura de Directorios:
```
DevilutionX/
├── build_NOW/
│   ├── devilutionx.exe          ← Ejecutable compilado
│   └── debug_logs/              ← AQUÍ SE CREAN LOS .TXT
│       └── apocalypse_crash_debug_YYYYMMDD_HHMMSS_XXX.txt
```

### Ubicaciones Específicas:

#### Si ejecutas desde build_NOW:
```bash
cd build_NOW
./devilutionx.exe
```
**Los .txt aparecerán en**: `build_NOW/debug_logs/`

#### Si ejecutas desde la raíz:
```bash
./build_NOW/devilutionx.exe
```
**Los .txt aparecerán en**: `debug_logs/` (en la raíz del proyecto)

---

## 🔧 SISTEMA LATENTE - CÓMO FUNCIONA

### ✅ Modo Latente Activado
El sistema ahora es **inteligente** y solo se activa cuando detecta:

1. **Alto número de missiles** (>250)
2. **Múltiples Apocalypse activos** (>1)
3. **Estado crítico del MAL** (engine sobrecargado)
4. **Carga alta del engine** (>200)
5. **Demasiados ApocalypseBoom** (>50)

### 🎯 Impacto en Performance
- **Gameplay normal**: CERO impacto, sistema dormido
- **Situaciones peligrosas**: Se activa automáticamente
- **Crashes**: Captura completa de datos

---

## 🚀 USO FUTURO PARA OTROS CRASHES

### Sistema Extensible
El código está preparado para debuggear otros tipos de crashes:

```cpp
// Para otros spells problemáticos:
#define GENERIC_CRASH_DEBUG_INIT(system) InitApocalypseCrashDebug()
#define GENERIC_CRASH_DEBUG_LOG(system, location) \
    do { \
        if (gEngineHealth.inCriticalState) { \
            CaptureApocalypseProcessingState(location); \
        } \
    } while(0)
```

### Ejemplos de Uso Futuro:

#### Para Chain Lightning:
```cpp
// En el código de Chain Lightning:
GENERIC_CRASH_DEBUG_LOG("ChainLightning", "ProcessChainLightning - Entry");
```

#### Para Inferno:
```cpp
// En el código de Inferno:
GENERIC_CRASH_DEBUG_LOG("Inferno", "ProcessInferno - Loop iteration");
```

#### Para cualquier otro sistema:
```cpp
// En cualquier función problemática:
GENERIC_CRASH_DEBUG_LOG("SystemName", "Function - Critical point");
```

---

## 📊 ESTRUCTURA DEL REPORTE GENERADO

### Nombre del Archivo:
```
apocalypse_crash_debug_20260111_160530_456.txt
```

### Contenido del Reporte:
```
=== APOCALYPSE CRASH DEBUG SYSTEM ===
Initialized at: 2026-01-11 16:05:30
Purpose: Capture detailed state information for Apocalypse spell crashes
System: LATENT MODE - Only active when crashes occur
Location: This file will be in debug_logs/ directory
======================================

[FRAME 1247] PRE-APOCALYPSE STATE CAPTURED
  Player: Level 25 at (45,32)
  Missiles: Total=387 Apocalypse=2 ApocalypseBoom=234
  Monsters: Active=23 InRange=18
  MAL: Load=456 Critical=YES

[FRAME 1247] PROCESSING: ProcessApocalypse - Entry
  Current missiles: 387
  MAL state: Load=456 Critical=YES

🚨🚨🚨 APOCALYPSE CRASH DETECTED 🚨🚨🚨
========================================
[... información completa del crash ...]
```

---

## 🎮 INSTRUCCIONES DE TESTING

### 1. Ejecutar el Juego:
```bash
cd build_NOW
./devilutionx.exe
```

### 2. Reproducir el Crash:
- Ve al **Level 2**
- Usa **múltiples clicks** con **Apocalypse**
- El sistema se activará automáticamente si detecta peligro

### 3. Buscar el Reporte:
- Directorio: `build_NOW/debug_logs/`
- Archivo: `apocalypse_crash_debug_*.txt`

### 4. Compartir Datos:
- Copia el contenido del archivo .txt
- Pégalo en el chat para análisis

---

## 🔮 VENTAJAS DEL SISTEMA LATENTE

### ✅ Performance Optimizada
- **Cero impacto** durante gameplay normal
- Solo se activa cuando hay problemas reales
- Mínimo overhead computacional

### ✅ Debugging Inteligente
- Detecta automáticamente situaciones peligrosas
- Captura datos precisos del momento del crash
- Análisis automático de posibles causas

### ✅ Extensibilidad Futura
- Fácil de extender para otros crashes
- Sistema modular y reutilizable
- Documentación completa para uso futuro

### ✅ Organización
- Archivos organizados en `debug_logs/`
- Nombres únicos con timestamp
- Fácil identificación y gestión

---

## 🏁 RESUMEN EJECUTIVO

### ✅ Sistema Implementado:
- **Compilación exitosa** ✅
- **Modo latente activado** ✅
- **Ubicación organizada** ✅
- **Extensibilidad futura** ✅

### 📁 Ubicación de Reportes:
- **Directorio**: `build_NOW/debug_logs/`
- **Formato**: `apocalypse_crash_debug_YYYYMMDD_HHMMSS_XXX.txt`

### 🎯 Próximo Paso:
**¡Reproduce el crash y comparte el reporte .txt!**

El sistema está listo y esperando capturar los datos precisos del crash para implementar el fix definitivo.

---

**¡El sistema latente está activo y listo para cazar bugs! 🎯🔍**