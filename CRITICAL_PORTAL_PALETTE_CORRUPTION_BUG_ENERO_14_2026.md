# 🚨 CRITICAL BUG: Portal Palette Corruption
**Fecha**: Enero 14, 2026  
**Severidad**: CRÍTICA - Peor que Apocalypse  
**Estado**: 🔄 EN CORRECCIÓN

## 🔥 Descripción del Bug

### Síntoma Visual
Al pasar por portales o escaleras entre niveles, la pantalla muestra **corrupción masiva de colores**:
- Colores psicodélicos/distorsionados
- Paleta completamente incorrecta
- Efecto "arcoíris corrupto"
- Dura varios frames (no es instantáneo)

### Impacto
- **Experiencia de Usuario**: DESTRUIDA - Rompe completamente la inmersión
- **Profesionalismo**: CRÍTICO - Parece un juego roto
- **Comparación**: **PEOR QUE APOCALYPSE** - Apocalypse crasheaba, esto corrompe visualmente

## 🔍 Análisis Técnico

### Causa Raíz Identificada
El problema ocurre en la secuencia de transición de nivel:

1. **Frame N**: Jugador usa portal
2. **Frame N+1**: Cutscene de portal (usa paleta especial)
3. **Frame N+2**: Nuevo nivel cargado PERO paleta vieja aún activa
4. **Frame N+3-5**: Renderizado con paleta INCORRECTA → CORRUPCIÓN VISUAL
5. **Frame N+6**: Paleta correcta finalmente cargada

### Problema Específico
```cpp
// En DrawGame() - scrollrt.cpp línea ~1583
if (g_needsPaletteReloadAfterPortal) {
    g_needsPaletteReloadAfterPortal = false;
    
    // ❌ PROBLEMA: Esto ocurre DESPUÉS de que ya se renderizaron frames corruptos
    LoadPalette("levels\\...");
    UpdateSystemPalette(logical_palette);
    MakeLightTable();
}
```

**El timing está mal**: La paleta se recarga DESPUÉS de renderizar, no ANTES.

### Secuencia Actual (INCORRECTA)
```
Portal usado
  ↓
Skip 3 frames (g_skipRenderFramesAfterPortal = 3)
  ↓
Frame 1: Renderiza con paleta VIEJA → CORRUPCIÓN ❌
Frame 2: Renderiza con paleta VIEJA → CORRUPCIÓN ❌
Frame 3: Renderiza con paleta VIEJA → CORRUPCIÓN ❌
  ↓
g_needsPaletteReloadAfterPortal = true
  ↓
Frame 4: Recarga paleta → OK ✅
```

### Secuencia Correcta (OBJETIVO)
```
Portal usado
  ↓
Skip 3 frames
  ↓
Frame 1: PRIMERO recarga paleta, LUEGO renderiza → OK ✅
Frame 2: Renderiza con paleta correcta → OK ✅
Frame 3: Renderiza con paleta correcta → OK ✅
```

## 🎯 Solución Propuesta

### Fix 1: Recargar Paleta ANTES del Primer Frame
Mover la recarga de paleta al INICIO de DrawGame, no después del skip.

```cpp
// ANTES del check de skip frames
if (g_needsPaletteReloadAfterPortal) {
    // Recargar INMEDIATAMENTE
    LoadPalette(...);
    UpdateSystemPalette(logical_palette);
    MakeLightTable();
    g_needsPaletteReloadAfterPortal = false;
}

// DESPUÉS renderizar
if (g_skipRenderFramesAfterPortal > 0) {
    // ...
}
```

### Fix 2: Aumentar Skip Frames Durante Recarga
Mientras se recarga la paleta, no renderizar nada.

```cpp
if (g_needsPaletteReloadAfterPortal) {
    // Recargar paleta
    LoadPalette(...);
    UpdateSystemPalette(logical_palette);
    MakeLightTable();
    g_needsPaletteReloadAfterPortal = false;
    
    // NO renderizar este frame
    return;
}
```

### Fix 3: Forzar Recarga en LoadGame
Asegurar que la paleta se recarga INMEDIATAMENTE al cargar el nivel.

```cpp
// En diablo.cpp después de LoadLevel()
LoadPalette(...);
UpdateSystemPalette(logical_palette);
MakeLightTable();
```

## 📊 Comparación con Apocalypse

| Aspecto | Apocalypse Bug | Portal Palette Bug |
|---------|----------------|-------------------|
| **Frecuencia** | Cada portal | Cada portal/escalera |
| **Impacto Visual** | Ninguno (crash inmediato) | MASIVO (corrupción) |
| **Experiencia** | Frustración | Náusea visual |
| **Profesionalismo** | Juego inestable | Juego roto |
| **Severidad** | Alta | **CRÍTICA** |

**Conclusión**: Este bug es PEOR porque:
1. Ocurre en CADA transición (no solo portales)
2. Es VISIBLE y HORRIBLE
3. Hace que el juego parezca amateur/roto
4. Afecta la experiencia constantemente

## 🔧 Plan de Corrección

### Paso 1: Mover Recarga de Paleta
- Ubicación: `scrollrt.cpp` línea ~1560
- Acción: Mover ANTES del skip check

### Paso 2: Agregar Return Inmediato
- Ubicación: Después de recargar paleta
- Acción: No renderizar durante recarga

### Paso 3: Testing Exhaustivo
- Town → Dungeon
- Dungeon → Town
- Escaleras entre niveles
- Múltiples transiciones rápidas

### Paso 4: Validación Visual
- Captura de pantalla de cada frame
- Verificar paleta correcta en frame 1
- Sin corrupción visible

## 📝 Archivos a Modificar

```
Source/engine/render/scrollrt.cpp
  - DrawGame(): Reordenar recarga de paleta
  - Línea ~1560-1620

Source/diablo.cpp
  - LoadGameLevel(): Forzar recarga inmediata
  - Línea ~3850-3880
```

## 🎯 Criterio de Éxito

✅ **CERO frames con corrupción visual**  
✅ **Transición suave y limpia**  
✅ **Paleta correcta desde frame 1**  
✅ **Sin lag perceptible**  

## 🚀 Prioridad

**MÁXIMA** - Este bug destruye la experiencia del jugador y hace que el juego parezca roto.

---

**Nota**: Este bug es más crítico que Apocalypse porque afecta la percepción de calidad del juego completo, no solo la estabilidad.
