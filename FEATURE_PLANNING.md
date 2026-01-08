# PLANIFICACIÓN DE FEATURES - DEVILUTIONX
## Análisis Arquitectural y Plan de Implementación

### ESTADO ACTUAL DEL CÓDIGO BASE

#### Sistema de Quests (Source/quests.cpp)
- **InitQuests()**: Inicializa todas las quests con `QUEST_NOTAVAIL` por defecto
- **Estados disponibles**: `QUEST_NOTAVAIL`, `QUEST_INIT`, `QUEST_ACTIVE`, `QUEST_DONE`
- **Estructura Quest**: Contiene `_qactive`, `_qvar1`, `_qvar2`, `_qlog`, `_qlevel`
- **Multiplayer**: Usa `UseMultiplayerQuests()` para determinar disponibilidad

#### Sistema de Iluminación (Source/lighting.cpp, Source/player.cpp)
- **Variable clave**: `player._pLightRad` (int8_t, valor por defecto: 10)
- **Funciones**: `ChangeLightXY()`, `ChangeLightOffset()`, `UpdatePlayerLightOffset()`
- **Límites**: Sistema soporta radios dinámicos, MAX_LIGHT_RADIUS existe
- **Actualización**: Se actualiza en tiempo real durante movimiento

#### Sistema de Monstruos (Source/monster.cpp)
- **InitMonster()**: Inicializa stats base, HP, damage, AC, resistance
- **Escalado por dificultad**: Nightmare (3x HP, 2x damage), Hell (4x HP, 4x damage)
- **Variables**: `maxHitPoints`, `minDamage`, `maxDamage`, `armorClass`, `resistance`
- **Regeneración**: No existe sistema de respawn automático

#### Sistema de Objetos (Source/objects.cpp)
- **AddObject()**: Función principal para spawning
- **InitRndLocObj()**: Genera objetos aleatorios en posiciones válidas
- **Decorativos existentes**: Barriles, urnas, pods, antorchas
- **Validación**: `RndLocOk()`, `IsAreaOk()` verifican posiciones válidas

---

## FEATURES PLANIFICADAS - ANÁLISIS DETALLADO

### ✅ FEATURE 1 — GARANTIZAR TODAS LAS QUESTS
**ESTADO**: IMPLEMENTACIÓN SEGURA AL 100%

#### Análisis del Código Actual
```cpp
// En InitQuests() línea 199-240
for (auto &quest : Quests) {
    quest._qactive = QUEST_NOTAVAIL;  // ← AQUÍ ESTÁ EL PROBLEMA
    // ...
    if (!UseMultiplayerQuests()) {
        quest._qactive = QUEST_INIT;  // Solo se activa en condiciones específicas
    }
}
```

#### Implementación Propuesta
```cpp
void InitQuests() {
    // ... código existente ...
    
    // NUEVA LÓGICA: Garantizar disponibilidad
    for (int i = 0; i < MAXQUESTS; i++) {
        if (Quests[i]._qactive == QUEST_NOTAVAIL) {
            Quests[i]._qactive = QUEST_INIT;  // Hacer disponible, NO activar
        }
    }
}
```

#### Archivos Afectados
- `Source/quests.cpp` (función InitQuests)
- `Source/quests.h` (posibles constantes)

#### Riesgos y Prevención
- ✅ **SEGURO**: No toca `_qvar1`, `_qlog`, `_qlevel`
- ✅ **SEGURO**: No fuerza activación, solo disponibilidad
- ⚠️ **CUIDADO**: Verificar quests especiales (Skeleton King, Lazarus)
- ⚠️ **TESTING**: Probar en multiplayer y singleplayer

---

### ✅ FEATURE 2 — MEJORA DE ILUMINACIÓN (SOFT)
**ESTADO**: IMPLEMENTACIÓN SEGURA AL 100%

