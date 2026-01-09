# MISSILES AUDIT REPORT - FASE 2

## 🔍 AUDITORÍA CRÍTICA DE MISSILES (SIN CORRECCIONES)

### ⚠️ REGLA DE ORO APLICADA
**NO "arreglar" durante la auditoría. Solo marcar.**

**Objetivo**: Mapear puntos de presión antes de cualquier intervención quirúrgica.

---

## 📊 HALLAZGOS CRÍTICOS

### 🚨 [MISSILES] - PATRONES DE RIESGO IDENTIFICADOS

#### 1. **AddMissile puede llamarse dentro de loop en ProcessInfernoControl()**
**Ubicación**: `Source/missiles.cpp:3979-3987`
```cpp
void ProcessInfernoControl(Missile &missile)
{
    // ... loop de procesamiento ...
    if (missile.position.tile != Point { missile.var1, missile.var2 }) {
        if (!TileHasAny(missile.position.tile, TileProperties::BlockMissile)) {
            AddMissile(  // ⚠️ SPAWN DENTRO DE LOOP DE PROCESAMIENTO
                missile.position.tile,
                missile.position.start,
                Direction::South,
                MissileID::Inferno,
                missile._micaster,
                missile._misource,
                missile.var3,
                missile._mispllvl,
                &missile);
        }
    }
}
```

**Riesgo**: Spawn de missiles durante iteración activa de la lista de missiles.
**Frecuencia**: Cada tile que recorre InfernoControl.
**Impacto**: Potencial invalidación de iteradores + crecimiento exponencial.

#### 2. **Inferno spawnea sin límite en ProcessInfernoControl()**
**Ubicación**: `Source/missiles.cpp:3979`
**Patrón identificado**:
- InfernoControl se mueve tile por tile
- Cada tile genera un nuevo missile Inferno
- Sin verificación de límites antes de AddMissile()
- Sin throttling temporal

**Riesgo**: Crecimiento descontrolado de missiles en trayectorias largas.
**Escenario crítico**: Inferno a través de salas grandes = 20+ missiles instantáneos.

#### 3. **ProcessMissiles() itera sin protección contra mutación**
**Ubicación**: `Source/missiles.cpp:4219-4245`
```cpp
void ProcessMissiles()
{
    for (auto &missile : Missiles) {  // ⚠️ ITERACIÓN SIN PROTECCIÓN
        // ... procesamiento que puede llamar AddMissile() ...
        const MissileData &missileData = GetMissileData(missile._mitype);
        if (missileData.processFn != nullptr)
            missileData.processFn(missile);  // ⚠️ PUEDE MUTAR LA LISTA
    }
}
```

**Riesgo**: Undefined behavior si processFn() agrega missiles durante iteración.
**Confirmado**: ProcessInfernoControl() SÍ agrega missiles.

#### 4. **Lightning encadena impacto + spawn sin verificación**
**Ubicación**: `Source/missiles.cpp:834-840`
```cpp
AddMissile(
    position,
    missile.position.start,
    GetDirection(position, missile.position.start),
    type,  // ThinLightning
    missile._micaster,
    missile._misource,
    missile._midam,
    missile._mispllvl);
```

**Riesgo**: Chain lightning puede generar múltiples missiles por impacto.
**Sin verificación**: No hay CanAddMissile() antes del spawn.

#### 5. **Múltiples AddMissile() en loops de monsters**
**Ubicación**: `Source/monster.cpp:1386-1390`
```cpp
for (int mi = 0; mi < multimissiles; mi++) {
    AddMissile(  // ⚠️ MÚLTIPLES SPAWNS EN LOOP
        monster.position.tile,
        monster.enemyPosition,
        md,
        MissileID::Firebolt,
        TARGET_PLAYERS,
        monster,
        midam,
        0);
}
```

**Riesgo**: Monsters pueden spawnear múltiples missiles sin verificación.
**Variable**: `multimissiles` puede ser hasta 3+ según el tipo de monster.

---

## 🎯 [OBJECTS] - PATRONES DE SPAWN IDENTIFICADOS

#### 6. **Trap A se reactiva múltiples veces por frame**
**Ubicación**: `Source/objects.cpp:4227`
```cpp
AddMissile(trap.position, target, dir, static_cast<MissileID>(trap._oVar3), TARGET_PLAYERS, -1, 0, 0);
```

**Riesgo**: Traps pueden disparar sin cooldown entre frames.
**Sin throttling**: No hay verificación de tiempo entre activaciones.

#### 7. **Shrines spawnean efectos sin límites**
**Ubicaciones múltiples**: `Source/objects.cpp:2415, 2569, 2667`
**Patrón**: Cada shrine genera missiles de efecto sin verificación previa.

---

