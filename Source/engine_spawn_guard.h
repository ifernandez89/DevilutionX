#pragma once

/**
 * @file engine_spawn_guard.h
 * 
 * GUARDIÁN DE SPAWN - DISEÑO ULTRA SIMPLE
 * 
 * PRINCIPIO: "Diablo no necesita protección inteligente, necesita límites tontos"
 * 
 * - Sin estados globales
 * - Sin contadores cruzados  
 * - Sin memoria entre calls
 * - Solo límites directos en puntos de presión
 */

namespace devilution {

/**
 * Resultado de intento de spawn - ultra simple
 */
enum class SpawnResult {
    Ok,
    LimitReached,
};

/**
 * Guardián genérico de spawn - template ultra simple
 * 
 * @param pool - Container a verificar (Missiles, Monsters, etc)
 * @param max - Límite máximo permitido
 * @return SpawnResult - Ok o LimitReached
 */
template <typename Pool>
inline SpawnResult TrySpawn(Pool &pool, int max) {
    if (pool.size() >= max)
        return SpawnResult::LimitReached;
    return SpawnResult::Ok;
}

/**
 * Límites específicos para cada subsistema
 * Estos son los únicos números que importan
 */
constexpr int MAXMISSILES = 500;  // Límite conservador pero generoso
constexpr int MAXMONSTERS = 200;  // Límite original de Diablo
constexpr int MAXLIGHTS = 32;     // Límite de luces activas

} // namespace devilution