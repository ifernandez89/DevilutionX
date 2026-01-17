# 🔧 CRITICAL FIX: ReturnHome Anti-Oscillation System
**Fecha**: Enero 16, 2026  
**Branch**: `npcWalk`  
**Commit**: `c6393faea`  
**Severidad**: CRÍTICA (previene loop infinito)

---

## 🚨 PROBLEMA IDENTIFICADO

### A. Pathfinding de ReturnHome
**Issue**: Oscilación infinita si bloqueado

```cpp
// ANTES (MALO):
void ReturnHome(Towner &npc) {
    // ...
    if (!IsTileWalkableForNPC(targetPos)) {
        // Teleport inmediato (sin intentos)
        TeleportHome();
    }
    // ...
}
```

**Escenarios problemáticos**:
1. ❌ Otro NPC parado en el camino → oscilación
2. ❌ Player bloqueando → oscilación
3. ❌ Tile temporalmente ocupado → oscilación
4. ❌ Sin timeout claro
5. ❌ Sin contador de intentos

### B. Retry Logic en TryStartWalk
**Status**: ✅ YA ESTABA BIEN

```cpp
// CORRECTO:
if (!IsTileWalkableForNPC(targetPos)) {
    npc.nextMicroMoveTick = currentTime + 2000-5000;  // Reprograma timer
    return;  // No reintenta inmediatamente
}
```

**Por qué está bien**:
- ✅ Reprograma `nextMicroMoveTick`
- ✅ No reintenta en el mismo frame
- ✅ Retry después de 2-5 segundos

---

## ✅ SOLUCIÓN IMPLEMENTADA

### 1. Añadido Campo a Estructura
```cpp
// Source/towners.h
struct Towner {
    // ... campos existentes ...
    
    // FEATURE: NPC Micro-Movements
    Point home;
    uint8_t homeRadius;
    uint32_t nextMicroMoveTick;
    uint8_t microState;
    bool microEnabled;
    uint8_t returnAttempts;  // ← NUEVO: Contador de intentos
};
```

### 2. Inicialización
```cpp
// Source/towners.cpp - InitNPCMicroMovement()
void InitNPCMicroMovement(Towner &npc)
{
    npc.home = npc.position;
    npc.homeRadius = 2;
    npc.microState = IDLE;
    npc.nextMicroMoveTick = SDL_GetTicks() + 8000-15000;
    npc.microEnabled = CanNPCUseMicroMovement(npc);
    npc.returnAttempts = 0;  // ← NUEVO: Inicializar contador
}
```

### 3. Reset al Iniciar Return
```cpp
// Source/towners.cpp - UpdateWalk()
if (IsTooFarFromHome(npc)) {
    npc.microState = RETURN_HOME;
    npc.nextMicroMoveTick = currentTime + WALK_DURATION;
    npc.returnAttempts = 0;  // ← NUEVO: Reset al iniciar
}
```

### 4. Lógica de Fallback Mejorada
```cpp
// Source/towners.cpp - ReturnHome()
void ReturnHome(Towner &npc)
{
    const uint32_t currentTime = SDL_GetTicks();
    
    // Check timer
    if (currentTime < npc.nextMicroMoveTick) return;
    
    // Already home?
    if (npc.position == npc.home) {
        npc.microState = IDLE;
        npc.returnAttempts = 0;  // ← Reset al llegar
        return;
    }
    
    // SAFETY: Too many attempts? Teleport immediately
    if (npc.returnAttempts > 3) {
        TeleportHome(npc);
        npc.microState = IDLE;
        npc.returnAttempts = 0;  // ← Reset después de teleport
        
        #ifdef _DEBUG
        LogVerbose("Teleported home (too many attempts)");
        #endif
        return;
    }
    
    // Try to move
    Direction dirToHome = GetDirection(npc.position, npc.home);
    Point targetPos = npc.position + dirToHome;
    
    if (!IsTileWalkableForNPC(targetPos)) {
        // Blocked! Increment counter and try again next tick
        npc.returnAttempts++;
        npc.nextMicroMoveTick = currentTime + WALK_DURATION;
        
        #ifdef _DEBUG
        LogVerbose("Blocked returning home (attempt {}/3)", npc.returnAttempts);
        #endif
        return;
    }
    
    // Move successful!
    MoveNPC(npc, targetPos);
    npc.returnAttempts = 0;  // ← Reset en movimiento exitoso
    
    // Continue or finish
    if (npc.position == npc.home) {
        npc.microState = IDLE;
    } else {
        npc.nextMicroMoveTick = currentTime + WALK_DURATION;
    }
}
```

