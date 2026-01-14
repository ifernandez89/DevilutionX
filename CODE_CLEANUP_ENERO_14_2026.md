# Code Cleanup - 14 de Enero de 2026

## Resumen de Cambios

Se realizó una limpieza profunda del código para eliminar sobre-ingeniería, duplicación y posibles bugs.

---

## 1. Simplificación del Global Protection System (GPS)

**Antes:** Clase singleton compleja con 300+ líneas de código, múltiples métodos, sistema de alertas, etc.

**Después:** Funciones simples (~80 líneas):
- `InitGlobalProtection()` - Inicialización
- `ResetGlobalProtection()` - Reset para nueva partida
- `UpdateGlobalProtection()` - Actualización por frame
- `CanCreateMissile()` - Verificación simple
- `CanCastSpell()` - Verificación simple
- `IsInCriticalState()` - Estado del sistema

**Beneficio:** Código más simple, más fácil de mantener, mismo nivel de protección.

---

## 2. Eliminación de Duplicación de Iluminación

**Antes:** Dos sistemas de iluminación activos simultáneamente:
- `nightmare_lighting.cpp` - Sistema completo con tipos de luz
- `organic_lighting.cpp` - Sistema simple duplicado

**Después:** Solo `nightmare_lighting.cpp` (más completo)

**Archivos eliminados:**
- `Source/organic_lighting.h`
- `Source/organic_lighting.cpp`

**Archivos actualizados:**
- `Source/ui_nightmare.cpp` - Usa nightmare_lighting
- `Source/CMakeLists.txt` - Removido organic_lighting

---

## 3. Corrección de Variables Static en Funciones

**Problema:** Variables `static` dentro de funciones persisten entre partidas, causando comportamiento inesperado.

**Correcciones:**

### safety_metrics.cpp
```cpp
// ANTES (problemático):
void UpdateSafetyMetrics() {
    static uint32_t lastFrameTime = SDL_GetTicks();
    static std::vector<uint32_t> frameTimes;
    // ...
}

// DESPUÉS (correcto):
namespace {
    uint32_t g_lastFrameTime = 0;
    std::vector<uint32_t> g_frameTimes;
}

void ResetSafetyMetrics() {
    g_lastFrameTime = 0;
    g_frameTimes.clear();
}
```

### engine_health.cpp (ya corregido anteriormente)
```cpp
// Cooldown map movido a scope de archivo
static std::unordered_map<...> gApocalypseCooldownMap;

void ResetApocalypseCooldowns() {
    gApocalypseCooldownMap.clear();
}
```

---

## 4. Funciones de Reset Agregadas

Nuevas funciones para limpiar estado al iniciar nueva partida:

| Función | Archivo | Propósito |
|---------|---------|-----------|
| `ResetApocalypseCooldowns()` | engine_health.cpp | Limpia cooldowns de Apocalypse |
| `ResetGlobalProtection()` | global_protection_system.cpp | Resetea estado del GPS |
| `ResetSafetyMetrics()` | safety_metrics.cpp | Limpia métricas de safety |

Todas se llaman desde `StartGame()` en `diablo.cpp`.

---

## 5. Arquitectura Resultante

### Sistemas de Protección (Simplificados)

```
┌─────────────────────────────────────────────────────────┐
│                    PROTECCIÓN                           │
├─────────────────────────────────────────────────────────┤
│  engine_health.h/cpp                                    │
│  - CanSafelyCastApocalypse() ← Protección Apocalypse   │
│  - CanSafelyCastInferno()    ← Protección Inferno      │
│  - UpdateEngineHealth()      ← Monitoreo general       │
├─────────────────────────────────────────────────────────┤
│  global_protection_system.h/cpp (SIMPLIFICADO)         │
│  - CanCreateMissile()        ← Límite 500 missiles     │
│  - CanCastSpell()            ← Bloqueo en crítico      │
│  - UpdateGlobalProtection()  ← Detección de crítico    │
├─────────────────────────────────────────────────────────┤
│  safety/safety.h (headers-only)                         │
│  - CanAddMissile()           ← Verificación inline     │
│  - CanAddMonster()           ← Verificación inline     │
│  - Macros de conveniencia                              │
└─────────────────────────────────────────────────────────┘
```

### Sistema de Iluminación (Consolidado)

```
┌─────────────────────────────────────────────────────────┐
│                    ILUMINACIÓN                          │
├─────────────────────────────────────────────────────────┤
│  nightmare_lighting.h/cpp (ÚNICO)                       │
│  - Parpadeo de antorchas                               │
│  - Parpadeo de velas                                   │
│  - Parpadeo de fuego                                   │
│  - Pulso mágico                                        │
└─────────────────────────────────────────────────────────┘
```

---

## 6. Verificación

- ✅ Sin errores de diagnóstico
- ✅ Código más simple y mantenible
- ✅ Duplicación eliminada
- ✅ Variables static corregidas
- ✅ Funciones de reset implementadas

---

*Cleanup realizado por Kiro - 14 de Enero de 2026*
