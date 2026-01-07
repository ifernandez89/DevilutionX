# DevilutionX - Roadmap de Implementación de Features

## 📋 Análisis del Estado Actual

**Fecha**: Enero 2026  
**Base**: DevilutionX original + Muerte de Diablo sin cinemática  
**Objetivo**: Implementar mejoras de gameplay manteniendo la esencia original  

## 🎯 Features Identificadas para Implementación

### **✅ COMPLETADAS**
1. **Muerte de Diablo sin Cinemática** - Implementado
2. **Intercambio de Posiciones NPCs** - Implementado

### **🔄 PENDIENTES DE IMPLEMENTACIÓN**

---

## 📊 Clasificación por Complejidad y Riesgo

### **🟢 NIVEL 1: BAJO RIESGO - CONFIGURACIÓN SIMPLE**
*Modificaciones de datos TSV y configuraciones sin tocar lógica de engine*

#### **1.1 Garantizar Todas las Quests Siempre Disponibles**
- **Complejidad**: ⭐ (Muy Baja)
- **Riesgo de Bugs**: ⭐ (Muy Bajo)
- **Archivos a Modificar**: `Source/quests.cpp`
- **Descripción**: Asegurar que todas las quests aparezcan en cada partida

**Código Necesario:**
```cpp
// En Source/quests.cpp - función InitQuests()
void InitQuests()
{
    // Forzar todas las quests como disponibles en vanilla Diablo
    if (!gbIsHellfire) {
        Quests[Q_ROCK]._qactive = QUEST_INIT;
        Quests[Q_MUSHROOM]._qactive = QUEST_INIT;
        Quests[Q_GARBUD]._qactive = QUEST_INIT;
        Quests[Q_ZHAR]._qactive = QUEST_INIT;
        Quests[Q_VEIL]._qactive = QUEST_INIT;
        Quests[Q_DIABLO]._qactive = QUEST_INIT;
        Quests[Q_BUTCHER]._qactive = QUEST_INIT;
        Quests[Q_LTBANNER]._qactive = QUEST_INIT;
        Quests[Q_BLIND]._qactive = QUEST_INIT;
        Quests[Q_BLOOD]._qactive = QUEST_INIT;
        Quests[Q_ANVIL]._qactive = QUEST_INIT;
        Quests[Q_WARLORD]._qactive = QUEST_INIT;
        Quests[Q_SKELKING]._qactive = QUEST_INIT;
        Quests[Q_PWATER]._qactive = QUEST_INIT;
        Quests[Q_SCHAMB]._qactive = QUEST_INIT;
        Quests[Q_BETRAYER]._qactive = QUEST_INIT;
    }
}
```

**Sistemas que NO Tocar:**
- Lógica de generación de niveles
- Sistema de saves
- Networking

---

#### **1.2 Mejora de Iluminación del Jugador**
- **Complejidad**: ⭐ (Muy Baja)
- **Riesgo de Bugs**: ⭐ (Muy Bajo)
- **Archivos a Modificar**: `Source/player.cpp`
- **Descripción**: Aumentar el radio de luz del jugador para mejor visibilidad

**Código Necesario:**
```cpp
// En Source/player.cpp - función que maneja la luz del jugador
void UpdatePlayerLight(Player &player)
{
    if (!gbIsHellfire) {
        // Aumentar radio de luz base del jugador en 40%
        int baseRadius = player._pLightRad;
        int enhancedRadius = baseRadius + (baseRadius * 40 / 100);
        player._pLightRad = std::min(enhancedRadius, 15); // Cap máximo
    }
}
```

---

### **🟡 NIVEL 2: RIESGO MEDIO - MODIFICACIONES DE GAMEPLAY**
*Cambios en lógica de juego que requieren testing cuidadoso*

