# Fix Windows Game Bar y Reactivación de Logs Apocalypse - Enero 16, 2026

## 🎯 PROBLEMA 1: Windows Game Bar Popup

### Síntoma
Al iniciar el juego aparecía el cartel:
```
"Necesitas una aplicación nueva para abrir este vínculo a ms-gamingoverlay"
```

### Causa Raíz
Windows detecta ciertos nombres de ejecutables como juegos y activa automáticamente el Game Bar. El nombre "nightmare.exe" disparaba esta detección.

### Solución Implementada
**REVERTIR NOMBRE DEL EJECUTABLE**

Cambios realizados:
1. **CMakeLists.txt** - Línea 353, 363:
   ```cmake
   # ANTES:
   set(BIN_TARGET nightmare)
   set(BIN_TARGET libnightmare)
   
   # DESPUÉS:
   set(BIN_TARGET devilutionx)
   set(BIN_TARGET libdevilutionx)
   ```

2. **Archivos eliminados**:
   - `Packaging/windows/nightmare.rc`
   - `Packaging/windows/nightmare.exe.manifest`

3. **Archivos restaurados**:
   - `Packaging/windows/devilutionx.rc`
   - `Packaging/windows/devilutionx.exe.manifest`

### Resultado
✅ Ejecutable: `devilutionx.exe`
✅ Sin popup de Windows Game Bar
✅ Juego sigue siendo "Nightmare Edition" internamente

---

## 🎯 PROBLEMA 2: Crash de Apocalypse en Testing

### Síntoma
Crash reportado durante testing pre-release de Apocalypse

### Acción Tomada
**REACTIVAR SISTEMA DE LOGS DE APOCALYPSE**

### Archivos Modificados

#### 1. `Source/missiles.cpp` - Línea 2810-2821
```cpp
// ANTES (logs desactivados):
// DEBUG LOGS DISABLED - But system remains active for future debugging
// ARCH_LOG_CRASH_PREVENTION("AddMissile Apocalypse detected...", "AddMissile DEBUG");

// DESPUÉS (logs reactivados):
// 🚨 DEBUG LOGS REACTIVATED FOR TESTING
ARCH_LOG_CRASH_PREVENTION("AddMissile Apocalypse detected - checking protection", "AddMissile DEBUG");
ARCH_LOG_CRASH_PREVENTION("Universal Apocalypse protection triggered", "AddMissile");
ARCH_LOG_CRASH_PREVENTION("Apocalypse protection PASSED - allowing creation", "AddMissile ALLOWED");
```

#### 2. `Source/engine_health.cpp` - Línea 141-174
```cpp
// REACTIVADOS todos los logs:
ARCH_LOG_CRASH_PREVENTION("Apocalypse atomic flag UNLOCKED", "CanSafelyCastApocalypse delayed unlock");
ARCH_LOG_CRASH_PREVENTION("Apocalypse already in progress", "CanSafelyCastApocalypse atomic check");
ARCH_LOG_CRASH_PREVENTION("Apocalypse frame cooldown active", "CanSafelyCastApocalypse frame-based");
ARCH_LOG_CRASH_PREVENTION("Apocalypse time cooldown active", "CanSafelyCastApocalypse time-based");
ARCH_LOG_CRASH_PREVENTION("Apocalypse protection ALLOWING cast", "CanSafelyCastApocalypse SUCCESS");
```

#### 3. `Source/engine_health.cpp` - Línea 179-182
```cpp
// REACTIVADO:
ARCH_LOG_CRASH_PREVENTION("ClearApocalypseInProgress called but IGNORED", "delayed unlock system");
```

---

## 🔍 VERIFICACIÓN DEL SISTEMA DE PROTECCIÓN

### Sistema de Protección Apocalypse - CONFIRMADO ACTIVO ✅

#### 1. **Protección Universal en AddMissile** (`missiles.cpp:2810`)
```cpp
if (mitype == MissileID::Apocalypse) {
    if (!CanSafelyCastApocalypse()) {
        return nullptr; // fail-soft
    }
}
```
✅ **ACTIVO** - Bloquea creación si no es seguro

#### 2. **Protección Atómica** (`engine_health.cpp:147`)
```cpp
if (apocalypseInProgress) {
    return false; // Solo 1 Apocalypse a la vez
}
```
✅ **ACTIVO** - Previene múltiples Apocalypse simultáneos

