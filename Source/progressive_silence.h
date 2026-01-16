#pragma once

/**
 * @file progressive_silence.h
 * 
 * 🎯 PROGRESSIVE SILENCE SYSTEM
 * 
 * OBJETIVO:
 * Aumentar tensión psicológica mediante silencio progresivo
 * a mayor profundidad del dungeon.
 * 
 * REGLAS:
 * - Level 1-4: Música normal
 * - Level 5-8: Música con pausas aleatorias
 * - Level 9+: Música muy rara o nula
 * 
 * IMPLEMENTACIÓN:
 * - Timer que no reinicia música inmediatamente
 * - Delay aleatorio antes de replay
 * - No HUD, no flags visibles
 * - Todo es sensación pura
 */

#include <cstdint>
#include "engine/sound.h"

namespace devilution {

// ============================================================================
// 🎵 PROGRESSIVE SILENCE CONSTANTS
// ============================================================================

namespace ProgressiveSilence {
    constexpr int NORMAL_LEVELS = 4;        // Levels 1-4: normal music
    constexpr int PAUSE_LEVELS = 8;         // Levels 5-8: music with pauses
    constexpr int DEEP_LEVELS = 9;          // Level 9+: rare or no music
    
    constexpr uint32_t MIN_PAUSE_MS = 3000;     // 3 seconds minimum pause
    constexpr uint32_t MAX_PAUSE_MS = 12000;    // 12 seconds maximum pause
    constexpr uint32_t DEEP_MIN_PAUSE_MS = 8000;  // 8 seconds minimum in deep levels
    constexpr uint32_t DEEP_MAX_PAUSE_MS = 30000; // 30 seconds maximum in deep levels
    
    constexpr int DEEP_MUSIC_CHANCE = 25;   // 25% chance to play music in deep levels
}

// ============================================================================
// 🎵 PROGRESSIVE SILENCE STATE
// ============================================================================

struct ProgressiveSilenceState {
    bool enabled = true;
    bool musicPaused = false;
    uint32_t pauseStartTime = 0;
    uint32_t pauseDuration = 0;
    _music_id pendingTrack = NUM_MUSIC;
    bool deepLevelSilenceMode = false;
};

// ============================================================================
// 🎵 PROGRESSIVE SILENCE API
// ============================================================================

/**
 * Initialize the progressive silence system
 */
void InitProgressiveSilence();

/**
 * Update the progressive silence system (call every frame)
 */
void UpdateProgressiveSilence();

/**
 * Start music with progressive silence consideration
 * This replaces direct music_start calls for level music
 */
void ProgressiveSilence_StartMusic(_music_id track);

/**
 * Check if music should be started immediately or delayed
 */
bool ShouldStartMusicImmediately();

/**
 * Get the current silence behavior based on level depth
 */
int GetSilenceBehaviorForLevel(int level);

/**
 * Force stop any pending music (for menu transitions, etc.)
 */
void ProgressiveSilence_ForceStop();

/**
 * Get current state (for debugging)
 */
const ProgressiveSilenceState& GetProgressiveSilenceState();

} // namespace devilution