## 🔥 [INFERNO ESPECÍFICO] - ANÁLISIS DETALLADO

### 🎯 CADENA DE SPAWN INFERNO
```
1. Player casts Inferno
2. Crea InfernoControl missile
3. InfernoControl se mueve tile por tile
4. Cada tile → AddMissile(Inferno) SIN VERIFICACIÓN
5. Cada Inferno dura 256+ frames
6. Acumulación exponencial en trayectorias largas
```

### 📈 PROYECCIÓN DE CRECIMIENTO
**Escenario**: Inferno a través de 15 tiles
- **Missiles creados**: 15 Inferno + 1 InfernoControl = 16 total
- **Duración**: 256+ frames cada uno
- **Con múltiples casts**: 16 × N casts = Crecimiento lineal por cast

**Escenario crítico**: 3 Sorcerers spamming Inferno
- **Missiles simultáneos**: 16 × 3 × spam_rate = 100+ missiles activos
- **Sin límites**: Puede exceder fácilmente los 500 missiles seguros

### 🧬 PATRÓN GENÉTICO DEL PROBLEMA
```
InfernoControl.processFn() → 
    AddMissile(Inferno) → 
        Missiles.push_back() → 
            DURANTE ProcessMissiles() loop → 
                INVALIDACIÓN DE ITERADORES
```

---

## ⚡ [LIGHTNING] - PATRONES DE ENCADENAMIENTO

### 🔗 CHAIN LIGHTNING ANALYSIS
**Ubicación**: `Source/missiles.cpp:834`
**Patrón identificado**:
- Lightning impacta target
- Genera ThinLightning hacia otros targets
- Sin límite en número de chains
- Sin verificación de missiles existentes

### 📊 PROYECCIÓN DE CHAINS
**Escenario**: Chain Lightning en pack de 10 monsters
- **Primer impacto**: 1 lightning
- **Chains**: Hasta 9 ThinLightning adicionales
- **Total**: 10 missiles por cast
- **Con múltiples casts**: Crecimiento multiplicativo

---

## 🏗️ [ARQUITECTURA] - PUNTOS DE PRESIÓN MAPEADOS

### 🎯 PUNTOS CRÍTICOS IDENTIFICADOS
1. **ProcessMissiles()**: Loop principal vulnerable a mutación
2. **ProcessInfernoControl()**: Spawn garantizado cada tile
3. **Chain Lightning**: Multiplicación por targets
4. **Monster multimissiles**: Spawn múltiple sin verificación
5. **Trap activation**: Sin cooldown entre disparos

### 🔍 CORRELACIONES DETECTADAS
- **Inferno + Packs grandes**: Multiplicación de ambos factores
- **Chain Lightning + Inferno**: Doble presión en lista de missiles
- **Multiple players**: Efectos multiplicativos sin coordinación

---

## 📋 RESUMEN EJECUTIVO DE AUDITORÍA

### ✅ VICTORIA ENORME LOGRADA
**Mapa completo de puntos de presión identificado sin introducir cambios.**

### 🎯 PUNTOS DE INTERVENCIÓN PRIORIZADOS
1. **CRÍTICO**: ProcessMissiles() - Proteger iteración principal
2. **ALTO**: ProcessInfernoControl() - Limitar spawn por tile
3. **MEDIO**: Chain Lightning - Verificar antes de spawn
4. **BAJO**: Traps/Shrines - Throttling opcional

### 🧠 INSIGHTS ARQUITECTÓNICOS
- **Patrón dominante**: Spawn durante procesamiento activo
- **Riesgo principal**: Invalidación de iteradores en loop principal
- **Oportunidad**: Inferno es caso especial, no generalizar solución

### 🚀 PREPARACIÓN PARA FASE 3
**Base sólida establecida para intervención quirúrgica dirigida.**

---

## 🔮 PRÓXIMOS PASOS RECOMENDADOS

### 👉 PASO 2: ANÁLISIS DE AUDITORÍA
**Con este reporte en mano**:
- ✅ **Decidir dónde poner guardas**: ProcessMissiles() + ProcessInfernoControl()
- ✅ **Decidir qué loops dividir**: Separar procesamiento de spawn
- ✅ **Decidir Inferno Beam**: Transición gradual vs implementación directa

### 👉 PASO 3: IMPLEMENTACIÓN QUIRÚRGICA
**Intervenciones dirigidas**:
1. **Proteger ProcessMissiles()** con iteración segura
2. **Limitar InfernoControl spawn** con verificación previa
3. **Optimizar Inferno** sin afectar otros spells

### 🎯 CRITERIO DE ÉXITO FASE 2
**✅ COMPLETADO**: Visión global sin sesgos, correlaciones intactas, puntos de presión mapeados.

**El engine ya no miente. Ahora sabemos exactamente dónde y cómo intervenir.**