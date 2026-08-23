# 🔧 FIX: Menu Order & Sorcerer Color - Enero 2026

## 🎯 PROBLEMAS IDENTIFICADOS Y SOLUCIONADOS

### **1. ORDEN DEL MENÚ IN-GAME** ✅ SOLUCIONADO

#### **Problema**
- En Diablo 1 original: Save Game aparece primero en el menú
- En DevilutionX: Options aparecía primero, Save Game segundo

#### **Solución Implementada**
**Archivo**: `Source/gamemenu.cpp`

**Antes**:
```cpp
TMenuItem sgSingleMenu[] = {
    { GMENU_ENABLED, N_("Options"),           &GamemenuOptions    },
    { GMENU_ENABLED, N_("Save Game"),         &gamemenu_save_game },
    { GMENU_ENABLED, N_("Load Game"),         &gamemenu_load_game },
    // ...
};
```

**Después**:
```cpp
TMenuItem sgSingleMenu[] = {
    { GMENU_ENABLED, N_("Save Game"),         &gamemenu_save_game },
    { GMENU_ENABLED, N_("Options"),           &GamemenuOptions    },
    { GMENU_ENABLED, N_("Load Game"),         &gamemenu_load_game },
    // ...
};
```

#### **Ajustes Adicionales**
- Actualizada función `GamemenuUpdateSingle()` para manejar los nuevos índices
- Save Game (índice 0): Se habilita/deshabilita según estado del jugador
- Options (índice 1): Se habilita/deshabilita según estado del jugador
- Load Game (índice 2): Se habilita según `gbValidSaveFile`

#### **Resultado**
✅ **Orden correcto como Diablo 1 original**:
1. Save Game
2. Options  
3. Load Game
4. Exit to Main Menu
5. Quit Game

---

### **2. COLOR DEL SORCERER** ⚠️ PENDIENTE DE DECISIÓN

#### **Problema Actual**
- Transformación violeta/púrpura causa tonos blanquecinos y rojos extraños
- Implementación en `Source/engine/trn.cpp` líneas 41-73
- Mapeo de colores puede estar causando conflictos visuales

#### **Opciones Disponibles**

##### **OPCIÓN 1: REVERTIR AL ROJO ORIGINAL** ✅ (Más Seguro)
```cpp
// Eliminar completamente el bloque de transformación de Sorcerer
// Volver al comportamiento original de Diablo 1
```
- **Ventajas**: Cero riesgo, color clásico garantizado
- **Desventajas**: No hay personalización de color

##### **OPCIÓN 2: AZUL LIMPIO** 🔵 (Recomendado)
```cpp
// Transformar rojo → azul usando rangos estables
// Mapear a paleta azul (32-47, 96-111, etc.)
```
- **Ventajas**: Color distintivo, mejor contraste que violeta
- **Desventajas**: Requiere testing cuidadoso

##### **OPCIÓN 3: VIOLETA MEJORADO** 🟣 (Experimental)
```cpp
// Corregir rangos de violeta actuales
// Usar paleta violeta más estable (144-159, etc.)
```
- **Ventajas**: Mantiene la intención original
- **Desventajas**: Riesgo de efectos visuales extraños

#### **Análisis Técnico del Problema**
El código actual mapea:
- Rojos oscuros (224-231) → Púrpuras (200-207)
- Naranjas/marrones (232-239) → Violetas (160-167)  
- Rojos claros (240-247) → Púrpuras claros (208-215)
- Rojos adicionales (176-181) → Púrpuras/violetas específicos

**Posible causa de tonos blanquecinos**: Los rangos de destino (200-207, 160-167, 208-215) pueden no ser los correctos en la paleta de Diablo, causando mapeo a colores no deseados.

---

## 🎮 ESTADO ACTUAL

### **Completado** ✅
- [x] Orden del menú corregido (Save Game primero)
- [x] Función `GamemenuUpdateSingle()` actualizada
- [x] Compilación exitosa
- [x] Ejecutable funcional: `build_NOW/devilutionx.exe`

### **Pendiente** ⏳
- [ ] Decisión sobre color del Sorcerer
- [ ] Implementación de la opción elegida
- [ ] Testing visual del resultado

---

## 🔧 ARCHIVOS MODIFICADOS

```
Source/gamemenu.cpp                    - Orden del menú corregido
FIX_MENU_ORDER_AND_SORCERER_COLOR.md  - Esta documentación
```

---

## 🎯 RECOMENDACIÓN

**Para el color del Sorcerer**, recomiendo **OPCIÓN 1** (revertir al rojo original) por las siguientes razones:

1. **Estabilidad garantizada**: Cero riesgo de efectos visuales extraños
2. **Fidelidad al original**: Mantiene la experiencia clásica de Diablo 1
3. **Simplicidad**: Elimina código complejo que puede causar problemas
4. **Tiempo**: Permite enfocar esfuerzos en FASE D2 y otras features

Si prefieres color personalizado, **OPCIÓN 2** (azul limpio) sería la segunda mejor opción.

---

*Análisis y solución por: Arquitecto Senior C++*  
*Fecha: Enero 9, 2026*  
*Estado: Menú ✅ SOLUCIONADO | Color ⏳ PENDIENTE DE DECISIÓN*