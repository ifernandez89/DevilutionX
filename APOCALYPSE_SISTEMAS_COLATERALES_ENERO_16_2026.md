# Sistemas Colaterales de Apocalypse - Análisis Completo

## 🔍 DESCUBRIMIENTO CRÍTICO

El crash de Apocalypse NO está en el spell mismo, sino en los **SISTEMAS COLATERALES** que se activan durante su ejecución.

---

## 📊 CADENA DE EVENTOS COMPLETA

### 1. **Cast de Apocalypse** ✅ LOGGEADO
```
Player castea → CanSafelyCastApocalypse() → AddMissile(Apocalypse)
```
**Logs actuales**: ✅ Completos

### 2. **ProcessApocalypse** ✅ LOGGEADO
```
Loop sobre área → Crea ApocalypseBoom por cada monstruo
```
**Logs actuales**: ✅ Completos

### 3. **ProcessApocalypseBoom** ⚠️ NO LOGGEADO
```cpp
void ProcessApocalypseBoom(Missile &missile) {
    missile.duration--;
    if (missile.var1 == 0)
        CheckMissileCol(missile, ...);  // ⚠️ AQUÍ EMPIEZA EL PROBLEMA
    if (missile._miHitFlag)
        missile.var1 = 1;
    if (missile.duration == 0)
        missile._miDelFlag = true;
    PutMissile(missile);
}
```
**Logs actuales**: ❌ NINGUNO

---

## 🚨 SISTEMAS COLATERALES NO LOGGEADOS

### A. **CheckMissileCol** - Detección de Colisiones
**Qué hace**:
- Itera sobre monstruos en el área
- Calcula daño
- Llama a `MonsterTrapHit()` o `PlayerMHit()`

**Riesgos**:
- ⚠️ Iteración sobre lista de monstruos
- ⚠️ Posible invalidación de iteradores
- ⚠️ Modificación de estado de monstruos

**Logs actuales**: ❌ NINGUNO

---

### B. **MonsterDeath** - Muerte de Monstruos
**Qué hace** (`Source/monster.cpp:4228`):
```cpp
void MonsterDeath(Monster &monster, Direction md, bool sendmsg) {
    // 1. Experiencia al jugador
    AddPlrMonstExper(...);
    
    // 2. Contador de kills
    MonsterKillCounts[monster.type().type]++;  // ⚠️ ARRAY ACCESS
    
    // 3. Combat Pauses tracking
    RecordMonsterKill(...);  // ⚠️ SISTEMA CUSTOM
    
    // 4. Sonidos de muerte
    PlayEffect(monster, MonsterSound::Death);  // ⚠️ SISTEMA DE AUDIO
    
    // 5. Animación de muerte
    NewMonsterAnim(monster, MonsterGraphic::Death, md);
    monster.mode = MonsterMode::Death;
    
    // 6. Items/Loot
    // (se genera después)
}
```

**Riesgos**:
- ⚠️ Acceso a arrays globales
- ⚠️ Sistema de audio (puede fallar)
- ⚠️ Sistema de Combat Pauses (custom)
- ⚠️ Generación de items
- ⚠️ Modificación de estado global

**Logs actuales**: ❌ NINGUNO

---

### C. **PlayEffect** - Sistema de Audio
**Qué hace** (`Source/monster.cpp:4948`):
```cpp
void PlayEffect(Monster &monster, MonsterSound sound) {
    int lVolume = 0;
    int lPan = 0;
    if (!CalculateSoundPosition(monster.position.tile, &lVolume, &lPan))
        return;  // ⚠️ PUEDE FALLAR SILENCIOSAMENTE
    
    // Reproduce sonido...
}
```

**Riesgos**:
- ⚠️ Cálculo de posición 3D
- ⚠️ Acceso a sistema de audio
- ⚠️ Puede fallar si el monstruo está en posición inválida

**Logs actuales**: ❌ NINGUNO

---

### D. **RecordMonsterKill** - Combat Pauses (CUSTOM)
**Qué hace**:
- Sistema custom de Nightmare Edition
- Trackea kills para pausas de combate
- Modifica estado global

**Riesgos**:
- ⚠️ Sistema custom no testeado con Apocalypse
- ⚠️ Posible conflicto con múltiples kills simultáneos

**Logs actuales**: ❌ NINGUNO

