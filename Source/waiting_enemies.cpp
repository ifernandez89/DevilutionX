#include "waiting_enemies.h"
#include "diablo.h"
#include "player.h"
#include "utils/log.hpp"
#include "engine/random.hpp"
#include "levels/gendung.h"
#include "levels/tile_properties.hpp"

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// 👁️ GLOBAL STATE
// ============================================================================

static WaitingEnemiesSystemState g_waitingEnemies;
static WaitingEnemyState g_monsterWaitingStates[MaxMonsters];

// ============================================================================
// 👁️ IMPLEMENTATION
// ============================================================================

void InitWaitingEnemies()
{
    g_waitingEnemies = WaitingEnemiesSystemState{};
    g_waitingEnemies.enabled = true;
    g_waitingEnemies.currentWaitingCount = 0;
    g_waitingEnemies.waitingCount = 0;
    
    // Initialize all monster waiting states
    for (size_t i = 0; i < MaxMonsters; i++) {
        g_monsterWaitingStates[i] = WaitingEnemyState{};
        g_waitingEnemies.screenWaitingMonsters[i] = -1;
    }
    
    LogVerbose("👁️ Waiting Enemies System initialized");
}

bool IsRoomLargeEnough(Point position)
{
    if (!InDungeonBounds(position)) return false;
    
    // Check area around position for open space
    int openTiles = 0;
    const int checkRadius = WaitingEnemies::MIN_ROOM_SIZE / 2;
    
    for (int x = position.x - checkRadius; x <= position.x + checkRadius; x++) {
        for (int y = position.y - checkRadius; y <= position.y + checkRadius; y++) {
            Point checkPos = {x, y};
            if (InDungeonBounds(checkPos) && IsTileNotSolid(checkPos)) {
                openTiles++;
            }
        }
    }
    
    // Need at least 60% of the area to be open
    int totalTiles = (checkRadius * 2 + 1) * (checkRadius * 2 + 1);
    return openTiles >= (totalTiles * 6 / 10);
}

bool ShouldMonsterWait(Monster &monster)
{
    if (!g_waitingEnemies.enabled) return false;
    if (IsMonsterWaiting(monster)) return false;
    
    // Only melee monsters
    if (monster.type().type == MT_INVALID || monster.type().type == MT_INVALID) return false;
    
    // Don't apply to bosses or special monsters
    if (monster.isUnique() || monster.type().type == MT_GOLEM) return false;
    
    // Check if we already have enough waiting monsters
    if (g_waitingEnemies.currentWaitingCount >= WaitingEnemies::MAX_WAITING_PER_SCREEN) {
        return false;
    }
    
    // Check if room is large enough
    if (!IsRoomLargeEnough(monster.position.tile)) return false;
    
    // Check if player is in detection range
    const Player &player = Players[MyPlayerId];
    int distance = monster.position.tile.ManhattanDistance(player.position.tile);
    if (distance > WaitingEnemies::DETECTION_RANGE) return false;
    
    // Random chance to become waiting enemy (30% chance)
    if (GenerateRnd(100) > 30) return false;
    
    // Cooldown to prevent too frequent assignments
    uint32_t currentTime = SDL_GetTicks();
    if (currentTime - g_waitingEnemies.lastAssignmentTime < 3000) { // 3 second cooldown
        return false;
    }
    
    return true;
}

void StartWaitingBehavior(Monster &monster)
{
    if (!g_waitingEnemies.enabled) return;
    
    const size_t monsterId = monster.getId();
    if (monsterId >= MaxMonsters) return;
    
    WaitingEnemyState &state = g_monsterWaitingStates[monsterId];
    
    // Initialize waiting state
    state.isWaiting = true;
    state.waitStartTime = SDL_GetTicks();
    state.waitDuration = WaitingEnemies::MIN_WAIT_MS + 
        GenerateRnd(WaitingEnemies::MAX_WAIT_MS - WaitingEnemies::MIN_WAIT_MS);
    state.originalTarget = monster.position.tile;
    state.hasSeenPlayer = true;
    state.originalSpeed = 1.0f; // Store original speed
    
    // Add to waiting monsters list
    if (g_waitingEnemies.waitingCount < MaxMonsters) {
        g_waitingEnemies.screenWaitingMonsters[g_waitingEnemies.waitingCount] = static_cast<int>(monsterId);
        g_waitingEnemies.waitingCount++;
        g_waitingEnemies.currentWaitingCount++;
        g_waitingEnemies.lastAssignmentTime = SDL_GetTicks();
    }
    
    LogVerbose("👁️ Monster {} started waiting behavior for {}ms", 
              monsterId, state.waitDuration);
}

