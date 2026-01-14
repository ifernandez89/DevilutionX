# APOCALYPSE RAPID-FIRE FIX - ENERO 13, 2026

## PROBLEMA IDENTIFICADO

El juego crasheaba al castear Apocalypse debido a un bug crítico en el sistema de protección que permitía **múltiples casts en el mismo frame**.

### Evidencia del Log
```
21:26:16 [APOCALYPSE_CAST] Player:0 Level:13 CurrentMissiles:1 CastCount:1
21:26:16 [APOCALYPSE_CAST] Player:0 Level:13 CurrentMissiles:11 CastCount:2
21:26:16 [APOCALYPSE_CAST] Player:0 Level:13 CurrentMissiles:10 CastCount:3
```

**3 Apocalypse casteados en el mismo segundo** → 30 booms creados → overflow del array de misiles → CRASH

## ROOT CAUSE

En `Source/engine_health.cpp`, la función `CanSafelyCastApocalypse()` tenía un bug fatal:

```cpp
// CÓDIGO BUGUEADO (ANTES)
bool CanSafelyCastApocalypse()
{
    frameCounter++;  // ❌ SE INCREMENTA CADA VEZ QUE SE LLAMA
    
    if (lastApocalypseFrame == frameCounter) {
        return false;  // Nunca se cumple porque frameCounter siempre es diferente
    }
    
    lastApocalypseFrame = frameCounter;
    return true;  // ✅ SIEMPRE PERMITE
}
```

### Por qué fallaba:
1. **Primera llamada**: `frameCounter=1`, `lastApocalypseFrame=-1` → PERMITE (setea `lastApocalypseFrame=1`)
2. **Segunda llamada**: `frameCounter=2`, `lastApocalypseFrame=1` → PERMITE (setea `lastApocalypseFrame=2`)
3. **Tercera llamada**: `frameCounter=3`, `lastApocalypseFrame=2` → PERMITE (setea `lastApocalypseFrame=3`)

El contador se incrementaba **dentro de la función de validación**, haciendo que cada llamada pareciera estar en un frame diferente.

## SOLUCIÓN IMPLEMENTADA

Eliminé el sistema de frame counter defectuoso y simplifiqué la protección usando **solo tiempo real**:

```cpp
// CÓDIGO CORREGIDO (AHORA)
bool CanSafelyCastApocalypse()
{
    // ATOMIC CHECK: Si hay un Apocalypse en progreso, rechazar
    if (apocalypseInProgress) {
        return false;
    }
    
    // TIME-BASED COOLDOWN: 50ms entre casts
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastCast = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastApocalypseCast);
    
    if (timeSinceLastCast.count() < 50) {
        return false;  // Cooldown activo
    }
    
    // ATOMIC LOCK: Bloquear hasta que termine
    apocalypseInProgress = true;
    lastApocalypseCast = now;
    return true;
}

void ClearApocalypseInProgress()
{
    // Desbloquear cuando termine el procesamiento
    apocalypseInProgress = false;
}
```

### Mejoras:
1. **Atomic lock simple**: Una variable booleana que bloquea mientras se procesa
2. **Cooldown de 50ms**: Previene rapid-fire sin afectar la jugabilidad
3. **Sin frame counters**: Eliminado el sistema defectuoso
4. **Unlock explícito**: `ClearApocalypseInProgress()` ahora sí desbloquea

## ARCHIVOS MODIFICADOS

- `Source/engine_health.cpp` (líneas 125-150)
  - Eliminadas variables: `frameCounter`, `lastApocalypseFrame`, `apocalypseUnlockFrame`
  - Simplificada lógica de `CanSafelyCastApocalypse()`
  - Corregida función `ClearApocalypseInProgress()`

## TESTING

1. ✅ Compilación exitosa sin warnings de variables no usadas
2. ✅ Ejecutable generado: `devilutionx.exe` (5.74 MB)
3. ⏳ **PENDIENTE**: Testing en juego para verificar que no hay más crashes

## PRÓXIMOS PASOS

1. Testear Apocalypse múltiples veces en combate
2. Verificar que el cooldown de 50ms no afecta la jugabilidad
3. Si sigue crasheando, aumentar el cooldown a 100ms
4. Monitorear logs en `build_NOW/debug_logs/architectural_analysis.log`

---

**COMPILADO**: 21:32 - Enero 13, 2026  
**EJECUTABLE**: `devilutionx.exe` (root directory)  
**STATUS**: ✅ LISTO PARA TESTING
