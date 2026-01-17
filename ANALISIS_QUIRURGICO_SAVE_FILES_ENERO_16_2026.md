# 🔬 ANÁLISIS QUIRÚRGICO - SISTEMA DE SAVE FILES
**Fecha**: Enero 16, 2026  
**Propósito**: Entender EXACTAMENTE cómo se propaga la corrupción de datos

---

## 📦 **ESTRUCTURA COMPLETA DE UN SAVE FILE (.sv)**

Un archivo `.sv` en DevilutionX es un **archivo MPQ** (archivo comprimido) que contiene MÚLTIPLES archivos internos:

### **Archivos Dentro del .sv:**

1. **`game`** - Datos principales del juego
   - Header (tipo de juego: RETL/HELF/SHAR/SHLF)
   - Estado del nivel actual (setlevel, currlevel, leveltype)
   - Posición de la cámara (ViewPosition)
   - Flags de UI (invflag, CharFlag)
   - Contadores (ActiveMonsterCount, ActiveItemCount, etc.)
   - Seeds de dungeons para todos los niveles
   - **DATOS DEL JUGADOR COMPLETOS** ← AQUÍ ESTÁN LAS COORDENADAS
   - Quests
   - Portales
   - Monster kill counts
   - Monstruos activos (si no es town)
   - Misiles activos
   - Objetos activos
   - Items en el suelo
   - Luces activas
   - Datos de visión
   - Items únicos encontrados
   - Grids del dungeon (dLight, dFlags, dPlayer, dItem, dMonster, etc.)
   - Items premium de Griswold
   - Estado del automapa

2. **`heroitems`** - Items del héroe
   - Items equipados (InvBody)
   - Items en inventario (InvList)
   - Items en cinturón (SpdList)

3. **`hotkeys`** - Hotkeys de spells

4. **`additionalMissiles`** - Misiles adicionales (>125)

5. **`levelseeds`** - Seeds de niveles

6. **`tempX`** o **`permX`** - Archivos de NIVEL INDIVIDUAL (uno por cada nivel visitado)
   - `temp0`, `temp1`, `temp2`, etc. (niveles temporales)
   - `perm0`, `perm1`, `perm2`, etc. (niveles permanentes)
   
   Cada archivo de nivel contiene:
   - dCorpse (cadáveres)
   - ActiveMonsterCount, ActiveItemCount, ActiveObjectCount
   - **Monstruos del nivel** ← PUEDEN TENER COORDENADAS CORRUPTAS
   - **Objetos del nivel** ← PUEDEN TENER COORDENADAS CORRUPTAS
   - **Items en el suelo** ← PUEDEN TENER COORDENADAS CORRUPTAS
   - dFlags
   - dMonster, dObject grids
   - dLight, dPreLight
   - AutomapView

---

## 🦠 **VECTORES DE INFECCIÓN - CÓMO SE PROPAGA LA CORRUPCIÓN**

### **Vector 1: Coordenadas del Jugador (PROTEGIDO ✅)**
```
Archivo: game
Función: LoadPlayer()
Protección: ✅ Architectural Shield sanitiza en LoadPlayer()
Estado: PROTEGIDO
```

### **Vector 2: Coordenadas de Monstruos (VULNERABLE ❌)**
```
Archivo: game + tempX/permX
Función: LoadMonster()
Datos corruptos:
  - monster.position.tile.x/y
  - monster.position.future.x/y
  - monster.position.old.x/y
  - monster.position.temp.x/y
  - monster.position.last.x/y
Protección: ❌ NO SANITIZADO
Estado: VULNERABLE
```

### **Vector 3: Coordenadas de Items (VULNERABLE ❌)**
```
Archivo: game + tempX/permX
Función: LoadItemData()
Datos corruptos:
  - item.position.x/y
Protección: ❌ NO SANITIZADO
Estado: VULNERABLE
```

### **Vector 4: Coordenadas de Objetos (VULNERABLE ❌)**
```
Archivo: tempX/permX
Función: LoadObject()
Datos corruptos:
  - object.position.x/y
Protección: ❌ NO SANITIZADO
Estado: VULNERABLE
```

### **Vector 5: Coordenadas de Luces (VULNERABLE ❌)**
```
Archivo: game
Función: LoadLighting()
Datos corruptos:
  - pLight->position.tile.x/y
  - pLight->position.old.x/y
Protección: ❌ NO SANITIZADO
Estado: VULNERABLE
```

### **Vector 6: Coordenadas de Portales (VULNERABLE ❌)**
```
Archivo: game
Función: LoadPortal()
Datos corruptos:
  - pPortal->position.x/y
Protección: ❌ NO SANITIZADO
Estado: VULNERABLE
```