#### **2.1 Aumento de Densidad de Elementos Decorativos**
- **Complejidad**: ⭐⭐ (Baja-Media)
- **Riesgo de Bugs**: ⭐⭐ (Bajo-Medio)
- **Archivos a Modificar**: `Source/levels/drlg_l1.cpp`, `drlg_l2.cpp`, `drlg_l3.cpp`, `drlg_l4.cpp`
- **Descripción**: Duplicar la densidad de antorchas, columnas, librerías sin nuevos assets

**Código Necesario:**
```cpp
// En Source/levels/drlg_l1.cpp - función de colocación de objetos decorativos
void PlaceDecorations()
{
    if (!gbIsHellfire) {
        // Duplicar probabilidad de elementos decorativos
        for (int j = 0; j < DMAXY; j++) {
            for (int i = 0; i < DMAXX; i++) {
                if (dungeon[i][j] == FLOOR_TYPE) {
                    // Probabilidad original * 2 para antorchas
                    if (GenerateRnd(100) < 20) { // Era 10
                        PlaceTorch(i, j);
                    }
                    // Probabilidad original * 2 para columnas
                    if (GenerateRnd(100) < 8) { // Era 4
                        PlaceColumn(i, j);
                    }
                }
            }
        }
    }
}
```

**Sistemas que NO Tocar:**
- Generación base de niveles
- Pathfinding
- Colisiones

---

#### **2.2 Sistema de Regeneración de Monstruos**
- **Complejidad**: ⭐⭐ (Baja-Media)
- **Riesgo de Bugs**: ⭐⭐ (Bajo-Medio)
- **Archivos a Modificar**: `Source/diablo.cpp`
- **Descripción**: Regenerar monstruos al re-entrar niveles si Diablo no ha sido derrotado

**Código Necesario:**
```cpp
// En Source/diablo.cpp - función LoadGameLevel
void LoadGameLevel(bool firstflag, lvl_entry lvldir)
{
    if (!gbIsHellfire && !firstflag && 
        Quests[Q_DIABLO]._qactive != QUEST_DONE &&
        !gbIsMultiplayer) {
        
        // Forzar regeneración de monstruos
        InitMonsters();
        InitItems();
    }
}
```

---

### **🟠 NIVEL 3: RIESGO ALTO - SISTEMAS COMPLEJOS**
*Modificaciones que afectan múltiples sistemas*

#### **3.1 Sistema de Monstruos Élite**
- **Complejidad**: ⭐⭐⭐ (Media-Alta)
- **Riesgo de Bugs**: ⭐⭐⭐ (Medio-Alto)
- **Archivos a Modificar**: `Source/monster.h`, `Source/monster.cpp`, `Source/items.cpp`
- **Descripción**: Añadir tipos de monstruos élite con mejor IA y loot

**Código Necesario:**
```cpp
// En Source/monster.h - Añadir flags élite
enum MonsterFlag : uint16_t {
    // ... flags existentes
    MFLAG_ELITE_PACK = 1 << 13,
    MFLAG_ELITE_GUARDIAN = 1 << 14,
    MFLAG_ELITE_SUBBOSS = 1 << 15,
};

// En Source/monster.cpp - Funciones élite
void MakeElitePack(Monster &leader, int packSize)
{
    if (!gbIsHellfire && currlevel >= 5) {
        leader.flags |= MFLAG_ELITE_PACK;
        leader.hitPoints *= 2;
        leader.maxHitPoints *= 2;
        // Mejorar IA y stats
    }
}

bool Monster::isElite() const
{
    return (flags & (MFLAG_ELITE_PACK | MFLAG_ELITE_GUARDIAN | MFLAG_ELITE_SUBBOSS)) != 0;
}
```

**Sistemas que NO Tocar:**
- Formato de saves
- Networking de monstruos
- Rendering de monstruos

---

#### **3.2 Hechizos Restaurados (Doom Serpents, Blood Ritual, etc.)**
- **Complejidad**: ⭐⭐⭐⭐ (Alta)
- **Riesgo de Bugs**: ⭐⭐⭐ (Medio-Alto)
- **Archivos a Modificar**: `Source/spells.cpp`, `Source/missiles.cpp`, `assets/txtdata/spells/`
- **Descripción**: Implementar hechizos que estaban en desarrollo

