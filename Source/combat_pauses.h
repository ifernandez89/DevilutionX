#pragma once

/**
 * @file combat_pauses.h
 * 
 * 🎯 COMBAT PAUSES SYSTEM (FEATURE #2)
 * 
 * OBJETIVO:
 * Crear pausas naturales después de combates grandes para dar
 * ritmo al juego y evitar sensación arcade constante.
 * 
 * REGLAS:
 * - Después de combates grandes: silencio, ningún spawn inmediato
 * - Cooldown global de spawns: 5-8 segundos
 * - No visible, completamente invisible al jugador
 * - Tiempo para respirar entre combates intensos
 * 
 * IMPLEMENTACIÓN:
 * - Timestamp del último combate grande
 * - Check simple antes de spawn
 * - No HUD, no flags visibles
 * - Ritmo natural, no arcade
 */

#include <cstdint>

namespace devilution {

// ============================================================================
// ⚔️ COMBAT PAUSES CONSTANTS
// ============================================================================

namespace CombatPauses {
    constexpr uint32_t MIN_PAUSE_MS = 5000;     // 5 seconds minimum pause
    constexpr uint32_t MAX_PAUSE_MS = 8000;     // 8 seconds maximum pause
    constexpr int LARGE_COMBAT_THRESHOLD = 3;   // 3+ monsters killed = large combat
    constexpr uint32_t COMBAT_WINDOW_MS = 10000; // 10 seconds window to count kills
    constexpr int MAX_RECENT_KILLS = 10;        // Track last 10 kills for efficiency
}

// ============================================================================
// ⚔️ COMBAT TRACKING STRUCTURES
// ============================================================================

struct KillRecord {
    uint32_t timestamp;
    int monsterId;
    bool wasElite;
};

struct CombatPausesState {
    bool enabled = true;
    bool inCombatPause = false;
    uint32_t pauseStartTime = 0;
    uint32_t pauseDuration = 0;
    uint32_t lastLargeCombatTime = 0;
    
    // Kill tracking for combat detection
    KillRecord recentKills[CombatPauses::MAX_RECENT_KILLS];
    int killIndex = 0;
    int totalKills = 0;
};

// ============================================================================
// ⚔️ COMBAT PAUSES API
// ============================================================================

/**
 * Initialize the combat pauses system
 */
void InitCombatPauses();

/**
 * Update the combat pauses system (call every frame)
 */
void UpdateCombatPauses();

/**
 * Record a monster kill for combat tracking
 * @param monsterId The ID of the killed monster
 * @param wasElite Whether the monster was elite/unique
 */
void RecordMonsterKill(int monsterId, bool wasElite);

/**
 * Check if spawning should be allowed right now
 * @return true if spawning is allowed, false if in combat pause
 */
bool CanSpawnMonsters();

/**
 * Check if we're currently in a large combat situation
 * @return true if recent kills indicate large combat
 */
bool IsInLargeCombat();

/**
 * Get the number of recent kills in the combat window
 * @return number of kills in the last COMBAT_WINDOW_MS milliseconds
 */
int GetRecentKillCount();

/**
 * Force end any current combat pause (for level transitions, etc.)
 */
void CombatPauses_ForceEnd();

/**
 * Get current state (for debugging)
 */
const CombatPausesState& GetCombatPausesState();

} // namespace devilution