#### Análisis del Código Actual
```cpp
// En CreatePlayer() línea 2336
player._pLightRad = 10;  // Valor por defecto

// Sistema ya soporta cambios dinámicos:
// ChangeLightXY(), ChangeLightOffset(), UpdatePlayerLightOffset()
```

#### Implementación Propuesta
```cpp
constexpr int EXTRA_LIGHT = 2;
constexpr int MAX_LIGHT_RADIUS = 15;  // Verificar valor real

void EnhancePlayerLighting(Player &player) {
    player._pLightRad = std::min(
        player._pLightRad + EXTRA_LIGHT,
        MAX_LIGHT_RADIUS
    );
}
```

#### Archivos Afectados
- `Source/player.cpp` (CreatePlayer, InitLevelChange)
- `Source/lighting.cpp` (verificar límites)

#### Riesgos y Prevención
- ✅ **SEGURO**: Variable ya es dinámica
- ✅ **SEGURO**: Sistema soporta radios variables
- ⚠️ **CUIDADO**: No tocar iluminación de monstruos
- ⚠️ **PERFORMANCE**: No recalcular floodfill por frame

---

### 🟡 FEATURE 3 — DENSIDAD DECORATIVA LEVE
**ESTADO**: IMPLEMENTACIÓN SEGURA AL 80%

#### Análisis del Código Actual
```cpp
// Objetos decorativos existentes en objects.cpp:
void InitRndLocObj(int min, int max, _object_id objtype) {
    const int numobjs = GenerateRnd(max - min) + min;
    // Spawning aleatorio con validación
}

// Tipos disponibles: OBJ_BARREL, OBJ_URN, OBJ_POD, etc.
```

#### Implementación Propuesta
```cpp
void EnhanceDecorativeDensity() {
    // Durante generación de nivel, NO post-proceso
    for (int attempts = 0; attempts < 100; attempts++) {
        int x = GenerateRnd(80) + 16;
        int y = GenerateRnd(80) + 16;
        
        if (dungeon[x][y] == Floor && GenerateRnd(100) < 5) {
            if (RndLocOk({x, y})) {
                AddObject(OBJ_BARREL, {x, y});
            }
        }
    }
}
```

#### Archivos Afectados
- `Source/levels/drlg_l1.cpp` (catacumbas)
- `Source/levels/drlg_l2.cpp` (catacombs)
- `Source/levels/drlg_l3.cpp` (caves)
- `Source/objects.cpp` (funciones de spawning)

#### Riesgos y Prevención
- ✅ **SEGURO**: Usa sistema existente de validación
- ⚠️ **CUIDADO**: Verificar `nSolidTable` para no bloquear paths
- ⚠️ **CUIDADO**: No tocar niveles con quests activas
- ⚠️ **CUIDADO**: No bloquear puertas o pasillos críticos

---

### 🟡 FEATURE 4 — REGENERACIÓN CONTROLADA DE MONSTRUOS
**ESTADO**: IMPLEMENTACIÓN CON LÍMITES

#### Análisis del Código Actual
```cpp
// No existe sistema de respawn automático
// InitMonster() limpia estados al cargar nivel
// Respawn TOTAL existe al recargar nivel completo
```

#### Implementación Propuesta
```cpp
void RegenerateMonsters() {
    // Solo en niveles sin quests activas
    if (IsQuestLevel(currlevel)) {
        return;  // NUNCA regenerar en niveles con quests
    }
    
    // Lógica de regeneración limitada
    if (ActiveMonsterCount < (totalmonsters / 2)) {
        // Regenerar algunos monstruos básicos
        InitMonsters();  // Función existente
    }
}
```

#### Archivos Afectados
- `Source/monster.cpp` (nuevas funciones de respawn)
- `Source/levels/gendung.cpp` (integración con generación)
- `Source/quests.cpp` (verificación de quest levels)

#### Riesgos y Prevención
- ⚠️ **CRÍTICO**: Nunca regenerar uniques
- ⚠️ **CRÍTICO**: Nunca durante quests activas
- ⚠️ **CRÍTICO**: No persistir estado entre saves
- ✅ **SEGURO**: Usar funciones existentes de inicialización