---

## 🎯 **POR QUÉ SIGUE CRASHEANDO**

**El Architectural Shield solo protege las coordenadas DEL JUGADOR**, pero el crash puede ser causado por:

1. **Monstruo con coordenadas corruptas** → Rendering intenta dibujar en posición inválida → CRASH
2. **Item con coordenadas corruptas** → Sistema intenta acceder a dItem[x][y] con x,y inválidos → CRASH
3. **Objeto con coordenadas corruptas** → Sistema intenta acceder a dObject[x][y] → CRASH
4. **Luz con coordenadas corruptas** → Sistema de iluminación accede a memoria inválida → CRASH

---

## 🔍 **CÓMO SE INFECTA EL SISTEMA**

### **Escenario de Infección:**

1. **Inicio del juego**: Cargas el save corrupto
2. **LoadGame()**: Carga el archivo `game`
   - ✅ Coordenadas del jugador sanitizadas (39, 39)
   - ❌ Monstruos cargados con coordenadas corruptas
   - ❌ Items cargados con coordenadas corruptas
   - ❌ Luces cargadas con coordenadas corruptas

3. **Gameplay**: Juegas normalmente
   - El jugador se mueve correctamente (coordenadas sanitizadas)
   - Pero hay un monstruo en posición (999, 999) ← CORRUPTO
   - O un item en posición (-50, -50) ← CORRUPTO

4. **Rendering/Logic**: El juego intenta procesar el monstruo/item corrupto
   - Intenta acceder a `dMonster[999][999]` → FUERA DE RANGO
   - O intenta dibujar en pantalla coordenadas inválidas
   - **CRASH** 💥

5. **Cambio de nivel**: Usas teleport
   - **LoadLevel()** carga `temp4` (nivel 4)
   - ❌ Monstruos del nivel 4 tienen coordenadas corruptas
   - ❌ Items del nivel 4 tienen coordenadas corruptas
   - El juego intenta procesarlos → **CRASH** 💥

---

## 💡 **LA SOLUCIÓN QUIRÚRGICA REAL**

### **Opción 1: Protección Total (RECOMENDADO)**
Extender el Architectural Shield para sanitizar:
- ✅ Coordenadas de jugador (YA HECHO)
- ⚠️ Coordenadas de monstruos (FALTA)
- ⚠️ Coordenadas de items (FALTA)
- ⚠️ Coordenadas de objetos (FALTA)
- ⚠️ Coordenadas de luces (FALTA)
- ⚠️ Coordenadas de portales (FALTA)

### **Opción 2: Validación de Save File (ALTERNATIVA)**
Crear un sistema que:
1. Detecta saves corruptos al cargar
2. Valida TODAS las coordenadas antes de usarlas
3. Rechaza el save si está muy corrupto
4. Ofrece "reparar" o "empezar de nuevo"

### **Opción 3: Eliminar Save Corrupto (MÁS SIMPLE)**
- Borrar el `.sv` corrupto
- Empezar partida nueva
- El juego generará datos limpios

---

## 🧬 **RESPUESTA A TUS PREGUNTAS**

### **¿Qué contiene el .sv que arrastra corrupción?**
**TODO**: Jugador, monstruos, items, objetos, luces, portales, grids del dungeon.

### **¿Si comienzo partida de 0, no debería recargar todos los datos?**
**NO**. Cuando cargas un save existente, el juego:
- Lee los datos DEL SAVE FILE (que están corruptos)
- NO regenera nada, usa lo que está guardado
- Por eso la corrupción persiste

### **¿Cómo es posible que datos corruptos se FILTREN?**
**Porque solo protegemos las coordenadas del JUGADOR**, no las de:
- Monstruos
- Items  
- Objetos
- Luces
- Portales

### **¿Cómo vienen corruptos del .sv?**
El save file se corrompió en algún momento (bug anterior, crash durante save, etc.) y ahora contiene datos inválidos que se cargan directamente en memoria.

### **¿Cómo infectan nuestro sistema?**
Se cargan en memoria sin sanitizar → El rendering/logic intenta usarlos → Acceso a memoria inválida → CRASH

---

## 🎯 **RECOMENDACIÓN FINAL**

**IMPLEMENTAR PROTECCIÓN TOTAL**:
1. Extender `SanitizeCoordinates()` a LoadMonster()
2. Extender a LoadItemData()
3. Extender a LoadObject()
4. Extender a LoadLighting()
5. Extender a LoadPortal()

**Esto creará un escudo IMPENETRABLE que sanitiza TODOS los datos corruptos, no solo los del jugador.**

---

**¿Procedemos con la protección total?** 🛡️