---

## 🎯 COMPORTAMIENTO NUEVO

### Escenario 1: Camino Libre
```
Intento 1: Move exitoso → returnAttempts = 0
Intento 2: Move exitoso → returnAttempts = 0
Intento 3: Llega a home → IDLE
```

### Escenario 2: Bloqueado Temporalmente
```
Intento 1: Bloqueado → returnAttempts = 1
Intento 2: Bloqueado → returnAttempts = 2
Intento 3: Camino libre → Move exitoso → returnAttempts = 0
Intento 4: Move exitoso → returnAttempts = 0
Intento 5: Llega a home → IDLE
```

### Escenario 3: Bloqueado Permanentemente
```
Intento 1: Bloqueado → returnAttempts = 1
Intento 2: Bloqueado → returnAttempts = 2
Intento 3: Bloqueado → returnAttempts = 3
Intento 4: Bloqueado → returnAttempts = 4 (>3)
         → TELEPORT A HOME → IDLE
```

### Escenario 4: Oscilación Prevenida
```
ANTES (MALO):
Frame 1: Bloqueado → Teleport
Frame 2: Bloqueado → Teleport
Frame 3: Bloqueado → Teleport
... (loop infinito)

AHORA (BUENO):
Frame 1: Bloqueado → returnAttempts = 1, wait WALK_DURATION
Frame 2: (esperando...)
Frame 3: (esperando...)
Frame 4: Bloqueado → returnAttempts = 2, wait WALK_DURATION
... (máximo 4 intentos, luego teleport)
```

---

## 🔍 PREVENCIÓN DE PROBLEMAS

### 1. Oscilación Infinita
```
✅ Contador de intentos (máximo 3)
✅ Teleport después de 3 intentos fallidos
✅ Reset en movimiento exitoso
```

### 2. Loop Rápido en Mismo Frame
```
✅ Usa nextMicroMoveTick (no reintenta inmediatamente)
✅ Wait WALK_DURATION entre intentos (1.5s)
```

### 3. NPC Atascado Permanentemente
```
✅ Teleport a home después de 3 intentos
✅ Transición a IDLE después de teleport
✅ Logs de debug para tracking
```

### 4. Bloqueo por Otros NPCs
```
✅ Intenta 3 veces (otros NPCs pueden moverse)
✅ Si sigue bloqueado → teleport
```

### 5. Bloqueo por Player
```
✅ Intenta 3 veces (player puede moverse)
✅ Si sigue bloqueado → teleport
✅ Cancelación agresiva si player cerca (<5 tiles)
```

---

## 📊 MÉTRICAS DE SEGURIDAD

### Timeout Claro
```
Máximo tiempo atascado: 3 intentos × 1.5s = 4.5 segundos
Después: Teleport a home (seguro)
```

### Contador de Intentos
```
Máximo: 3 intentos
Después: Teleport automático
Reset: En movimiento exitoso o al llegar a home
```

### Logs de Debug
```
✅ "Blocked returning home (attempt 1/3)"
✅ "Blocked returning home (attempt 2/3)"
✅ "Blocked returning home (attempt 3/3)"
✅ "Teleported home (too many attempts)"
```

---

## 🧪 TESTING ADICIONAL

### Test 1: Bloqueo Temporal
```bash
# Bloquear camino de Farnham con player
# Esperar 1.5s
# Moverse
# Verificar que Farnham continúa
```

