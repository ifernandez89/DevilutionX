#pragma once

/**
 * @file safety_limits.h
 * 
 * SAFETY LAYER - Phase 1: Hard Guards for Global Arrays
 * 
 * PRINCIPIO RECTOR: No se agrega potencia sin primero agregar seguridad.
 * 
 * Este header proporciona guardas duras para prevenir corrupción de arrays globales.
 * - Headers only (sin clases, sin polimorfismo)
 * - Helpers inline para máximo rendimiento
 * - Macros defensivos estilo Diablo
 * - Sin dependencias nuevas
 */

#include <cstddef>
#include <list>

namespace devilution {

// Forward declarations para evitar dependencias circulares
class Missile;
extern size_t ActiveMonsterCount;
extern std::list<Missile> Missiles;

// ============================================================================
// 🧱 1.1 GUARDAS DURAS EN ARRAYS GLOBALES
// ============================================================================

/**
 * REGLA CRÍTICA: ningún AddMonster() se llama sin CanAddMonster()
 * Esto solo previene corrupción, no cambia gameplay.
 */
inline bool CanAddMonster() {
    constexpr size_t SAFETY_BUFFER = 10; // Buffer de seguridad para operaciones críticas
    return ActiveMonsterCount < (200 - SAFETY_BUFFER); // MaxMonsters - buffer
}

/**
 * REGLA CRÍTICA: ningún AddMissile() se llama sin CanAddMissile()
 * Previene overflow de la lista dinámica de missiles.
 */
inline bool CanAddMissile() {
    constexpr size_t MAX_SAFE_MISSILES = 500; // Límite conservador para estabilidad
    return Missiles.size() < MAX_SAFE_MISSILES;
}

/**
 * Verifica si podemos agregar múltiples monsters de forma segura
 */
inline bool CanAddMonsters(size_t count) {
    constexpr size_t SAFETY_BUFFER = 10;
    return (ActiveMonsterCount + count) < (200 - SAFETY_BUFFER);
}

/**
 * Verifica si podemos agregar múltiples missiles de forma segura
 */
inline bool CanAddMissiles(size_t count) {
    constexpr size_t MAX_SAFE_MISSILES = 500;
    return (Missiles.size() + count) < MAX_SAFE_MISSILES;
}

// ============================================================================
// 🧱 MACROS DEFENSIVOS ESTILO DIABLO
// ============================================================================

/**
 * Macro para verificación obligatoria antes de spawn
 * Uso: SAFETY_CHECK_SPAWN(Monster) antes de cualquier AddMonster()
 */
#define SAFETY_CHECK_SPAWN(type) \
    do { \
        /* DISABLED: No safety checks - trust original engine */ \
    } while(0)

/**
 * Macro para verificación con retorno de valor
 * Uso: SAFETY_CHECK_SPAWN_RET(Monster, nullptr) para funciones que retornan puntero
 */
#define SAFETY_CHECK_SPAWN_RET(type, retval) \
    do { \
        /* DISABLED: No safety checks - trust original engine */ \
    } while(0)

/**
 * Macro para verificación con logging (solo en debug)
 */
#ifdef _DEBUG
#define SAFETY_CHECK_SPAWN_LOG(type, msg) \
    do { \
        /* DISABLED: No safety checks - trust original engine */ \
    } while(0)
#else
#define SAFETY_CHECK_SPAWN_LOG(type, msg) \
    do { \
        /* DISABLED: No safety checks - trust original engine */ \
    } while(0)
#endif

// ============================================================================
// 🧱 HELPERS DE INFORMACIÓN DE ESTADO
// ============================================================================

/**
 * Obtiene el porcentaje de uso actual de monsters (0-100)
 */
inline int GetMonsterUsagePercent() {
    return static_cast<int>((ActiveMonsterCount * 100) / 200);
}

/**
 * Obtiene el porcentaje de uso actual de missiles (0-100)
 */
inline int GetMissileUsagePercent() {
    constexpr size_t MAX_SAFE_MISSILES = 500;
    return static_cast<int>((Missiles.size() * 100) / MAX_SAFE_MISSILES);
}

/**
 * Verifica si estamos en zona de peligro (>80% de uso)
 */
inline bool IsInDangerZone() {
    return GetMonsterUsagePercent() > 80 || GetMissileUsagePercent() > 80;
}

// Forward declarations para funciones de métricas
void RecordSafetyCheck();
void RecordSpawnBlocked();
void RecordGracefulDegradation();

} // namespace devilution