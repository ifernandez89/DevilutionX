#pragma once

/**
 * @file inferno_defense.h
 * 
 * 🔥 INFERNO DEFENSE SYSTEM
 * 
 * PROBLEMA IDENTIFICADO:
 * Inferno + muchos enemigos = combinatorial explosion O(N²)
 * - ProcessInfernoControl spawna missiles por tile
 * - ProcessInferno hace collision checks por frame  
 * - CheckMissileCol itera sobre todos los enemigos
 * - Resultado: crash determinístico por saturación lógica
 * 
 * SOLUCIÓN ARQUITECTÓNICA:
 * "Degradación elegante invisible" - mantiene intensidad visual,
 * controla carga computacional, nunca crashea.
 * 
 * PRINCIPIOS DE DISEÑO:
 * 1. NUNCA crashear - degradación antes que fallo
 * 2. MANTENER intensidad visual - se ve brutal siempre
 * 3. CONTROLAR carga - límites matemáticos seguros
 * 4. INVISIBLE al jugador - experiencia intacta
 * 5. CONSERVAR esencia - Inferno sigue siendo Inferno
 */

#include <cstdint>
#include "engine/point.hpp"
#include "missiles.h"

namespace devilution {

// ============================================================================
// 🔥 INFERNO DEFENSE CONSTANTS
// ============================================================================

namespace InfernoDefense {
    // Límites seguros para prevenir explosion combinatorial
    constexpr int MAX_INFERNO_MISSILES_PER_FRAME = 8;     // Máximo spawn por frame
    constexpr int MAX_INFERNO_COLLISION_CHECKS = 50;      // Máximo collision checks por frame
    constexpr int MAX_INFERNO_DAMAGE_APPLICATIONS = 25;   // Máximo damage applications por frame
    constexpr int INFERNO_THROTTLE_THRESHOLD = 15;        // Enemigos para activar throttling
    constexpr int INFERNO_EMERGENCY_THRESHOLD = 25;       // Enemigos para modo emergencia
    
    // Timers para evitar spam por frame
    constexpr int INFERNO_SPAWN_COOLDOWN_MS = 50;         // 50ms entre spawns masivos
    constexpr int INFERNO_DAMAGE_COOLDOWN_MS = 33;        // 33ms entre damage applications
    
    // Factores de intensidad visual vs computacional
    constexpr float VISUAL_INTENSITY_FACTOR = 1.0f;       // Mantener intensidad visual
    constexpr float COMPUTATIONAL_REDUCTION_FACTOR = 0.6f; // Reducir carga computacional
}

// ============================================================================
// 🔥 INFERNO DEFENSE STATE
// ============================================================================

struct InfernoDefenseState {
    // Contadores por frame
    int infernoMissilesSpawnedThisFrame = 0;
    int infernoCollisionChecksThisFrame = 0;
    int infernoDamageApplicationsThisFrame = 0;
    
    // Timers para throttling
    uint32_t lastInfernoSpawnTime = 0;
    uint32_t lastInfernoDamageTime = 0;
    
    // Estado del sistema
    bool isThrottlingActive = false;
    bool isEmergencyMode = false;
    int nearbyEnemyCount = 0;
    
    // Métricas para debugging (opcional)
    int totalInfernoMissilesPrevented = 0;
    int totalCollisionChecksPrevented = 0;
    int totalDamageApplicationsPrevented = 0;
};

// ============================================================================
// 🔥 INFERNO DEFENSE API
// ============================================================================

/**
 * Inicializa el sistema de defensa de Inferno
 * Llamar una vez al inicio del juego
 */
void InitInfernoDefense();

/**
 * Actualiza el estado del sistema por frame
 * Llamar al inicio de cada frame de juego
 */
void UpdateInfernoDefense();

/**
 * Verifica si podemos spawnar un missile de Inferno de forma segura
 * @param position Posición donde se quiere spawnar
 * @return true si es seguro spawnar, false si debe throttling
 */
bool CanSpawnInfernoMissile(Point position);

/**
 * Verifica si podemos hacer collision check de Inferno de forma segura
 * @param missile El missile de Inferno
 * @param position Posición del collision check
 * @return true si es seguro hacer check, false si debe throttling
 */
bool CanDoInfernoCollisionCheck(const Missile &missile, Point position);

/**
 * Verifica si podemos aplicar damage de Inferno de forma segura
 * @param damage Cantidad de daño a aplicar
 * @param position Posición del daño
 * @return true si es seguro aplicar, false si debe throttling
 */
bool CanApplyInfernoDamage(int damage, Point position);

/**
 * Registra que se spawneó un missile de Inferno
 * Para tracking y throttling
 */
void RegisterInfernoMissileSpawn();

/**
 * Registra que se hizo un collision check de Inferno
 * Para tracking y throttling
 */
void RegisterInfernoCollisionCheck();

/**
 * Registra que se aplicó damage de Inferno
 * Para tracking y throttling
 */
void RegisterInfernoDamageApplication();

/**
 * Cuenta enemigos cerca de una posición
 * @param center Centro de búsqueda
 * @param radius Radio de búsqueda
 * @return Número de enemigos en el área
 */
int CountNearbyEnemies(Point center, int radius = 5);

/**
 * Obtiene el estado actual del sistema de defensa
 * Para debugging y métricas
 */
const InfernoDefenseState& GetInfernoDefenseState();

/**
 * Resetea contadores por frame
 * Llamar al final de cada frame
 */
void ResetInfernoDefenseFrameCounters();

// ============================================================================
// 🔥 MACROS DE CONVENIENCIA
// ============================================================================

/**
 * Macro para verificación rápida antes de spawn de Inferno
 */
#define INFERNO_SAFE_SPAWN(pos) \
    (CanSpawnInfernoMissile(pos) ? (RegisterInfernoMissileSpawn(), true) : false)

/**
 * Macro para verificación rápida antes de collision check de Inferno
 */
#define INFERNO_SAFE_COLLISION(missile, pos) \
    (CanDoInfernoCollisionCheck(missile, pos) ? (RegisterInfernoCollisionCheck(), true) : false)

/**
 * Macro para verificación rápida antes de damage de Inferno
 */
#define INFERNO_SAFE_DAMAGE(damage, pos) \
    (CanApplyInfernoDamage(damage, pos) ? (RegisterInfernoDamageApplication(), true) : false)

} // namespace devilution