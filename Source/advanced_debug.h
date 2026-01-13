#pragma once

/**
 * @file advanced_debug.h
 * 
 * 🎮 FEATURE #8: ADVANCED DEBUG SYSTEM (INFERNO MASTER EDITION)
 * 
 * OBJETIVO:
 * Sistema de información avanzado en tiempo real para jugadores expertos
 * que muestra stats detallados de monstruos, combate, y features atmosféricas.
 * 
 * FILOSOFÍA:
 * "Información es poder. Los jugadores avanzados merecen ver qué está 
 *  pasando bajo el capó del juego."
 * 
 * CARACTERÍSTICAS:
 * ✅ Información de monstruos en tiempo real
 * ✅ Stats de combate y DPS
 * ✅ Estado de features atmosféricas
 * ✅ Hotkey toggle (F12)
 * ✅ Filtros por categoría
 * ✅ Colores para diferentes tipos de info
 */

#include <cstdint>
#include <string>

namespace devilution {

// Forward declarations
struct Monster;
struct Player;

// ============================================================================
// 🎮 ADVANCED DEBUG SYSTEM CONSTANTS
// ============================================================================

namespace AdvancedDebug {
    // Debug categories
    constexpr int CATEGORY_MONSTERS = 1 << 0;    // Monster information
    constexpr int CATEGORY_PLAYER = 1 << 1;      // Player stats
    constexpr int CATEGORY_COMBAT = 1 << 2;      // Combat metrics
    constexpr int CATEGORY_ATMOSPHERE = 1 << 3;  // Atmospheric features
    constexpr int CATEGORY_LEVEL = 1 << 4;       // Level information
    constexpr int CATEGORY_ALL = 0xFF;           // All categories
    
    // Update frequencies (in ticks)
    constexpr int MONSTER_UPDATE_FREQ = 90;      // ~1.5 seconds
    constexpr int PLAYER_UPDATE_FREQ = 120;      // ~2 seconds
    constexpr int COMBAT_UPDATE_FREQ = 90;       // ~1.5 seconds
    constexpr int ATMOSPHERE_UPDATE_FREQ = 180;  // ~3 seconds
    
    // Display limits
    constexpr int MAX_MONSTERS_SHOWN = 8;        // Max monsters to show
    constexpr int MAX_DISTANCE_SHOWN = 15;       // Max distance to show monsters
}

// ============================================================================
// 🎮 DEBUG SYSTEM STATE
// ============================================================================

struct AdvancedDebugState {
    // System state
    bool enabled = false;
    int activeCategories = AdvancedDebug::CATEGORY_ALL;
    
    // Update timers
    int monsterUpdateTimer = 0;
    int playerUpdateTimer = 0;
    int combatUpdateTimer = 0;
    int atmosphereUpdateTimer = 0;
    
    // Combat tracking
    int combatStartTime = 0;
    int totalDamageDealt = 0;
    int totalKills = 0;
    int lastKillTime = 0;
    std::string lastKilledMonster = "";
    
    // Performance tracking
    int frameCount = 0;
    int lastFPSUpdate = 0;
    int currentFPS = 0;
};

// ============================================================================
// 🎮 ADVANCED DEBUG SYSTEM API
// ============================================================================

/**
 * Initialize the advanced debug system
 */
void InitAdvancedDebug();

/**
 * Update the advanced debug system (called every frame)
 */
void UpdateAdvancedDebug();

/**
 * Toggle the debug system on/off
 */
void ToggleAdvancedDebug();

/**
 * Toggle specific debug category
 * @param category Category to toggle (use AdvancedDebug::CATEGORY_* constants)
 */
void ToggleDebugCategory(int category);

/**
 * Check if debug system is enabled
 * @return true if debug system is active
 */
bool IsAdvancedDebugEnabled();

/**
 * Display monster information
 */
void ShowMonsterDebugInfo();

/**
 * Display player information
 */
void ShowPlayerDebugInfo();

/**
 * Display combat metrics
 */
void ShowCombatDebugInfo();

/**
 * Display atmospheric features status
 */
void ShowAtmosphereDebugInfo();

/**
 * Display level information
 */
void ShowLevelDebugInfo();

/**
 * Record damage dealt (for DPS calculation)
 * @param damage Amount of damage dealt
 */
void RecordDamageDealt(int damage);

/**
 * Record monster kill (for kill tracking)
 * @param monsterName Name of killed monster
 */
void RecordMonsterKill(const std::string& monsterName);

/**
 * Get monster AI state name
 * @param monster The monster to check
 * @return human-readable AI state name
 */
std::string GetMonsterAIStateName(const Monster& monster);

/**
 * Get monster mutation info
 * @param monster The monster to check
 * @return mutation description or empty string
 */
std::string GetMonsterMutationInfo(const Monster& monster);

/**
 * Calculate distance between monster and player
 * @param monster The monster
 * @return distance in tiles
 */
int GetMonsterPlayerDistance(const Monster& monster);

} // namespace devilution