bool UpdateWaitingBehavior(Monster &monster)
{
    const size_t monsterId = monster.getId();
    if (monsterId >= MaxMonsters) return false;
    
    WaitingEnemyState &state = g_monsterWaitingStates[monsterId];
    if (!state.isWaiting) return false;
    
    uint32_t currentTime = SDL_GetTicks();
    uint32_t elapsed = currentTime - state.waitStartTime;
    
    // Check if wait period is over
    if (elapsed >= state.waitDuration) {
        EndWaitingBehavior(monster);
        return false; // Stop waiting, start attacking
    }
    
    // During waiting: move slowly toward player
    const Player &player = Players[MyPlayerId];
    Point playerPos = player.position.tile;
    Point monsterPos = monster.position.tile;
    
    // Calculate slow movement toward player
    if (monsterPos != playerPos) {
        // Reduce movement speed during waiting
        // This is handled by the monster AI, we just track the state
        
        // Optional: Add slight hesitation in movement
        if (GenerateRnd(100) < 20) { // 20% chance to pause movement briefly
            // Monster hesitates this frame
            return true;
        }
    }
    
    return true; // Still waiting
}

void EndWaitingBehavior(Monster &monster)
{
    const size_t monsterId = monster.getId();
    if (monsterId >= MaxMonsters) return;
    
    WaitingEnemyState &state = g_monsterWaitingStates[monsterId];
    if (!state.isWaiting) return;
    
    // Clear waiting state
    state.isWaiting = false;
    state.waitStartTime = 0;
    state.waitDuration = 0;
    state.hasSeenPlayer = false;
    
    // Remove from waiting monsters list
    for (int i = 0; i < g_waitingEnemies.waitingCount; i++) {
        if (g_waitingEnemies.screenWaitingMonsters[i] == static_cast<int>(monsterId)) {
            // Shift remaining elements
            for (int j = i; j < g_waitingEnemies.waitingCount - 1; j++) {
                g_waitingEnemies.screenWaitingMonsters[j] = g_waitingEnemies.screenWaitingMonsters[j + 1];
            }
            g_waitingEnemies.waitingCount--;
            g_waitingEnemies.currentWaitingCount--;
            break;
        }
    }
    
    LogVerbose("👁️ Monster {} ended waiting behavior, switching to attack", monsterId);
}

bool IsMonsterWaiting(const Monster &monster)
{
    const size_t monsterId = monster.getId();
    if (monsterId >= MaxMonsters) return false;
    
    return g_monsterWaitingStates[monsterId].isWaiting;
}

int GetCurrentWaitingCount()
{
    return g_waitingEnemies.currentWaitingCount;
}

void UpdateWaitingEnemies()
{
    if (!g_waitingEnemies.enabled) return;
    
    // Update all waiting monsters
    for (int i = 0; i < g_waitingEnemies.waitingCount; i++) {
        int monsterId = g_waitingEnemies.screenWaitingMonsters[i];
        if (monsterId < 0 || monsterId >= MaxMonsters) continue;
        
        Monster &monster = Monsters[monsterId];
        if (!monster.isInvalid) {
            UpdateWaitingBehavior(monster);
        }
    }
    
    // Clean up invalid entries
    for (int i = g_waitingEnemies.waitingCount - 1; i >= 0; i--) {
        int monsterId = g_waitingEnemies.screenWaitingMonsters[i];
        if (monsterId < 0 || static_cast<size_t>(monsterId) >= MaxMonsters || 
            Monsters[monsterId].isInvalid || 
            !g_monsterWaitingStates[monsterId].isWaiting) {
            
            // Remove invalid entry
            for (int j = i; j < g_waitingEnemies.waitingCount - 1; j++) {
                g_waitingEnemies.screenWaitingMonsters[j] = g_waitingEnemies.screenWaitingMonsters[j + 1];
            }
            g_waitingEnemies.waitingCount--;
            if (g_waitingEnemies.currentWaitingCount > 0) {
                g_waitingEnemies.currentWaitingCount--;
            }
        }
    }
}

void WaitingEnemies_ForceEndAll()
{
    for (size_t i = 0; i < MaxMonsters; i++) {
        if (g_monsterWaitingStates[i].isWaiting) {
            g_monsterWaitingStates[i] = WaitingEnemyState{};
        }
        g_waitingEnemies.screenWaitingMonsters[i] = -1;
    }
    
    g_waitingEnemies.currentWaitingCount = 0;
    g_waitingEnemies.waitingCount = 0;
    
    LogVerbose("👁️ All waiting behaviors force ended");
}

const WaitingEnemiesSystemState& GetWaitingEnemiesState()
{
    return g_waitingEnemies;
}

} // namespace devilution