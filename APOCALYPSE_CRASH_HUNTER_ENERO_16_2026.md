# APOCALYPSE CRASH HUNTER - SISTEMA DE CAZA DEL CULPABLE
**Fecha**: Enero 16, 2026  
**Objetivo**: Encontrar la causa exacta del crash determinístico

---

## 🎯 ESTRATEGIA

El Architectural Shield agresivo estaba ACTIVO cuando debería estar desactivado, causando más problemas que soluciones. Ahora vamos a:

1. **DESACTIVAR** todos los sistemas que puedan interferir con dungeons
2. **ACTIVAR** logging agresivo para encontrar al culpable
3. **VOLVER** casi al estado original de DevilutionX
4. **ENCONTRAR** exactamente qué operación causa el crash

---

## 🚫 SISTEMAS DESACTIVADOS

### 1. Architectural Shield (COMPLETAMENTE DESACTIVADO)
- **Archivo**: `Source/architectural_shield.cpp`
- **Cambios**:
  - `SanitizeCoordinates()` → Retorna false, no sanitiza nada
  - `SanitizePaletteData()` → No hace nada
  - `InterceptAndSanitizeData()` → No intercepta nada
  - `InitializeArchitecturalShield()` → Solo loguea que está desactivado

**Razón**: Sistema agresivo estaba "sanitizando" coordenadas VÁLIDAS (75,68) → (39,39), causando teletransporte espontáneo y posiblemente corrupción de datos.

### 2. Nightmare Items (DESACTIVADO)
- **Archivo**: `Source/nightmare_items_DISABLED.h`
- **Qué hace**: Items especiales que se generan en dungeons
- **Razón**: Puede estar generando items con datos corruptos

### 3. Hidden Content (DESACTIVADO)
- **Archivo**: `Source/hidden_content_DISABLED.h`
- **Qué hace**: Textos y diálogos ocultos recuperados
- **Razón**: Puede estar inyectando datos no validados

### 4. Dormant Assets (DESACTIVADO)
- **Archivo**: `Source/dormant_assets_DISABLED.h`
- **Qué hace**: Assets visuales dormidos (sprites, decoraciones)
- **Razón**: Puede estar modificando generación de dungeons

### 5. Nightmare Audio (DESACTIVADO)
- **Archivo**: `Source/nightmare_audio_DISABLED.h`
- **Qué hace**: Efectos de audio atmosféricos
- **Razón**: Puede estar interfiriendo con el game loop

---

## ✅ SISTEMAS ACTIVOS

### 1. Crash Hunter (NUEVO - LOGGING AGRESIVO)
- **Archivos**: `Source/crash_hunter.h`, `Source/crash_hunter.cpp`
- **Qué hace**: Loguea TODOS los eventos críticos relacionados con dungeons
- **Log file**: `build_CRASH_HUNTER/debug_logs/crash_hunter.log`

**Eventos logueados**:
- 🏗️ Generación de niveles
- 👹 Spawn de monstruos (tipo, coordenadas, nombre)
- 💎 Spawn de items (tipo, coordenadas, nombre)
- 🚶 Posición del jugador (con contexto)
- 🚪 Cambios de nivel
- 💾 Carga de saves
- 📊 Datos de dungeon
- ⚠️ Operaciones críticas
- ✅ Checkpoints (puntos seguros)

### 2. Apocalypse Protection (MANTENER)
- **Archivo**: `Source/apocalypse_protection.cpp`
- **Qué hace**: Protección específica para el spell Apocalypse
- **Razón**: Es la única protección que necesitamos

### 3. Corruption Detector (MANTENER)
- **Archivo**: `Source/corruption_detector.cpp`
- **Qué hace**: Detecta saves corruptos y fuerza factory reset
- **Razón**: Sistema de prevención, no de sanitización

---

## 📋 CÓMO USAR EL CRASH HUNTER

### Paso 1: Compilar
```batch
COMPILE_CRASH_HUNTER.bat
```

### Paso 2: Ejecutar el juego
```batch
cd build_CRASH_HUNTER
devilutionx.exe
```

### Paso 3: Reproducir el crash
- Carga tu save
- Juega normalmente
- Espera el crash

### Paso 4: Analizar el log
```batch
notepad build_CRASH_HUNTER\debug_logs\crash_hunter.log
```

**Busca**:
- El ÚLTIMO evento antes del crash
- Patrones repetitivos
- Coordenadas sospechosas
- Operaciones críticas fallidas

---

## 🔍 QUÉ BUSCAR EN EL LOG

### Patrón 1: Coordenadas corruptas
```
🚶 PLAYER POSITION: (999,999) - After teleport
```
→ Coordenadas claramente inválidas

### Patrón 2: Spawn en posición inválida
```
👹 MONSTER SPAWN: Zombie (Type:5) at (-50,-50)
💎 ITEM SPAWN: Gold (Type:1) at (200,200)
```
→ Coordenadas fuera de rango

### Patrón 3: Operación crítica fallida
```
⚠️ CRITICAL: LoadMonster - Invalid monster data
⚠️ CRITICAL: GenerateLevel - Dungeon generation failed
```
→ Operación que falló justo antes del crash

### Patrón 4: Cambio de nivel problemático
```
🚪 LEVEL CHANGE: From Level 5 to Level 6
[CRASH]
```
→ Crash durante cambio de nivel

---

## 🎯 PRÓXIMOS PASOS

1. **Compilar** con `COMPILE_CRASH_HUNTER.bat`
2. **Reproducir** el crash
3. **Analizar** `crash_hunter.log`
4. **Identificar** el último evento antes del crash
5. **Reportar** hallazgos para investigación quirúrgica

---

## 📊 ESTADO ACTUAL

- ✅ Architectural Shield DESACTIVADO
- ✅ Nightmare Items DESACTIVADO
- ✅ Hidden Content DESACTIVADO
- ✅ Dormant Assets DESACTIVADO
- ✅ Nightmare Audio DESACTIVADO
- ✅ Crash Hunter ACTIVO
- ✅ Apocalypse Protection ACTIVO
- ✅ Corruption Detector ACTIVO

---

## 🔥 FILOSOFÍA

**ANTES**: "Sanitizar todo, confiar en nada"  
**AHORA**: "Confiar en DevilutionX original, loguear todo, encontrar al culpable"

**OBJETIVO**: Volver casi al estado original de DevilutionX para aislar el problema real.

---

## ⚠️ IMPORTANTE

Este build es SOLO para debugging. Una vez encontrado el culpable:
1. Eliminar Crash Hunter (demasiado logging)
2. Reactivar sistemas necesarios
3. Aplicar fix quirúrgico al problema real
4. Mantener código limpio y simple

---

**¡VAMOS A ENCONTRAR A ESE HIJO DE PERRA!** 🎯
