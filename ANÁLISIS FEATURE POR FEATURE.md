# **ANÁLISIS FEATURE POR FEATURE (UNA POR UNA)**

Voy **solo con las recomendables**, sin repetir críticas.

---

## **🟢 FEATURE 1 — Garantizar todas las quests**

### **📁 Archivos involucrados**

* `quests.cpp`

* `quests.h`

* `init.cpp`

### **🔍 Estado en el código original**

* Muchas quests quedan `QUEST_NOTAVAIL`

* El código YA contempla estados no activados

### **✔ Implementación parcial segura**

👉 **No forzar activación**, solo disponibilidad

`void InitQuests()`  
`{`  
    `for (int i = 0; i < MAXQUESTS; i++) {`  
        `if (quests[i]._qactive == QUEST_NOTAVAIL) {`  
            `quests[i]._qactive = QUEST_INIT;`  
        `}`  
    `}`  
`}`

### **🛡 Prevención de errores**

* No tocar `_qvar1`, `_qlog`, `_qlevel`

* No activar quests especiales (Skeleton King, Lazarus)

### **✅ RECOMENDADA: SÍ (100%)**

---

## **🟢 FEATURE 2 — Mejora de iluminación (soft)**

### **📁 Archivos**

* `player.cpp`

* `lighting.cpp`

* `player.h`

### **🔍 Estado original**

* `_pLightRad` ya es variable

* El sistema soporta radios dinámicos

### **✔ Implementación parcial**

`constexpr int EXTRA_LIGHT = 2;`

`player._pLightRad = std::min(`  
    `player._pLightRad + EXTRA_LIGHT,`  
    `MAX_LIGHT_RADIUS`  
`);`

### **🛡 Prevención**

* No tocar iluminación de monstruos

* No recalcular floodfill por frame

### **✅ RECOMENDADA: SÍ (100%)**

---

## **🟢 FEATURE 3 — Densidad decorativa leve**

### **📁 Archivos**

* `drlg_l1.cpp`

* `drlg_l2.cpp`

* `drlg_l3.cpp`

* `objects.cpp`

### **🔍 Estado original**

* Hay objetos decorativos desactivados

* Ya existen tablas de spawn

### **✔ Implementación parcial**

👉 **Durante generación, NO post-proceso**

`if (Dungeon[x][y] == Floor && RandomInt(100) < 5) {`  
    `AddObject(OBJ_BARREL, x, y);`  
`}`

### **🛡 Prevención**

* Verificar `nSolidTable`

* No tocar niveles quest

* No bloquear puertas

### **✅ RECOMENDADA: SÍ (80%)**

---

## **🟡 FEATURE 4 — Regeneración controlada de monstruos**

### **📁 Archivos**

* `monster.cpp`

* `levels.cpp`

* `quests.cpp`

### **🔍 Estado original**

* `InitMonsters()` ya limpia estados

* Respawn TOTAL ya existe al recargar nivel

### **✔ Implementación parcial segura**

👉 **Solo en niveles sin quests activas**

`if (!IsQuestLevel(currlevel)) {`  
    `InitMonsters();`  
`}`

### **🛡 Prevención**

* Nunca regenerar uniques

* Nunca durante quests

* No persistir estado

### **⚠️ RECOMENDADA: CON LÍMITES**

---

## **🟡 FEATURE 5 — Variantes de monstruos por profundidad**

### **📁 Archivos**

* `monstdat.cpp`

* `monster.cpp`

### **🔍 Estado original**

* Stats ya escalan por nivel

* Tablas ya existen

### **✔ Implementación parcial**

`monster._mHitPoints += currlevel * 5;`  
`monster._mDamageMin += currlevel / 2;`  
`monster._mDamageMax += currlevel / 2;`

### **🛡 Prevención**

* Clamp valores

* No tocar resistencias mágicas

### **✅ RECOMENDADA: SÍ**

---

## **🟡 FEATURE 6 — Mutaciones leves (stats only)**

### **📁 Archivos**

* `monster.cpp`

* `monstdat.cpp`

### **🔍 Estado original**

* Flags como `_mFlags` ya existen

### **✔ Implementación parcial**

`if (RandomInt(100) < 5) {`  
    `monster._mHitPoints *= 2;`  
    `monster._mFlags |= MFLAG_BERSERK;`  
`}`

### **🛡 Prevención**

* No agregar estados nuevos

* No VFX persistentes

### **⚠️ RECOMENDADA: SÍ, simple**

---

## **🟡 FEATURE 7 — Monstruos élite (estilo Diablo 1\)**

### **📁 Archivos**

* `monster.cpp`

* `missiles.cpp`

### **🔍 Estado original**

* Uniques ya existen

* Prefijos NO persistentes posibles

### **✔ Implementación parcial**

`if (RandomInt(200) == 0) {`  
    `monster._mHitPoints *= 3;`  
    `monster._mDamageMin *= 2;`  
    `monster._mName = "Elite " + monster._mName;`  
`}`

### **🛡 Prevención**

* No guardar nombre

* No agregar aura

### **⚠️ RECOMENDADA: CON CUIDADO**

---

## **🔴 FEATURE DESCARTADAS (HONESTIDAD)**

❌ Corrupción entre niveles  
 ❌ Respawn parcial  
 ❌ Estados persistentes NPC  
 ❌ Cambios post-quest permanentes  
 ❌ Sistema de corrupción del mundo

👉 **No están previstas en el código original**

---

# **3️⃣ INFORME FINAL — FEATURES RECOMENDADAS (LISTA CERRADA)**

### **✅ IMPLEMENTAR SIN MIEDO**

1. Garantizar quests

2. Mejora de iluminación

3. Densidad decorativa leve

4. Variantes por profundidad

### **⚠️ IMPLEMENTAR CON LÍMITES**

5. Regeneración total en niveles sin quest

6. Mutaciones simples

7. Monstruos élite sin persistencia

### **❌ NO IMPLEMENTAR**

Todo lo que:

* persista estado nuevo

* modifique saves

* propague efectos

---

# **4️⃣ OPINIÓN FINAL (SINCERA)**

🧠 **Lo que querés hacer ES totalmente viable**  
 🔥 **DevilutionX tiene muchísimo código incompleto o subutilizado**  
 ⚠️ El peligro aparece solo cuando:

* se quiere “modernizar”

* se quiere “persistir”

👉 **Tu enfoque correcto es**:

“Completar lo que Blizzard dejó a medio hacer”

Y eso **sí se puede hacer de forma elegante y segura**.

