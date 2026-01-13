#include "combat_pauses.h"
#include "diablo.h"
#include "utils/log.hpp"
#include "engine/random.hpp"

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// ⚔️ GLOBAL STATE
// ============================================================================

static CombatPausesState g_combatPauses;

// ============================================================================
// ⚔️ IMPLEMENTATION
// ============================================================================

void InitCombatPauses()
{
    g_combatPauses = CombatPausesState{};
    g_combatPauses.enabled = true;
    
    // Initialize kill records
    for (int i = 0; i < CombatPauses::MAX_RECENT_KILLS; i++) {
        g_combatPauses.recentKills[i] = KillRecord{0, -1, false};
    }
    
    LogVerbose("⚔️ Combat Pauses System initialized");
}

int GetRecentKillCount()
{
    if (!g_combatPauses.enabled) return 0;
    
    uint32_t currentTime = SDL_GetTicks();
    int recentCount = 0;
    
    for (int i = 0; i < CombatPauses::MAX_RECENT_KILLS; i++) {
        const KillRecord& kill = g_combatPauses.recentKills[i];
        if (kill.timestamp == 0) continue; // Empty slot
        
        uint32_t killAge = currentTime - kill.timestamp;
        if (killAge <= CombatPauses::COMBAT_WINDOW_MS) {
            recentCount++;
        }
    }
    
    return recentCount;
}

bool IsInLargeCombat()
{
    return GetRecentKillCount() >= CombatPauses::LARGE_COMBAT_THRESHOLD;
}

void RecordMonsterKill(int monsterId, bool wasElite)
{
    if (!g_combatPauses.enabled) return;
    
    uint32_t currentTime = SDL_GetTicks();
    
    // Record the kill in circular buffer
    KillRecord& record = g_combatPauses.recentKills[g_combatPauses.killIndex];
    record.timestamp = currentTime;
    record.monsterId = monsterId;
    record.wasElite = wasElite;
    
    // Advance circular buffer index
    g_combatPauses.killIndex = (g_combatPauses.killIndex + 1) % CombatPauses::MAX_RECENT_KILLS;
    g_combatPauses.totalKills++;
    
    // Check if this kill triggers a large combat pause
    int recentKills = GetRecentKillCount();
    
    LogVerbose("⚔️ Monster kill recorded: ID={}, Elite={}, Recent kills={}", 
              monsterId, wasElite, recentKills);
    
    // If we just reached the large combat threshold, start a pause
    if (recentKills >= CombatPauses::LARGE_COMBAT_THRESHOLD && 
        !g_combatPauses.inCombatPause) {
        
        // Only start pause if enough time has passed since last large combat
        uint32_t timeSinceLastCombat = currentTime - g_combatPauses.lastLargeCombatTime;
        if (timeSinceLastCombat > CombatPauses::MAX_PAUSE_MS) {
            
            g_combatPauses.inCombatPause = true;
            g_combatPauses.pauseStartTime = currentTime;
            g_combatPauses.pauseDuration = CombatPauses::MIN_PAUSE_MS + 
                GenerateRnd(CombatPauses::MAX_PAUSE_MS - CombatPauses::MIN_PAUSE_MS);
            g_combatPauses.lastLargeCombatTime = currentTime;
            
            LogVerbose("⚔️ Large combat detected! Starting {}ms spawn pause", 
                      g_combatPauses.pauseDuration);
        }
    }
}

bool CanSpawnMonsters()
{
    if (!g_combatPauses.enabled) return true;
    if (!g_combatPauses.inCombatPause) return true;
    
    // We're in a combat pause - check if it should end
    uint32_t currentTime = SDL_GetTicks();
    uint32_t elapsed = currentTime - g_combatPauses.pauseStartTime;
    
    if (elapsed >= g_combatPauses.pauseDuration) {
        // Pause is over
        g_combatPauses.inCombatPause = false;
        g_combatPauses.pauseStartTime = 0;
        g_combatPauses.pauseDuration = 0;
        
        LogVerbose("⚔️ Combat pause ended, spawning resumed");
        return true;
    }
    
    // Still in pause
    return false;
}

void UpdateCombatPauses()
{
    if (!g_combatPauses.enabled) return;
    
    // The main work is done in CanSpawnMonsters() when checked
    // This function could be used for additional periodic cleanup
    
    // Clean up very old kill records (older than 2x combat window)
    uint32_t currentTime = SDL_GetTicks();
    uint32_t cleanupThreshold = CombatPauses::COMBAT_WINDOW_MS * 2;
    
    for (int i = 0; i < CombatPauses::MAX_RECENT_KILLS; i++) {
        KillRecord& kill = g_combatPauses.recentKills[i];
        if (kill.timestamp == 0) continue;
        
        uint32_t killAge = currentTime - kill.timestamp;
        if (killAge > cleanupThreshold) {
            kill.timestamp = 0; // Mark as empty
            kill.monsterId = -1;
            kill.wasElite = false;
        }
    }
}

void CombatPauses_ForceEnd()
{
    g_combatPauses.inCombatPause = false;
    g_combatPauses.pauseStartTime = 0;
    g_combatPauses.pauseDuration = 0;
    
    LogVerbose("⚔️ Combat pause force ended");
}

const CombatPausesState& GetCombatPausesState()
{
    return g_combatPauses;
}

} // namespace devilution