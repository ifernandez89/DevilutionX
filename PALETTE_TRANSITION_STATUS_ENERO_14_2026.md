# 🎨 ESTADO: CORRUPCIÓN DE PALETA EN TRANSICIONES
**Fecha**: Enero 14, 2026  
**Estado**: 🔧 EN INVESTIGACIÓN

---

## 📋 PROBLEMA

Durante las transiciones de nivel (portales, escaleras, inicio de juego), la paleta se corrompe mostrando colores psicodélicos por un momento antes de estabilizarse.

---

## 🔍 DIAGNÓSTICO

### Síntomas:
1. Al cargar un nivel, los colores se ven correctos por un instante
2. Luego se corrompen (colores psicodélicos/incorrectos)
3. Después de un momento, se estabilizan
4. Ocurre en TODAS las transiciones:
   - Portales (town ↔ dungeon)
   - Escaleras (nivel a nivel)
   - Inicio del juego (carga inicial del town)

### Causa Raíz Identificada:
El sistema de **paleta contextual** en `Source/engine/palette.cpp` aplica efectos basados en `leveltype` (líneas 147-230). Durante las transiciones, `leveltype` puede estar en un estado inválido o intermedio, causando que se apliquen efectos incorrectos.

---

## 🔧 SOLUCIONES INTENTADAS

### Intento 1: Agregar UpdateSystemPalette en LoadGameLevel
**Resultado**: ❌ No funcionó

### Intento 2: Crear UpdateProgressPalette()
**Resultado**: ❌ No funcionó

### Intento 3: Remover UpdateSystemPalette de LoadCutsceneBackground
**Resultado**: ❌ Empeoró el problema

### Intento 4: Saltar cutscene para portales/escaleras
**Resultado**: ❌ Causó crash

### Intento 5: Flag g_skipContextualPaletteEffects
**Archivo**: `Source/engine/palette.cpp` + `Source/interfac.cpp`  
**Implementación**:
```cpp
// En palette.cpp (línea 124)
bool g_skipContextualPaletteEffects = false;

// En ApplyGlobalBrightness (línea 147)
if (g_skipContextualPaletteEffects) {
    return; // Skip contextual effects during transitions
}

// En interfac.cpp ShowProgress (línea 648)
g_skipContextualPaletteEffects = true;

// En interfac.cpp WM_DONE (línea 540)
g_skipContextualPaletteEffects = false;
UpdateSystemPalette(logical_palette);
```

**Resultado**: 🔧 **PENDIENTE DE TESTING**

---

## 💡 TEORÍA ACTUAL

El problema persiste porque:

1. **Timing**: La paleta se actualiza en múltiples puntos durante la transición
2. **Estado Intermedio**: `leveltype` cambia antes de que la paleta se actualice completamente
3. **Múltiples Sistemas**: Varios sistemas tocan la paleta durante la carga:
   - `LoadCutsceneBackground()` carga la paleta de la cutscene
   - `LoadGameLevel()` carga la paleta del nivel
   - `UpdateSystemPalette()` aplica efectos contextuales
   - `PaletteFadeIn/Out()` aplica fades

---

## 🎯 PRÓXIMA SOLUCIÓN A INTENTAR

### Opción A: Deshabilitar Efectos Contextuales Permanentemente
**Pros**: Garantiza que no habrá corrupción  
**Contras**: Perdemos los efectos atmosféricos que mejoran la experiencia

### Opción B: Aplicar Efectos Solo Después de Fade-In Completo
**Idea**: No aplicar efectos contextuales hasta que el fade-in termine completamente

```cpp
// En PaletteFadeIn, al final:
if (!g_skipContextualPaletteEffects) {
    UpdateSystemPalette(logical_palette); // Aplicar efectos DESPUÉS del fade
}
```

### Opción C: Guardar y Restaurar Paleta del Nivel
**Idea**: Guardar la paleta correcta del nivel antes de mostrar la cutscene

```cpp
// Antes de LoadCutsceneBackground:
std::array<SDL_Color, 256> savedLevelPalette = logical_palette;

// Después de cargar el nivel:
logical_palette = savedLevelPalette;
UpdateSystemPalette(logical_palette);
```

---

## 🔍 ÁREAS A INVESTIGAR

1. **Orden de Operaciones**: ¿En qué orden se llaman las funciones de paleta?
2. **LoadGameLevel**: ¿Cuándo exactamente se carga la paleta del nivel?
3. **Fade Timing**: ¿El fade-in ocurre antes o después de aplicar efectos?
4. **Multiple Updates**: ¿Se llama `UpdateSystemPalette` múltiples veces?

---

## 📊 IMPACTO

### Jugabilidad:
- ✅ No afecta la jugabilidad (solo visual)
- ✅ No causa crashes
- ⚠️ Experiencia visual degradada durante transiciones

### Prioridad:
- **Media-Baja**: Es un problema cosmético, no crítico
- El juego es completamente jugable
- Sería ideal arreglarlo para una experiencia pulida

---

## 🎮 WORKAROUND TEMPORAL

**Para el usuario**: El problema es temporal (dura ~1 segundo) y no afecta el juego. Simplemente espera a que los colores se estabilicen después de cada transición.

---

## 📝 CÓDIGO RELEVANTE

### Archivos Involucrados:
- `Source/engine/palette.cpp` - Sistema de paleta y efectos contextuales
- `Source/engine/palette.h` - Declaraciones
- `Source/interfac.cpp` - Cutscenes y transiciones
- `Source/diablo.cpp` - LoadGameLevel (carga de niveles)

### Funciones Clave:
- `ApplyGlobalBrightness()` - Aplica efectos contextuales (líneas 147-230)
- `UpdateSystemPalette()` - Actualiza la paleta del sistema
- `LoadCutsceneBackground()` - Carga paleta de cutscene
- `PaletteFadeIn/Out()` - Efectos de fade
- `ShowProgress()` - Maneja el proceso de carga

---

## 🔗 DOCUMENTOS RELACIONADOS

- `APOCALYPSE_PORTAL_CRASH_FIX_FINAL_ENERO_14_2026.md` - Fix del crash del portal (EXITOSO)
- `PALETTE_CORRUPTION_FIX_FINAL_ENERO_14_2026.md` - Intentos anteriores
- `TECHNICAL_ARCHITECTURE_SUMMARY_ENERO_12_2026.md` - Arquitectura del sistema de paleta

---

**Última Actualización**: Enero 14, 2026  
**Próximo Paso**: Testing del flag g_skipContextualPaletteEffects  
**Prioridad**: Media-Baja (cosmético, no crítico)
