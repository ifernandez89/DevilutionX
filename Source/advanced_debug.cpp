#include "advanced_debug.h"
#include "monster.h"
#include "player.h"
#include "diablo.h"
#include "multi.h"
#include "engine/ticks.hpp"
#include "utils/log.hpp"
#include <algorithm>
#include <cmath>

namespace devilution {

// ============================================================================
// 🎮 GLOBAL DEBUG STATE
// ============================================================================

static AdvancedDebugState g_debugState;

// ============================================================================
// 🎮 IMPLEMENTATION
// ============================================================================

void InitAdvancedDebug()
{
    g_debugState = AdvancedDebugState{};
    g_debugState.enabled = false;
    g_debugState.activeCategories = AdvancedDebug::CATEGORY_MONSTERS; // Solo monstruos por defecto
    
    LogVerbose("🎮 Advanced Debug System initialized - Monsters only");
}

void ToggleAdvancedDebug()
{
    g_debugState.enabled = !g_debugState.enabled;
    
    if (g_debugState.enabled) {
        NetSendCmdString(1 << MyPlayerId, "🎯 TARGET DEBUG: ON (F12 to toggle)");
        // Reset timer when enabling
        g_debugState.monsterUpdateTimer = 0;
    } else {
        NetSendCmdString(1 << MyPlayerId, "🎯 TARGET DEBUG: OFF");
    }
}

void ToggleDebugCategory(int category)
{
    if (!g_debugState.enabled) return;
    
    g_debugState.activeCategories ^= category;
    
    const char* categoryName = "Unknown";
    switch (category) {
    case AdvancedDebug::CATEGORY_MONSTERS: categoryName = "MONSTERS"; break;
    case AdvancedDebug::CATEGORY_PLAYER: categoryName = "PLAYER"; break;
    case AdvancedDebug::CATEGORY_COMBAT: categoryName = "COMBAT"; break;
    case AdvancedDebug::CATEGORY_ATMOSPHERE: categoryName = "ATMOSPHERE"; break;
    case AdvancedDebug::CATEGORY_LEVEL: categoryName = "LEVEL"; break;
    }
    
    bool isEnabled = (g_debugState.activeCategories & category) != 0;
    char msg[100];
    snprintf(msg, sizeof(msg), "🎮 DEBUG %s: %s", categoryName, isEnabled ? "ON" : "OFF");
    NetSendCmdString(1 << MyPlayerId, msg);
}

bool IsAdvancedDebugEnabled()
{
    return g_debugState.enabled;
}

std::string GetMonsterAIStateName(const Monster& monster)
{
    switch (monster.mode) {
    case MonsterMode::Stand: return "Stand";
    case MonsterMode::MoveNorthwards: return "Walk";
    case MonsterMode::MeleeAttack: return "Melee";
    case MonsterMode::HitRecovery: return "Hit";
    case MonsterMode::Death: return "Death";
    case MonsterMode::SpecialMeleeAttack: return "Special";
    case MonsterMode::FadeIn: return "FadeIn";
    case MonsterMode::FadeOut: return "FadeOut";
    case MonsterMode::RangedAttack: return "Ranged";
    case MonsterMode::SpecialRangedAttack: return "SpRanged";
    case MonsterMode::Delay: return "Delay";
    case MonsterMode::Charge: return "Charge";
    case MonsterMode::Petrified: return "Stone";
    default: return "Unknown";
    }
}

std::string GetMonsterMutationInfo(const Monster& monster)
{
    // Check for light mutations (Feature #6)
    // This is a simplified check - in real implementation we'd check the actual mutation system
    if (monster.hitPoints > monster.maxHitPoints * 1.2f) {
        return "Tank";
    } else if (monster.armorClass > 50) {
        return "Armored";
    } else if (monster.minDamage > monster.maxDamage * 0.8f) {
        return "Berserker";
    }
    return "";
}

int GetMonsterPlayerDistance(const Monster& monster)
{
    if (!MyPlayer) return 999;
    
    const int dx = std::abs(monster.position.tile.x - MyPlayer->position.tile.x);
    const int dy = std::abs(monster.position.tile.y - MyPlayer->position.tile.y);
    return std::max(dx, dy); // Chebyshev distance (like Diablo uses)
}

void ShowMonsterDebugInfo()
{
    if (!(g_debugState.activeCategories & AdvancedDebug::CATEGORY_MONSTERS)) return;
    
    Monster* closestMonster = nullptr;
    int closestDistance = 999;
    
    // Encontrar el monstruo más cercano y activo
    for (size_t i = 0; i < MaxMonsters; i++) {
        Monster& monster = Monsters[i];
        
        // Skip inactive monsters
        if (monster.mode != MonsterMode::Death) {
            const int distance = GetMonsterPlayerDistance(monster);
            
            // Solo considerar monstruos dentro de rango razonable
            if (distance <= AdvancedDebug::MAX_DISTANCE_SHOWN && distance < closestDistance) {
                closestMonster = &monster;
                closestDistance = distance;
            }
        }
    }
    
    // Mostrar solo el monstruo más cercano en una línea
    if (closestMonster != nullptr) {
        const std::string aiState = GetMonsterAIStateName(*closestMonster);
        const std::string mutation = GetMonsterMutationInfo(*closestMonster);
        const int hpPercent = (closestMonster->hitPoints * 100) / std::max(1, closestMonster->maxHitPoints);
        
        char monsterInfo[200];
        if (!mutation.empty()) {
            snprintf(monsterInfo, sizeof(monsterInfo),
                "[TARGET] %s: HP=%d(%d%%) Dist=%d AI=%s Mut=%s",
                std::string(closestMonster->name()).c_str(), closestMonster->hitPoints,
                hpPercent, closestDistance, aiState.c_str(), mutation.c_str());
        } else {
            snprintf(monsterInfo, sizeof(monsterInfo),
                "[TARGET] %s: HP=%d(%d%%) Dist=%d AI=%s",
                std::string(closestMonster->name()).c_str(), closestMonster->hitPoints,
                hpPercent, closestDistance, aiState.c_str());
        }
        
        NetSendCmdString(1 << MyPlayerId, monsterInfo);
    } else {
        NetSendCmdString(1 << MyPlayerId, "[TARGET] No monsters nearby");
    }
}

void ShowPlayerDebugInfo()
{
    if (!(g_debugState.activeCategories & AdvancedDebug::CATEGORY_PLAYER)) return;
    if (!MyPlayer) return;
    
    const Player& player = *MyPlayer;
    
    char playerInfo[200];
    snprintf(playerInfo, sizeof(playerInfo),
        "[PLR] HP=%d/%d Mana=%d/%d AC=%d Lvl=%d Exp=%d",
        player._pHitPoints >> 6, player._pMaxHP >> 6,
        player._pMana >> 6, player._pMaxMana >> 6,
        player._pIAC + player._pIAC, player.getCharacterLevel(), player._pExperience);
    
    NetSendCmdString(1 << MyPlayerId, playerInfo);
}

void ShowCombatDebugInfo()
{
    if (!(g_debugState.activeCategories & AdvancedDebug::CATEGORY_COMBAT)) return;
    
    // Calculate DPS (simplified)
    const int currentTime = SDL_GetTicks();
    const int combatDuration = currentTime - g_debugState.combatStartTime;
    const int dps = (combatDuration > 0) ? (g_debugState.totalDamageDealt * 60) / combatDuration : 0;
    
    char combatInfo[200];
    snprintf(combatInfo, sizeof(combatInfo),
        "[CMB] DPS=%d Kills=%d LastKill=%s Duration=%ds",
        dps, g_debugState.totalKills, g_debugState.lastKilledMonster.c_str(),
        combatDuration / 60);
    
    NetSendCmdString(1 << MyPlayerId, combatInfo);
}

void ShowAtmosphereDebugInfo()
{
    if (!(g_debugState.activeCategories & AdvancedDebug::CATEGORY_ATMOSPHERE)) return;
    
    // This would check the actual state of our atmospheric features
    // For now, we'll show placeholder info
    char atmosphereInfo[200];
    snprintf(atmosphereInfo, sizeof(atmosphereInfo),
        "[ATM] Silence=%s Pauses=%s Mutations=%d WaitEnemies=%d",
        "Active", "Ready", 2, 1); // Placeholder values
    
    NetSendCmdString(1 << MyPlayerId, atmosphereInfo);
}

void ShowLevelDebugInfo()
{
    if (!(g_debugState.activeCategories & AdvancedDebug::CATEGORY_LEVEL)) return;
    
    // Count active monsters
    int activeMonsters = 0;
    for (size_t i = 0; i < MaxMonsters; i++) {
        if (Monsters[i].mode != MonsterMode::Death) {
            activeMonsters++;
        }
    }
    
    char levelInfo[200];
    snprintf(levelInfo, sizeof(levelInfo),
        "[LVL] Depth=%d Monsters=%d/%d Items=%d",
        currlevel, activeMonsters, static_cast<int>(MaxMonsters), ActiveItemCount);
    
    NetSendCmdString(1 << MyPlayerId, levelInfo);
}

void RecordDamageDealt(int damage)
{
    if (!g_debugState.enabled) return;
    
    g_debugState.totalDamageDealt += damage;
    
    // Start combat timer if not already started
    if (g_debugState.combatStartTime == 0) {
        g_debugState.combatStartTime = SDL_GetTicks();
    }
}

void RecordMonsterKill(const std::string& monsterName)
{
    if (!g_debugState.enabled) return;
    
    g_debugState.totalKills++;
    g_debugState.lastKilledMonster = monsterName;
    g_debugState.lastKillTime = SDL_GetTicks();
}

void UpdateAdvancedDebug()
{
    if (!g_debugState.enabled) return;
    
    // Solo actualizar información de monstruos
    if (--g_debugState.monsterUpdateTimer <= 0) {
        g_debugState.monsterUpdateTimer = AdvancedDebug::MONSTER_UPDATE_FREQ;
        ShowMonsterDebugInfo();
    }
}

} // namespace devilution