**Esperado**:
```
✅ Intento 1: "Blocked returning home (attempt 1/3)"
✅ Player se mueve
✅ Intento 2: Move exitoso, returnAttempts = 0
✅ Llega a home
```

### Test 2: Bloqueo Permanente
```bash
# Bloquear camino de Adria permanentemente
# Esperar 6 segundos (4 intentos)
```

**Esperado**:
```
✅ Intento 1: "Blocked (attempt 1/3)"
✅ Intento 2: "Blocked (attempt 2/3)"
✅ Intento 3: "Blocked (attempt 3/3)"
✅ Intento 4: "Teleported home (too many attempts)"
✅ Adria en home, estado IDLE
```

### Test 3: Oscilación Prevenida
```bash
# Crear situación de oscilación
# Verificar que no hay loop infinito
```

**Esperado**:
```
✅ Máximo 4 intentos
✅ Teleport después de 3 intentos fallidos
✅ No loop infinito
```

---

## ✅ VALIDACIÓN

### Diagnósticos
```
✅ Source/towners.h: No diagnostics found
✅ Source/towners.cpp: No diagnostics found
```

### Archivos Modificados
- `Source/towners.h` - Añadido `returnAttempts`
- `Source/towners.cpp` - Lógica de fallback mejorada

### Líneas Añadidas
- **Estructura**: 1 campo nuevo
- **Inicialización**: 1 línea
- **Reset**: 3 ubicaciones
- **Lógica**: ~20 líneas
- **Total**: ~25 líneas

---

## 🎯 IMPACTO

### Antes del Fix
```
❌ Oscilación infinita posible
❌ Sin timeout claro
❌ Sin contador de intentos
❌ Teleport inmediato (sin intentos)
❌ Difícil de debuggear
```

### Después del Fix
```
✅ Oscilación prevenida (máximo 3 intentos)
✅ Timeout claro (4.5 segundos máximo)
✅ Contador de intentos implementado
✅ Teleport después de 3 intentos
✅ Logs de debug para tracking
✅ Reset en movimiento exitoso
```

---

## 💡 LECCIONES APRENDIDAS

### 1. Pathfinding Simple Necesita Fallbacks
```
Pathfinding directo (GetDirection) es simple pero:
- Puede bloquearse
- Necesita contador de intentos
- Necesita timeout claro
- Necesita teleport como último recurso
```

### 2. Retry Logic Debe Ser Conservador
```
✅ TryStartWalk: Reprograma timer (2-5s)
✅ ReturnHome: Reprograma timer (1.5s)
❌ NO reintentar en mismo frame
```

### 3. Logs de Debug Son Críticos
```
Sin logs: Difícil saber si hay oscilación
Con logs: Fácil identificar problemas
```

### 4. Contadores Simples Son Efectivos
```
uint8_t returnAttempts = 0;
if (returnAttempts > 3) TeleportHome();

Simple, efectivo, fácil de debuggear.
```

---

## 📝 CHECKLIST FINAL

### Código
- [x] Campo `returnAttempts` añadido
- [x] Inicialización en InitNPCMicroMovement
- [x] Reset al iniciar RETURN_HOME
- [x] Reset al llegar a home
- [x] Reset en movimiento exitoso
- [x] Lógica de fallback (>3 intentos)
- [x] Logs de debug añadidos
- [x] Cero errores de diagnóstico

### Testing (Pendiente en Casa)
- [ ] Bloqueo temporal funciona
- [ ] Bloqueo permanente → teleport
- [ ] No hay oscilación infinita
- [ ] Logs de debug funcionan

---

**Implementado por**: Kiro AI  
**Fecha**: Enero 16, 2026  
**Commit**: `c6393faea`  
**Severidad**: CRÍTICA  
**Status**: ✅ IMPLEMENTADO Y PUSHEADO

**Gracias por el catch!** Este fix previene un bug crítico de oscilación infinita.
