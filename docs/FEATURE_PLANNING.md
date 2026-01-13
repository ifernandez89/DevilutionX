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
        mons.idablecles estauitecturapautas arq siguen las si serupción  corin riesgo decaciones sas modifiestpermite  y adoructurien estionX está bvilut de Deaseo bdigcóDADO)

El te (CON CUIruos élistature 7: MonTES)
- 🔴 Fe(CON LÍMIa  controladRegeneracióne 4: eatur 🔴 F
-AVANZADA:**EMENTACIÓN S PARA IMPLURE*FEATmites)

* líURO conles (SEGes simp 6: Mutacioneatureo)
- 🟡 F(80% segurecorativa idad d 3: Denseature
- 🟡 FDADOSA:**ENTACIÓN CUIA IMPLEMFEATURES PARRO)

**idad (SEGUpor profundtes rian 5: VaFeature 100%)
- ✅ (SEGUROón naciumiora ilre 2: Mej
- ✅ FeatuSEGURO 100%)ts (esquizar antature 1: Gar Fe:**
- ✅TANMEDIA ITACIÓNEMENMPLAS PARA IECOMENDADURES R

**FEATONCLUSIÓN

## C

---ess grandnivelermance en 
- Perfotates sompletion
- Quest conizationsynchrultiplayer s
- Mniveleentre iciones 
- Transes completosoad cyclve/L SaCRÍTICO
-TING # TEScia

##ennsista coeRnd()` pareneratsar `Gdificar
- Untes de moests aados de qurobar estmpype` 
- Col` y `leveltleve `curralidarbios
- V cam de aplicartesiplayer` an `gbIsMultar VerificONTROL
-NTOS DE CPU## 

# fácilmenteures featactivaresY**: Poder dBILITOLLBACK CAPA4. **Restados
 y mitesificar líVer: TE**NSTANÓN COIDACI
3. **VALmentadases ya implear funcionch AproveSTENTES**:ISTEMAS EXI **USAR Sruntime
2.n culadas en ser caldebetures  las fea**: TodasFICAR SAVES. **NO MODICIÓN
1 IMPLEMENTANCIPIOS DE

### PRIGURIDADCTURA DE SE
## ARQUITE--
ng

-ce testimanng
- Perfortitesance s
- Balves/load saón deificaci- Verltiplayer
muesting en  T
-ngleplayersien o ng exhaustiv Testimana 4)
-IÓN (SeALIDAC VTESTING Y

###  (3 días)itetruos élonsure 7**: M2. **Feat(4 días)
lada ntron cocióegeneraure 4**: R
1. **FeatSemana 3) COMPLEJAS ( 3: FEATURES FASEdías)

###s (2 ones simple*: Mutaciure 6*. **Featdías)
2tiva (3 sidad decoraen**: Dture 3 **Fea 2)
1.anaS (SemMODERADAS  FEATURE
### FASE 2:(2 días)
ad or profundidiantes par Vature 5**:*Fe1 día)
3. *nación (umi: Mejora ileature 2**
2. **Fs (1 día)ar questGarantiz 1**: 1. **Featurena 1)
GURAS (Sema SETURESFEA# FASE 1: ACIÓN

##E IMPLEMENTAN D

## PL
---dad
li incompatibiRiesgo de- s en saves
ambiorequiere cr odificajos
- Mfin estados - NPCs tiene
sto eparadiseñado no istema *RAZÓN**: SNTES NPC
*ERSISTESTADOS P

### ❌ Eplayerlti en mugssgo de buuests
- Riema de qre con sisteInterfiemplejos
- ados cong de esttrackiequiere iva
- Rexcesjidad mple: CoÓN**AL
**RAZCIARSPAWN P RE### ❌artidas

 de pupciónto de corr Riesgo alatos
-ia de da persistencects
- Afvee sama dcar sistefiere modi Requiriginal
- oódigoa en el cstrevio está pAZÓN**: NVELES
**RTRE NIORRUPCIÓN EN
### ❌ C
RIESGOS)IS DE ISAS (ANÁL DESCARTADTURES

## FEA--icos

-stats básn de dificaciólo mo SoRO**: **SEGUpam
- ✅ar sa evitilidad parabob Muy baja pr*:ADO*UID **Clejos
- ⚠️mpco efectos  oregar auras*: No agCRÍTICO*)
- ⚠️ **stentes (no persinombre No guardar TICO**:RÍ**C
- ⚠️  Prevención Riesgos yags)

####ter.h` (flrce/monsble `Sou)
- Posi(InitMonsteronster.cpp` - `Source/mectados
Archivos Af

#### }
```  }
   visual
ndicadorRK;  // IBERSEgs |= MFLAG_monster.fla
        oralag temp fl/ Usar       /     
 TAR
   // EVI;  ter._mName" + mons"Elite e = ster._mNam/ mon        /tente)
siso perr nombre (nguardaNO        //   
 
      Damage *= 2;r.max   monste 2;
     nDamage *=r.mitensmo
        xHitPoints;ter.ma = monstsr.hitPoinmonste     3;
    *= axHitPointsster.m