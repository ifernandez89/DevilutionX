# Plan de Viabilidad y Ejecución: "Nightmare UI Pass" (DevilutionX)

Este plan establece la arquitectura, diseño por fases y protocolo de verificación para implementar la identidad visual **Nightmare UI** en DevilutionX (PC, Móvil y WebAssembly). Se basa en la transformación cromática no invasiva (*Lluvia atmosférica + Botones Gris Piedra*) garantizando 0% de regresión en la geometría de slots, drag & drop y estabilidad WASM.

---

## 🎯 Solución Arquitectónica (Transformación Cromática Cero Riesgo)

```
┌────────────────────────────────────────────────────────┐
│               CAPA DE LÓGICA (C++)                     │
│  • InvGetItem()  • CheckInvSwap()  • Stash pagination  │ ──► [100% INTACTA / CONGELADA]
└────────────────────────────────────────────────────────┘
                           │
┌──────────────────────────▼─────────────────────────────┐
│          CAPA VISUAL / PRESENTACIÓN (ASSETS)           │
│  • inv.cel / inventory.pcx  • stash.cel  • panel8bu    │ ──► [REMAPEO DE PALETA TRN]
└────────────────────────────────────────────────────────┘
```

---

## 📐 Matriz de Intervención por Fases

| Fase | Componente | Transformación | Impacto Visual | Riesgo Técnico | Decisión |
| :--- | :--- | :--- | :---: | :---: | :---: |
| **0** | **Lluvia & Botones Base** | Lluvia en Tristram + Traductores TRN | ⭐⭐⭐⭐⭐ | 🟢 Nulo | **Baseline Activo** |
| **1** | **Inventario (`inv.cpp`)** | Marco `gold → dark iron` & `warm stone → cold slate` vía `GetGoldToStoneTRN()` | ⭐⭐⭐⭐⭐ | 🟢 Muy Bajo | **Prioridad 1** |
| **2** | **Stash / Alijo (`stash.cpp`)** | Marco `wood/warm → dark wood/cold iron` vía `GetGoldToStoneTRN()` | ⭐⭐⭐⭐ | 🟢 Muy Bajo | **Prioridad 2** |
| **3** | **Botones HUD (`mainpanel.cpp`)** | Homogeneizar *Char, Quests, Map, Menu, Inv, Spells* a gris piedra con micro-estados | ⭐⭐⭐⭐ | 🟢 Bajo | **Prioridad 3** |
| **4** | **Orbes HP / Mana** | Tinte líquido profundo (*Blood Red / Deep Arcane Blue*) aislado vía paleta | ⭐⭐ | 🟡 Medio | **Condicional / Aislado** |

---

## 🎨 Matriz de Transformación Cromática ("Gothic Stone")

Se mantendrá la regla de transformación global por tablas de traducción de paleta TRN (`GetGoldToStoneTRN()`), aislando de forma estricta los elementos de información semántica:

### Regla Estructural (UI Frames & Backgrounds)
- **Bordes Dorados / Cálidos:** Muted Bronze / Aged Iron / Dark Slate (Paleta 240..255).
- **Fondos de Piedra Cálida:** Cold Dark Slate / Obsidian Charcoal.
- **Detalles de Madera/Cuero:** Dark Charcoal / Weathered Leather.

### Regla Semántica (Información Intacta)
- 🔴 **HP / Inválido:** Crimson / Rojo Púrpura Puro.
- 🔵 **Mana / Magia:** Azul Zafiro Profundo.
- 🟡 **Oro / Únicos:** Amarillo Oro Ocre.
- 🟢 **Veneno / Sets:** Verde Esmeralda.
- ⚪ **Items Normales / Texto:** Blanco Marfil Legible.

---

## Proposed Changes

### Layer 1: Engine Palette TRN Translation System

#### [MODIFY] [trn.cpp](file:///c:/Projects/DevilutionX/Source/engine/trn.cpp)
#### [MODIFY] [trn.hpp](file:///c:/Projects/DevilutionX/Source/engine/trn.hpp)
- Refinar `GetGoldToStoneTRN()` para mapear matices amarillos/dorados y tonos beige/cálidos a la gama slate gray (`PAL16_GRAY`).

---

### Layer 2: UI Panels Gothic Remap

#### [MODIFY] [inv.cpp](file:///c:/Projects/DevilutionX/Source/inv.cpp)
- Aplicar `ClxApplyTrans(*pInvCels, GetGoldToStoneTRN())` garantizando que los rectángulos interactivos (`InvRect[]`) permanezcan pixel-perfect.

#### [MODIFY] [stash.cpp](file:///c:/Projects/DevilutionX/Source/qol/stash.cpp)
- Aplicar `ClxApplyTrans` sobre `StashPanelArt` y `StashNavButtonArt` con la tabla `GetGoldToStoneTRN()`.

#### [MODIFY] [mainpanel.cpp](file:///c:/Projects/DevilutionX/Source/panels/mainpanel.cpp)
#### [MODIFY] [control_panel.cpp](file:///c:/Projects/DevilutionX/Source/control/control_panel.cpp)
- Aplicar traducción TRN a los botones principales (*Char, Quests, Automap, Menu, Inv, Spells*) y refinar la presentación visual del HUD nativo.

---

## Verification Plan

### Automated Tests / Compilación
- Compilar la versión nativa en Windows con Ninja:
  ```powershell
  cmake -S . -B build_COMPILE_FRESH -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DNONET=ON -DNOSOUND=OFF -DBUILD_TESTING=OFF -DDISABLE_ZERO_TIER=ON -DUNPACKED_MPQS=OFF -DUNPACKED_SAVES=OFF -DDEVILUTIONX_STATIC_BZIP2=ON -DDEVILUTIONX_SYSTEM_BZIP2=OFF
  cmake --build build_COMPILE_FRESH -j 4
  ```

### Manual Verification
1. **Verificación de Inventario & Alijo:**
   - Abrir el panel de inventario y el alijo (Stash). Confirmar la estética gris piedra fría/slate sin desalineación de casillas ni fallos en drag & drop.
2. **Verificación de Botones HUD:**
   - Interactuar con los 6 botones principales del panel (Char, Quests, Map, Menu, Inv, Spells) comprobando los micro-estados visuales.
3. **Verificación de Jugabilidad:**
   - Lanzar hechizos, consumir pociones y comprobar la legibilidad del texto de items en el suelo y tooltips.