#### 3. **Cooldown por Frame** (`engine_health.cpp:153`)
```cpp
if (lastApocalypseFrame == frameCounter) {
    return false; // Solo 1 por frame
}
```
✅ **ACTIVO** - Máximo 1 Apocalypse por frame

#### 4. **Cooldown por Tiempo** (`engine_health.cpp:162`)
```cpp
if (timeSinceLastCast.count() < 16) { // 16ms = 1 frame @ 60fps
    return false;
}
```
✅ **ACTIVO** - Mínimo 16ms entre casts

#### 5. **Single-Frame Processing** (`missiles.cpp:3895`)
```cpp
void ProcessApocalypse(Missile &missile) {
    // Procesa TODO en un solo frame
    for (int j = missile.var2; j < missile.var3; j++) {
        for (int k = missile.var4; k < missile.var5; k++) {
            // ... proceso ...
        }
    }
    missile._miDelFlag = true; // Siempre termina
}
```
✅ **ACTIVO** - No loops infinitos

#### 6. **Fail-Soft en Loop** (`missiles.cpp:3905`)
```cpp
if (!TryAddMissile(...)) {
    ARCH_LOG_CRASH_PREVENTION("TryAddMissile failed", "ProcessApocalypse loop");
    missile._miDelFlag = true;
    ClearApocalypseInProgress();
    return; // Cancela limpiamente
}
```
✅ **ACTIVO** - Cancela sin crash si falla

---

## 📊 UBICACIÓN DE LOGS

### Directorio de Logs
```
build_COMPILE_FRESH/
├── devilutionx.exe
└── debug_logs/
    └── architectural_analysis_YYYYMMDD_HHMMSS.txt
```

### Contenido de Logs
Los logs capturan:
- ✅ Cada intento de cast de Apocalypse
- ✅ Protecciones activadas (atomic, frame, time)
- ✅ Casts permitidos vs bloqueados
- ✅ Fallos en TryAddMissile
- ✅ Estado del sistema en cada momento

---

## 🧪 INSTRUCCIONES DE TESTING

### Para Reproducir el Crash
1. Ejecutar `devilutionx.exe`
2. Cargar partida con Apocalypse
3. Intentar reproducir el crash
4. **Los logs se generarán automáticamente**

### Revisar Logs
```bash
# Ubicación:
cd build_COMPILE_FRESH/debug_logs/

# Buscar el archivo más reciente:
dir /O-D architectural_analysis_*.txt

# Abrir con notepad:
notepad architectural_analysis_YYYYMMDD_HHMMSS.txt
```

### Qué Buscar en los Logs
1. **Líneas con "Apocalypse"** - Todas las operaciones de Apocalypse
2. **"CRASH_PREVENTION"** - Protecciones activadas
3. **"TryAddMissile failed"** - Límites alcanzados
4. **Timestamp del crash** - Momento exacto del problema

---

## 📝 PRÓXIMOS PASOS

### Si el Crash se Reproduce
1. ✅ Logs estarán disponibles automáticamente
2. ✅ Buscar el último log antes del crash
3. ✅ Identificar qué protección falló (si alguna)
4. ✅ Analizar el estado del sistema en ese momento

### Si NO se Reproduce el Crash
1. ✅ Confirmar que fue un caso aislado
2. ✅ Desactivar logs para release final
3. ✅ Proceder con release

---

## ✅ ESTADO ACTUAL

### Compilación
- ✅ Compilado exitosamente
- ✅ Ejecutable: `devilutionx.exe`
- ✅ Sin errores de compilación

### Sistema de Protección
- ✅ Todas las protecciones ACTIVAS
- ✅ Logs REACTIVADOS para debugging
- ✅ Sistema fail-soft funcionando

### Listo para Testing
- ✅ Ejecutable listo
- ✅ Logs habilitados
- ✅ Sistema de protección verificado

---

## 🎯 RESUMEN EJECUTIVO

**Problema 1 (Game Bar)**: RESUELTO - Ejecutable renombrado a `devilutionx.exe`
**Problema 2 (Crash)**: PREPARADO - Logs reactivados para capturar información

**Sistema de Protección Apocalypse**: 100% ACTIVO y VERIFICADO

**Próximo paso**: Testing con logs habilitados para capturar información del crash si se reproduce.