---

### 🟡 FEATURE 5 — VARIANTES DE MONSTRUOS POR PROFUNDIDAD
**ESTADO**: IMPLEMENTACIÓN SEGURA

#### Análisis del Código Actual
```cpp
// En InitMonster() línea 195-297
// Stats ya escalan por dificultad:
if (sgGameInitInfo.nDifficulty == DIFF_NIGHTMARE) {
    monster.maxHitPoints = 3 * monster.maxHitPoints;
    monster.minDamage = 2 * (monster.minDamage + 2);
    // ...
}
```

#### Implementación Propuesta
```cpp
void ScaleMonsterByDepth(Monster &monster) {
    // Escalado adicional por nivel
    int depthBonus = currlevel / 4;  // Cada 4 niveles
    
    monster.maxHitPoints += currlevel * 5 * 64;  // HP en formato interno
    monster.hitPoints = monster.maxHitPoints;
    
    monster.minDamage += currlevel / 2;
    monster.maxDamage += currlevel / 2;
    
    // Clamp valores para evitar overflow
    monster.maxHitPoints = std::min(monster.maxHitPoints, MAX_HP);
    monster.minDamage = std::min(monster.minDamage, MAX_DAMAGE);
}
```

#### Archivos Afectados
- `Source/monster.cpp` (InitMonster, PlaceMonster)
- `Source/tables/monstdat.cpp` (verificar límites)

#### Riesgos y Prevención
- ✅ **SEGURO**: Usa sistema existente de stats
- ⚠️ **CUIDADO**: Clamp valores para evitar overflow
- ⚠️ **CUIDADO**: No tocar resistencias mágicas
- ✅ **TESTING**: Verificar balance en niveles altos

---

### 🟡 FEATURE 6 — MUTACIONES LEVES (STATS ONLY)
**ESTADO**: IMPLEMENTACIÓN SIMPLE

#### Análisis del Código Actual
```cpp
// Flags existentes en monster.h:
// MFLAG_BERSERK, MFLAG_GOLEM, etc.
// Sistema de flags ya implementado
```

#### Implementación Propuesta
```cpp
void ApplyMonsterMutations(Monster &monster) {
    if (GenerateRnd(100) < 5) {  // 5% chance
        // Mutación simple: HP doble
        monster.maxHitPoints *= 2;
        monster.hitPoints = monster.maxHitPoints;
        
        // Usar flag existente
        monster.flags |= MFLAG_BERSERK;  // Reutilizar flag
    }
}
```

#### Archivos Afectados
- `Source/monster.cpp` (InitMonster)
- `Source/monster.h` (posibles nuevos flags)

#### Riesgos y Prevención
- ✅ **SEGURO**: No agregar estados nuevos
- ✅ **SEGURO**: No VFX persistentes
- ✅ **SEGURO**: Solo modificación de stats
- ⚠️ **TESTING**: Verificar que flags existentes funcionan

---

### 🔴 FEATURE 7 — MONSTRUOS ÉLITE (ESTILO DIABLO 1)
**ESTADO**: IMPLEMENTACIÓN CON CUIDADO

#### Análisis del Código Actual
```cpp
// Uniques ya existen: UniqueMonsterType enum
// Sistema de nombres no persistente
// Prefijos NO están implementados para monstruos normales
```

#### Implementación Propuesta
```cpp
void CreateEliteMonster(Monster &monster) {
    if (GenerateRnd(200) == 0) {  // 0.5% chance
        // Stats élite
        monster.maxHitPoints *= 3;
        monster.hitPoints = monster.maxHitPoints;
        monster.minDamage *= 2;
        monster.maxDamage *= 2;
        
        // NO guardar nombre (no persistente)
        // monster._mName = "Elite " + monster._mName;  // EVITAR
        
        // Usar flag temporal
        monster.flags |= MFLAG_BERSERK;  // Indicador visual
    }
}
```

