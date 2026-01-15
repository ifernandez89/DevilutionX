# 🔍 ANÁLISIS DETALLADO DEL FLUJO DE PALETA
**Fecha**: Enero 14, 2026  
**Basado en**: Documentación exhaustiva del debugging del portal crash

---

## 🎯 DESCUBRIMIENTO CLAVE

Revisando la documentación del debugging del portal crash, encontré el flujo EXACTO documentado:

### FLUJO DOCUMENTADO (De RESUMEN_FINAL_ENERO_14_2026_NOCHE.md):

```
1. ShowProgress() inicia
   ├─ g_isLevelTransition = true
   ├─ LoadCutsceneBackground()
   │  ├─ LoadPalette(cutscene.pal)
   │  └─ UpdateSystemPalette(logical_palette) ← Paleta de cutscene
   │
2. DoLoad() - Carga el nivel en background
   ├─ LoadGameLevel()
   │  └─ CreateLevel()
   │     ├─ LoadRndLvlPal(leveltype)
   │     │  └─ LoadPaletteAndInitBlending() ← Paleta del nivel
   │     └─ UpdateSystemPalette(logical_palette) ✅ FIX APLICADO
   │
3. WM_DONE - Finaliza la carga
   ├─ UpdateSystemPalette(ProgressEventHandlerState.palette) ← Paleta de cutscene
   ├─ PaletteFadeOut()
   ├─ UpdateSystemPalette(logical_palette) ← Paleta del nivel
   ├─ g_isLevelTransition = false
   └─ [ELIMINADO] UpdateSystemPalette(logical_palette) ← Llamada duplicada
```

---

## 🚨 PROBLEMA IDENTIFICADO

### El Bug Está en el Paso 3 (WM_DONE):

```cpp
// LÍNEA ~525: Usa paleta de CUTSCENE (guardada al inicio)
UpdateSystemPalette(ProgressEventHandlerState.palette);

// LÍNEA ~535: Fade out con paleta de CUTSCENE
PaletteFadeOut(8, ProgressEventHandlerState.palette);

// LÍNEA ~540: Intenta usar paleta del NIVEL
UpdateSystemPalette(logical_palette);
```

### ¿Cuál es el Problema?

1. **ProgressEventHandlerState.palette** = Paleta de la CUTSCENE (guardada en ShowProgress)
2. **logical_palette** = Paleta del NIVEL (cargada en CreateLevel)
3. **system_palette** = La que se muestra en pantalla

### La Secuencia Incorrecta:

```
Frame N:   UpdateSystemPalette(cutscene_palette)
           → system_palette = cutscene con efectos contextuales ❌
           
Frame N+1: PaletteFadeOut(cutscene_palette)
           → Fade desde cutscene_palette
           → Pero system_palette tiene efectos aplicados ❌
           
Frame N+2: UpdateSystemPalette(level_palette)
           → system_palette = level con efectos contextuales
           → Pero RedrawEverything() se llama AQUÍ
           → Los sistemas ven g_isLevelTransition = false (recién desactivado)
           → Aplican efectos sobre una paleta que ya tiene efectos ❌❌❌
```

---

## 💡 LA SOLUCIÓN

### Problema Real:
**UpdateSystemPalette aplica efectos contextuales CADA VEZ que se llama**, incluso durante transiciones.

### Solución:
**NO llamar UpdateSystemPalette en WM_DONE con la paleta de cutscene**. Solo aplicar la paleta del nivel UNA VEZ al final.

---

## 🔧 FIX PROPUESTO

### Cambio en `Source/interfac.cpp` WM_DONE:

```cpp
case WM_DONE: {
    if (!ProgressEventHandlerState.skipRendering) {
        NewCursor(CURSOR_HAND);

        if (!HeadlessMode) {
            assert(ghMainWnd);

            // ❌ ELIMINAR ESTO - No usar paleta de cutscene
            // UpdateSystemPalette(ProgressEventHandlerState.palette);

            // Ensure that all back buffers have the full progress bar.
            const void *initialPixels = PalSurface->pixels;
            do {
                DrawCutsceneForeground();
                if (DiabloUiSurface() == PalSurface)
                    BltFast(nullptr, nullptr);
                RenderPresent();
            } while (PalSurface->pixels != initialPixels);

            // ❌ ELIMINAR ESTO - No hacer fade con paleta de cutscene
            // PaletteFadeOut(8, ProgressEventHandlerState.palette);

            // ✅ NUEVO: Fade directo a negro sin tocar system_palette
            BlackPalette();
            
            // ✅ NUEVO: Aplicar paleta del nivel UNA SOLA VEZ
            // La paleta ya fue cargada en CreateLevel con LoadRndLvlPal
            // Solo necesitamos aplicarla al system_palette
            UpdateSystemPalette(logical_palette);
        }
    }
    
    // 🛡️ TRANSITION FLAG: Mark that transition is complete
    g_isLevelTransition = false;
    g_skipContextualPaletteEffects = false;
    
    // ... resto del código ...
}
```

