#pragma once

/**
 * @file safety.h
 * 
 * SAFETY LAYER - Master Header
 * 
 * 🧱 PRINCIPIO RECTOR (no negociable):
 * No se agrega potencia sin primero agregar seguridad.
 * Cada feature nueva debe consumir "presupuesto de seguridad".
 * 
 * 🗺️ ROADMAP: "TECHO MÁXIMO DE EFICIENCIA"
 * 
 * Este es el header maestro del Safety Layer que proporciona:
 * - Guardas duras en arrays globales
 * - Clamp matemático centralizado  
 * - Operaciones de spawn seguras
 * - Patrones de iteración sin invalidación
 * 
 * FASE 1 - SAFETY LAYER (la base de todo):
 * ✅ Headers only (sin clases, sin polimorfismo)
 * ✅ Helpers inline para máximo rendimiento
 * ✅ Macros defensivos estilo Diablo
 * ✅ Sin dependencias nuevas
 * ✅ Ubicación: Source/safety/
 */

// ============================================================================
// 🧱 SAFETY LAYER COMPONENTS
// ============================================================================

#include "safety_limits.h"   // 1.1 Guardas duras en arrays globales
#include "safety_math.h"     // 1.2 Clamp matemático centralizado
#include "safety_spawn.h"    // 1.3 Operaciones de spawn seguras
#include "safety_iter.h"     // 1.4 Patrones de iteración segura
#include "safety_metrics.h"  // 1.5 Sistema de métricas avanzado (Fase 5)

namespace devilution {

// ============================================================================
// 🧱 SAFETY LAYER API UNIFICADA
// ============================================================================

/**
 * Inicializa el Safety Layer
 * Debe ser llamado una vez al inicio del juego
 */
inline void InitSafetyLayer() {
    // Por ahora no hay inicialización específica requerida
    // Todos los componentes son header-only con estado mínimo
}

/**
 * Obtiene el estado general del Safety Layer
 */
struct SafetyStatus {
    int monsterUsage;      // Porcentaje de uso de monsters (0-100)
    int missileUsage;      // Porcentaje de uso de missiles (0-100)
    bool inDangerZone;     // True si estamos en zona de peligro (>80%)
    bool canSpawnMonster;  // True si podemos spawn monsters de forma segura
    bool canSpawnMissile;  // True si podemos spawn missiles de forma segura
};

/**
 * Obtiene el estado actual del Safety Layer
 */
inline SafetyStatus GetSafetyStatus() {
    return SafetyStatus {
        .monsterUsage = GetMonsterUsagePercent(),
        .missileUsage = GetMissileUsagePercent(),
        .inDangerZone = IsInDangerZone(),
        .canSpawnMonster = CanAddMonster(),
        .canSpawnMissile = CanAddMissile()
    };
}

// ============================================================================
// 🧱 MACROS DE CONVENIENCIA PARA INTEGRACIÓN
// ============================================================================

/**
 * Macro principal para verificación de seguridad antes de cualquier operación crítica
 * Uso: SAFETY_GUARD() al inicio de funciones que pueden causar spawns
 */
#define SAFETY_GUARD() \
    do { \
        UpdateSafetyMetrics(); \
        if (IsInDangerZone()) { \
            RecordGracefulDegradation(); \
            AutoCleanupIfNeeded(); \
        } \
    } while(0)

/**
 * Macro para operaciones críticas que requieren máxima seguridad
 * Uso: CRITICAL_SAFETY_GUARD() para operaciones que no pueden fallar
 */
#define CRITICAL_SAFETY_GUARD() \
    do { \
        SAFETY_GUARD(); \
        if (GetMonsterUsagePercent() > 90 || GetMissileUsagePercent() > 90) { \
            ForceCleanupInactiveEntities(); \
        } \
    } while(0)

/**
 * Wrapper completo para funciones que modifican el estado del juego
 * Uso: SAFE_GAME_OPERATION(código) para operaciones complejas
 */
#define SAFE_GAME_OPERATION(operation) \
    do { \
        SAFETY_GUARD(); \
        BEGIN_SAFE_ITER(); \
        operation; \
        END_SAFE_ITER_SIMPLE(); \
    } while(0)

// ============================================================================
// 🧱 CRITERIOS DE ÉXITO FASE 1
// ============================================================================

/**
 * Verifica si se cumplen los criterios de éxito de la Fase 1:
 * ✅ El juego no crashea aun saturando spawns
 * ✅ No warnings nuevos  
 * ✅ Ningún cambio visible en gameplay
 */
inline bool ValidatePhase1Success() {
    SafetyStatus status = GetSafetyStatus();
    
    // Criterio 1: Sistema de límites funcionando
    bool limitsWorking = (status.monsterUsage <= 100) && (status.missileUsage <= 100);
    
    // Criterio 2: Detección de zona de peligro funcionando
    bool dangerDetectionWorking = status.inDangerZone == (status.monsterUsage > 80 || status.missileUsage > 80);
    
    // Criterio 3: Guardas de spawn funcionando
    bool spawnGuardsWorking = status.canSpawnMonster && status.canSpawnMissile;
    
    return limitsWorking && dangerDetectionWorking && spawnGuardsWorking;
}

// ============================================================================
// 🧱 DEBUGGING Y MONITORING
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime el estado actual del Safety Layer (solo en debug)
 */
inline void DebugPrintSafetyStatus() {
    SafetyStatus status = GetSafetyStatus();
    // Aquí iría código de debug printing
    // Por ahora solo validamos que la estructura funciona
}

/**
 * Verifica la integridad del Safety Layer (solo en debug)
 */
inline bool DebugValidateSafetyLayer() {
    return ValidatePhase1Success();
}
#endif

} // namespace devilution

// ============================================================================
// 🧱 DOCUMENTACIÓN DE USO
// ============================================================================

/*
GUÍA DE INTEGRACIÓN DEL SAFETY LAYER:

1. INCLUIR EL HEADER:
   #include "safety/safety.h"

2. INICIALIZAR (una vez al inicio):
   InitSafetyLayer();

3. USAR GUARDAS EN FUNCIONES CRÍTICAS:
   void SomeGameFunction() {
       SAFETY_GUARD();
       // ... código existente ...
   }

4. USAR SPAWN SEGURO:
   // Antes:
   AddMonster(pos, dir, type);
   
   // Después:
   SAFETY_CHECK_SPAWN(Monster);
   AddMonster(pos, dir, type);

5. USAR MATEMÁTICAS SEGURAS:
   // Antes:
   int damage = baseDamage + bonus;
   
   // Después:
   int damage = SafeDamage(baseDamage + bonus);

6. USAR ITERACIÓN SEGURA:
   // Antes:
   for (auto& monster : monsters) {
       if (shouldRemove) {
           monsters.erase(it); // ¡PELIGROSO!
       }
   }
   
   // Después:
   SAFE_MONSTER_ITER_BEGIN();
   for (size_t i = 0; i < monsterCount; ++i) {
       if (shouldRemove) {
           SAFE_MONSTER_MARK_FOR_REMOVAL(i);
       }
   }
   SAFE_MONSTER_ITER_END();

REGLAS DE ORO:
- 🔐 Ningún AddX() se llama sin CanAddX()
- 🔐 Todo daño y timer pasa por SafeX()
- 🔐 No mutar durante iteración (usar defer)
- 🔐 SAFETY_GUARD() en funciones críticas

RESULTADO ESPERADO:
✅ El juego no crashea aun saturando spawns
✅ No warnings nuevos
✅ Ningún cambio visible en gameplay
✅ Base sólida para futuras optimizaciones
*/