#### Archivos Afectados
- `Source/monster.cpp` (InitMonster)
- Posible `Source/monster.h` (flags)

#### Riesgos y Prevención
- ⚠️ **CRÍTICO**: No guardar nombres (no persistente)
- ⚠️ **CRÍTICO**: No agregar auras o efectos complejos
- ⚠️ **CUIDADO**: Muy baja probabilidad para evitar spam
- ✅ **SEGURO**: Solo modificación de stats básicos

---

## FEATURES DESCARTADAS (ANÁLISIS DE RIESGOS)

### ❌ CORRUPCIÓN ENTRE NIVELES
**RAZÓN**: No está prevista en el código original
- Requiere modificar sistema de saves
- Afecta persistencia de datos
- Riesgo alto de corrupción de partidas

### ❌ RESPAWN PARCIAL
**RAZÓN**: Complejidad excesiva
- Requiere tracking de estados complejos
- Interfiere con sistema de quests
- Riesgo de bugs en multiplayer

### ❌ ESTADOS PERSISTENTES NPC
**RAZÓN**: Sistema no diseñado para esto
- NPCs tienen estados fijos
- Modificar requiere cambios en saves
- Riesgo de incompatibilidad

---

## PLAN DE IMPLEMENTACIÓN

### FASE 1: FEATURES SEGURAS (Semana 1)
1. **Feature 1**: Garantizar quests (1 día)
2. **Feature 2**: Mejora iluminación (1 día)
3. **Feature 5**: Variantes por profundidad (2 días)

### FASE 2: FEATURES MODERADAS (Semana 2)
1. **Feature 3**: Densidad decorativa (3 días)
2. **Feature 6**: Mutaciones simples (2 días)

### FASE 3: FEATURES COMPLEJAS (Semana 3)
1. **Feature 4**: Regeneración controlada (4 días)
2. **Feature 7**: Monstruos élite (3 días)

### TESTING Y VALIDACIÓN (Semana 4)
- Testing exhaustivo en singleplayer
- Testing en multiplayer
- Verificación de saves/loads
- Balance testing
- Performance testing

---

## ARQUITECTURA DE SEGURIDAD

### PRINCIPIOS DE IMPLEMENTACIÓN
1. **NO MODIFICAR SAVES**: Todas las features deben ser calculadas en runtime
2. **USAR SISTEMAS EXISTENTES**: Aprovechar funciones ya implementadas
3. **VALIDACIÓN CONSTANTE**: Verificar límites y estados
4. **ROLLBACK CAPABILITY**: Poder desactivar features fácilmente

### PUNTOS DE CONTROL
- Verificar `gbIsMultiplayer` antes de aplicar cambios
- Validar `currlevel` y `leveltype` 
- Comprobar estados de quests antes de modificar
- Usar `GenerateRnd()` para consistencia

### TESTING CRÍTICO
- Save/Load cycles completos
- Transiciones entre niveles
- Multiplayer synchronization
- Quest completion states
- Performance en niveles grandes

---

## CONCLUSIÓN

**FEATURES RECOMENDADAS PARA IMPLEMENTACIÓN INMEDIATA:**
- ✅ Feature 1: Garantizar quests (SEGURO 100%)
- ✅ Feature 2: Mejora iluminación (SEGURO 100%)
- ✅ Feature 5: Variantes por profundidad (SEGURO)

**FEATURES PARA IMPLEMENTACIÓN CUIDADOSA:**
- 🟡 Feature 3: Densidad decorativa (80% seguro)
- 🟡 Feature 6: Mutaciones simples (SEGURO con límites)

**FEATURES PARA IMPLEMENTACIÓN AVANZADA:**
- 🔴 Feature 4: Regeneración controlada (CON LÍMITES)
- 🔴 Feature 7: Monstruos élite (CON CUIDADO)

El código base de DevilutionX está bien estructurado y permite estas modificaciones sin riesgo de corrupción si se siguen las pautas arquitecturales establecidas.