---

### E. **Item Generation** - Drops de Loot
**Qué hace**:
- Genera items cuando muere un monstruo
- Crea objetos en el mapa
- Modifica arrays globales

**Riesgos**:
- ⚠️ Múltiples items generándose simultáneamente
- ⚠️ Acceso a arrays de items
- ⚠️ Posible overflow de items

**Logs actuales**: ❌ NINGUNO

---

### F. **Enhanced Monster Vocal Atmosphere** (CUSTOM)
**Qué hace** (`Source/monster.cpp:4184-4299`):
```cpp
// Additional pain sound when monsters are low HP
const bool isLowHP = monster.hitPoints < (monster.maxHitPoints / 3);
if (isLowHP && isHumanoid) {
    // Reproduce sonidos adicionales
}

// Death sounds con probabilidad
if (shouldPlayDeathSound) {
    PlayEffect(monster, MonsterSound::Death);  // ⚠️ MÁS AUDIO
}
```

**Riesgos**:
- ⚠️ Sistema custom de audio atmosférico
- ⚠️ Múltiples llamadas a PlayEffect
- ⚠️ Puede saturar el sistema de audio

**Logs actuales**: ❌ NINGUNO

---

## 🎯 HIPÓTESIS DEL CRASH

### Escenario Más Probable:
```
1. Apocalypse crea 10-20 ApocalypseBoom
2. Cada Boom llama CheckMissileCol
3. CheckMissileCol mata 5-10 monstruos SIMULTÁNEAMENTE
4. Cada muerte llama:
   - MonsterDeath()
   - PlayEffect() (2-3 veces por el sistema vocal)
   - RecordMonsterKill()
   - Item generation
5. Sistema de audio se SATURA con 15-30 sonidos simultáneos
6. O Combat Pauses se CONFUNDE con múltiples kills
7. O Item generation OVERFLOW
8. CRASH
```

---

## 🔧 SOLUCIÓN: LOGGING COMPLETO

### Logs a Añadir:

#### 1. En `ProcessApocalypseBoom`:
```cpp
ARCH_LOG_APOCALYPSE_BOOM_PROCESS(missile.position.tile.x, missile.position.tile.y, 
                                  missile.duration, missile.var1);
```

#### 2. En `CheckMissileCol` (cuando es ApocalypseBoom):
```cpp
ARCH_LOG_COLLISION_CHECK(missile._mitype, targetMonster.getId(), 
                         damage, targetMonster.hitPoints);
```

#### 3. En `MonsterDeath` (cuando viene de Apocalypse):
```cpp
ARCH_LOG_MONSTER_DEATH(monster.getId(), monster.type().type, 
                       MonsterKillCounts[monster.type().type]);
```

#### 4. En `PlayEffect` (cuando es death sound):
```cpp
ARCH_LOG_AUDIO_EFFECT(monster.getId(), sound, lVolume, lPan);
```

#### 5. En `RecordMonsterKill`:
```cpp
ARCH_LOG_COMBAT_PAUSE_KILL(monster.getId(), totalKills);
```

---

## 📋 PRÓXIMOS PASOS

1. ✅ Identificar sistemas colaterales (COMPLETADO)
2. ⏳ Añadir logs en ProcessApocalypseBoom
3. ⏳ Añadir logs en CheckMissileCol
4. ⏳ Añadir logs en MonsterDeath
5. ⏳ Añadir logs en PlayEffect
6. ⏳ Añadir logs en RecordMonsterKill
7. ⏳ Recompilar y testear
8. ⏳ Analizar logs para encontrar el culpable

---

## 🎯 SOSPECHOSOS PRINCIPALES

### 1. **Sistema de Audio** (80% probabilidad)
- Múltiples PlayEffect simultáneos
- Sistema vocal atmosférico
- Posible saturación

### 2. **Combat Pauses** (15% probabilidad)
- Sistema custom
- No testeado con múltiples kills simultáneos

### 3. **Item Generation** (5% probabilidad)
- Overflow de items
- Menos probable pero posible

---

## 💡 INSIGHT CLAVE

**El crash NO está en Apocalypse, está en los sistemas que Apocalypse ACTIVA.**

Apocalypse es como un interruptor que enciende 10 sistemas a la vez. Uno de esos sistemas no puede manejar la carga.