---

## 🎯 POR QUÉ ESTO FUNCIONA

### Antes (Con Bug):
```
1. UpdateSystemPalette(cutscene) → Aplica efectos sobre cutscene ❌
2. PaletteFadeOut(cutscene) → Fade desde paleta con efectos ❌
3. UpdateSystemPalette(level) → Aplica efectos sobre level
4. g_isLevelTransition = false
5. RedrawEverything() → Sistemas aplican efectos OTRA VEZ ❌❌
```

### Después (Fix):
```
1. BlackPalette() → Pantalla negra (simple)
2. UpdateSystemPalette(level) → Aplica efectos sobre level UNA VEZ ✅
3. g_isLevelTransition = false
4. RedrawEverything() → Sistemas ven flag desactivado pero paleta ya está correcta ✅
```

---

## 📊 VENTAJAS DEL FIX

1. ✅ **Elimina doble aplicación de efectos**
2. ✅ **Usa solo la paleta del nivel (correcta)**
3. ✅ **Más simple - menos llamadas a UpdateSystemPalette**
4. ✅ **Más rápido - no hace fade innecesario**
5. ✅ **Más predecible - un solo punto de aplicación**

---

## 🔍 ANÁLISIS DE POR QUÉ LA PRIMERA TRANSICIÓN FUNCIONA

### Primera Transición (Inicio del Town):
```
ShowProgress(WM_DIABNEWGAME)
├─ LoadCutsceneBackground("cutstart") ← Paleta de inicio
├─ DoLoad()
│  └─ CreateLevel()
│     ├─ LoadRndLvlPal(DTYPE_TOWN) ← Paleta de town
│     └─ UpdateSystemPalette(logical_palette) ✅ APLICADO AQUÍ
└─ WM_DONE
   ├─ UpdateSystemPalette(cutstart_palette) ← Sobrescribe con cutscene ❌
   ├─ PaletteFadeOut(cutstart_palette)
   └─ UpdateSystemPalette(town_palette) ← Vuelve a aplicar town ✅
```

**¿Por qué funciona?**: Porque la paleta de "cutstart" es similar a la de town, entonces la sobrescritura temporal no se nota tanto.

### Otras Transiciones (Portales/Escaleras):
```
ShowProgress(WM_DIABWARPLVL)
├─ LoadCutsceneBackground("cutportr") ← Paleta ROJA
├─ DoLoad()
│  └─ CreateLevel()
│     ├─ LoadRndLvlPal(DTYPE_HELL) ← Paleta de HELL (roja/naranja)
│     └─ UpdateSystemPalette(logical_palette) ✅ APLICADO AQUÍ
└─ WM_DONE
   ├─ UpdateSystemPalette(cutportr_palette) ← Sobrescribe con cutscene ROJA ❌
   ├─ PaletteFadeOut(cutportr_palette) ← Fade desde ROJO
   └─ UpdateSystemPalette(hell_palette) ← Aplica HELL con efectos
       → Pero los efectos se aplican sobre una paleta que ya tiene efectos ❌
       → CORRUPCIÓN VISIBLE
```

**¿Por qué falla?**: Porque las paletas son MUY diferentes (cutscene rojo vs nivel hell), y la doble aplicación de efectos causa corrupción visible.

---

## 🎉 CONCLUSIÓN

**El fix es simple**: 
1. NO usar la paleta de cutscene en WM_DONE
2. Ir directo a negro con BlackPalette()
3. Aplicar la paleta del nivel UNA SOLA VEZ

Esto elimina:
- ❌ Doble aplicación de efectos
- ❌ Uso de paleta incorrecta (cutscene)
- ❌ Fade innecesario que causa timing issues

Y garantiza:
- ✅ Paleta correcta desde el primer frame
- ✅ Efectos aplicados una sola vez
- ✅ Transición limpia y rápida

---

**Próximo Paso**: Implementar este fix en `Source/interfac.cpp` WM_DONE handler.