**Código Necesario:**
```cpp
// En Source/spells.cpp - Añadir nuevos hechizos
void AddDoomSerpents(Player &player, Point target)
{
    if (!gbIsHellfire) {
        // Implementar lógica de Doom Serpents
        for (int i = 0; i < 3; i++) {
            AddMissile(player.position.tile, target, 
                      Direction::South, MissileID::DoomSerpent, 
                      TARGET_MONSTERS, player, 0, 0);
        }
    }
}
```

---

### **🔴 NIVEL 4: RIESGO CRÍTICO - SISTEMAS CORE**
*Modificaciones que afectan sistemas fundamentales*

#### **4.1 Expansión de Quest Poisoned Water**
- **Complejidad**: ⭐⭐⭐⭐⭐ (Muy Alta)
- **Riesgo de Bugs**: ⭐⭐⭐⭐ (Alto)
- **Archivos a Modificar**: `Source/quests.cpp`, `Source/levels/drlg_l3.cpp`
- **Descripción**: Implementar la quest en cave level en lugar de setlevel

**Sistemas que NO Tocar:**
- Sistema de setlevels
- Generación base de caves
- Save format

---

## 🗺️ Plan de Implementación Recomendado

### **FASE 1: Fundamentos (Semana 1)**
1. ✅ Muerte de Diablo sin cinemática (COMPLETADO)
2. ✅ Intercambio NPCs (COMPLETADO)
3. 🔄 Garantizar todas las quests
4. 🔄 Mejora de iluminación del jugador

### **FASE 2: Mejoras Visuales (Semana 2)**
5. 🔄 Aumento de densidad decorativa
6. 🔄 Sistema de regeneración de monstruos

### **FASE 3: Sistemas Avanzados (Semana 3-4)**
7. 🔄 Sistema de monstruos élite
8. 🔄 Hechizos restaurados básicos

### **FASE 4: Features Complejas (Semana 5+)**
9. 🔄 Expansión Poisoned Water Quest
10. 🔄 Sistemas adicionales según testing

---

## 🛡️ Principios de Seguridad

### **NUNCA Modificar:**
- **Formato de saves** - Mantener compatibilidad total
- **Networking core** - Preservar multiplayer
- **Rendering engine** - Evitar crashes gráficos
- **Input systems** - Mantener controles estables
- **Memory management** - No introducir leaks

### **SIEMPRE Verificar:**
- **Compatibilidad Hellfire** - Usar `!gbIsHellfire`
- **Multiplayer** - Usar `!gbIsMultiplayer` cuando sea apropiado
- **Bounds checking** - Validar arrays y punteros
- **Performance** - No degradar FPS

### **Testing Obligatorio:**
- **Single Player** - Funcionalidad completa
- **Multiplayer** - Comportamiento original
- **Hellfire** - Sin cambios
- **Saves** - Carga/guardado correcto

---

## 📋 Checklist de Implementación

### **Para Cada Feature:**
- [ ] Análisis de impacto en sistemas existentes
- [ ] Implementación con guards de compatibilidad
- [ ] Testing en single player
- [ ] Testing en multiplayer
- [ ] Testing en Hellfire
- [ ] Verificación de saves
- [ ] Documentación de cambios
- [ ] Commit con descripción clara

---

## 🎯 Próximo Paso Recomendado

**COMENZAR CON: Garantizar Todas las Quests**

**Razones:**
1. **Riesgo mínimo** - Solo modifica inicialización
2. **Impacto inmediato** - Mejora notable en gameplay
3. **Testing simple** - Fácil de verificar
4. **Base sólida** - Para features más complejas

**Implementación estimada: 30 minutos**
**Testing estimado: 15 minutos**

---

*Este roadmap prioriza la estabilidad y compatibilidad mientras añade mejoras significativas al gameplay de Diablo vanilla.*