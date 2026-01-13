#pragma once

/**
 * @file waiting_enemies.h
 * 
 * 🎯 WAITING ENEMIES SYSTEM (FEATURE #3)
 * 
 * OBJETIVO:
 * Crear incertidumbre y miedo mediante enemigos que esperan antes de atacar.
 * Algunos monstruos no atacan al verte, esperan 1-2 segundos, se mueven lento.
 * 
 * REGLAS:
 * - Solo melee monsters (no ranged)
 * - Solo en rooms grandes (suficiente espacio)
 * - Máximo 1-2 por screen para mantener balance
 * - Esperan 1-2 segundos antes de cambiar a attack mode
 * - Se mueven lento hacia el jugador durante la espera
 * 
 * IMPLEMENTACIÓN:
 * - Flag simple: MONSTATE_AMBUSH
 * - Delay antes de cambiar a MONSTATE_ATTACK
 * - No HUD, no flags visibles
 * - Incertidumbre + miedo puro
 */

#include <cstdint>
#include "monster.h"

namespace devilution {

// ============================================================================
// 👁️ WAITING ENEMIES CONSTANTS
// ============================================================================

namespace WaitingEnemies {
    constexpr uint32_t MIN_WAIT_MS = 1000;      // 1 second minimum wait
    constexpr uint32_t MAX_WAIT_MS = 2000;      // 2 seconds maximum wait
    constexpr int MAX_WAITING_PER_SCREEN = 2;   // Maximum 2 waiting enemies per screen
    constexpr int MIN_ROOM_SIZE = 5;            // Minimum room size for waiting behavior
    constexpr float SLOW_MOVE_SPEED = 0.6f;     // 60% of normal speed during wait
    constexpr int DETECTION_RANGE = 8;          // Range to detect player and start waiting
}

// ============================================================================
// 👁️ WAITING ENEMY STATE
// ============================================================================

struct WaitingEnemyState {
    bool isWaiting = false;
    uint32_t waitStartTime = 0;
    uint32_t waitDuration = 0;
    Point originalTarget;
    bool hasSeenPlayer = false;
    float originalSpeed = 1.0f;
};

// ============================================================================
// 👁️ WAITING ENEMIES SYSTEM STATE
// ============================================================================

struct WaitingEnemiesSystemState {
    bool enabled = true;
    int currentWaitingCount = 0;
    uint32_t lastAssignmentTime = 0;
    int screenWaitingMonsters[MaxMonsters];  // Track which monsters are waiting
    int waitingCount = 0;
};

// ============================================================================
// 👁️ WAITING ENEMIES API
// ============================================================================

/**
 * Initialize the waiting enemies system
 */
void InitWaitingEnemies();

/**
 * Update the waiting enemies system (call every frame)
 */
void UpdateWaitingEnemies();

/**
 * Check if a monster should become a waiting enemy
 * @param monster The monster to check
 * @return true if monster should wait, false otherwise
 */
bool ShouldMonsterWait(Monster &monster);

/**
 * Start waiting behavior for a monster
 * @param monster The monster to make wait
 */
void StartWaitingBehavior(Monster &monster);

/**
 * Update waiting behavior for a monster
 * @param monster The monster to update
 * @return true if still waiting, false if should attack
 */
bool UpdateWaitingBehavior(Monster &monster);

/**
 * End waiting behavior for a monster
 * @param monster The monster to stop waiting
 */
void EndWaitingBehavior(Monster &monster);

/**
 * Check if monster is currently in waiting state
 * @param monster The monster to check
 * @return true if waiting, false otherwise
 */
bool IsMonsterWaiting(const Monster &monster);

/**
 * Check if room is large enough for waiting behavior
 * @param position The position to check
 * @return true if room is large enough
 */
bool IsRoomLargeEnough(Point position);

/**
 * Get current waiting enemies count on screen
 * @return number of monsters currently waiting
 */
int GetCurrentWaitingCount();

/**
 * Force end all waiting behaviors (for level transitions, etc.)
 */
void WaitingEnemies_ForceEndAll();

/**
 * Get current system state (for debugging)
 */
const WaitingEnemiesSystemState& GetWaitingEnemiesState();

} // namespace devilution