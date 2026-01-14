#pragma once

/**
 * @file safety_spawn.h
 * 
 * SAFETY LAYER - Phase 1: Safe Spawning Operations
 * 
 * PRINCIPIO RECTOR: Ningún spawn se ejecuta sin verificación previa.
 * 
 * Este header proporciona wrappers seguros para operaciones de spawn.
 * - Verificación automática de límites antes de spawn
 * - Fallback graceful en caso de límites alcanzados
 * - Logging opcional para debugging
 * - Compatibilidad total con código existente
 */

#include "safety_limits.h"
#include <functional>
#include <list>

namespace devilution {

// Forward declarations para evitar dependencias circulares
class Missile;
extern size_t ActiveMonsterCount;
extern std::list<Missile> Missiles;

// ============================================================================
// 🧱 1.3 WRAPPERS SEGUROS PARA SPAWN OPERATIONS
// ============================================================================

/**
 * Wrapper seguro para AddMonster que verifica límites automáticamente
 * Retorna nullptr si no se puede agregar el monster de forma segura
 */
template<typename AddMonsterFunc>
inline auto SafeAddMonster(AddMonsterFunc&& addFunc) -> decltype(addFunc()) {
    SAFETY_CHECK_SPAWN_RET(Monster, nullptr);
    return addFunc();
}

/**
 * Wrapper seguro para spawn de missiles
 * Retorna false si no se puede agregar el missile de forma segura
 */
template<typename AddMissileFunc>
inline bool SafeAddMissile(AddMissileFunc&& addFunc) {
    SAFETY_CHECK_SPAWN_RET(Missile, false);
    addFunc();
    return true;
}

/**
 * Wrapper seguro para operaciones de spawn múltiple
 * Verifica que se puedan agregar todos los elementos antes de proceder
 */
template<typename SpawnFunc>
inline bool SafeMultipleSpawn(size_t monsterCount, size_t missileCount, SpawnFunc&& spawnFunc) {
    // Verificar que podemos agregar todos los elementos de forma segura
    if (!CanAddMonsters(monsterCount) || !CanAddMissiles(missileCount)) {
        return false; // No proceder si no hay espacio suficiente
    }
    
    spawnFunc();
    return true;
}

// ============================================================================
// 🧱 HELPERS PARA SPAWN CONDICIONAL
// ============================================================================

/**
 * Ejecuta spawn solo si estamos por debajo del umbral de peligro
 * Útil para spawns opcionales que pueden ser omitidos bajo presión
 */
template<typename SpawnFunc>
inline bool SafeOptionalSpawn(SpawnFunc&& spawnFunc) {
    if (IsInDangerZone()) {
        return false; // Omitir spawn opcional si estamos en zona de peligro
    }
    
    spawnFunc();
    return true;
}

/**
 * Spawn con prioridad - siempre intenta ejecutar pero con verificación
 * Útil para spawns críticos que deben intentarse siempre
 */
template<typename SpawnFunc>
inline bool SafePrioritySpawn(SpawnFunc&& spawnFunc) {
    // Para spawns prioritarios, usamos un buffer más pequeño
    constexpr size_t PRIORITY_BUFFER = 5;
    
    if (ActiveMonsterCount >= (200 - PRIORITY_BUFFER)) {
        return false; // Incluso spawns prioritarios tienen límites
    }
    
    spawnFunc();
    return true;
}

// ============================================================================
// 🧱 SPAWN THROTTLING Y RATE LIMITING
// ============================================================================

/**
 * Estructura para controlar la tasa de spawn
 */
struct SpawnThrottle {
    uint32_t lastSpawnTime = 0;
    uint32_t minInterval = 100; // Mínimo intervalo entre spawns en ms
    
    bool CanSpawnNow(uint32_t currentTime) const {
        return (currentTime - lastSpawnTime) >= minInterval;
    }
    
    void RecordSpawn(uint32_t currentTime) {
        lastSpawnTime = currentTime;
    }
};

/**
 * Spawn con throttling temporal
 * Previene spam de spawns en corto tiempo
 */
template<typename SpawnFunc>
inline bool SafeThrottledSpawn(SpawnThrottle& throttle, uint32_t currentTime, SpawnFunc&& spawnFunc) {
    if (!throttle.CanSpawnNow(currentTime)) {
        return false; // Demasiado pronto para otro spawn
    }
    
    SAFETY_CHECK_SPAWN_RET(Monster, false);
    
    spawnFunc();
    throttle.RecordSpawn(currentTime);
    return true;
}

// ============================================================================
// 🧱 CLEANUP Y GARBAGE COLLECTION
// ============================================================================

/**
 * Fuerza limpieza de entities inactivos para hacer espacio
 * Útil cuando nos acercamos a los límites
 */
inline void ForceCleanupInactiveEntities() {
    // Esta función será implementada para limpiar monsters/missiles inactivos
    // cuando nos acerquemos a los límites de capacidad
    
    // TODO: Implementar limpieza de monsters con HP <= 0
    // TODO: Implementar limpieza de missiles expirados
    // TODO: Implementar limpieza de efectos temporales
}

/**
 * Limpieza automática basada en uso de memoria
 */
inline void AutoCleanupIfNeeded() {
    if (GetMonsterUsagePercent() > 85 || GetMissileUsagePercent() > 85) {
        ForceCleanupInactiveEntities();
    }
}

// ============================================================================
// 🧱 MACROS DE CONVENIENCIA
// ============================================================================

/**
 * Macro para spawn seguro con cleanup automático
 */
#define SAFE_SPAWN_WITH_CLEANUP(type, spawnCode) \
    do { \
        AutoCleanupIfNeeded(); \
        SAFETY_CHECK_SPAWN(type); \
        spawnCode; \
    } while(0)

/**
 * Macro para spawn condicional basado en carga del sistema
 */
#define SAFE_CONDITIONAL_SPAWN(condition, spawnCode) \
    do { \
        if ((condition) && !IsInDangerZone()) { \
            spawnCode; \
        } \
    } while(0)

} // namespace devilution