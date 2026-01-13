#include "light_mutations.h"
#include "diablo.h"
#include "monster.h"
#include "utils/log.hpp"
#include "engine/random.hpp"
#include <algorithm>

namespace devilution {

// ============================================================================
// 🧬 GLOBAL STATE
// ============================================================================

static LightMutationsState g_lightMutations;

// ============================================================================
// 🧬 IMPLEMENTATION
// ============================================================================

void InitLightMutations()
{
    g_lightMutations = LightMutationsState{};
    g_lightMutations.enabled = true;
    g_lightMutations.currentLevel = currlevel;
    g_lightMutations.mutationChance = CalculateMutationChance();
    
    // Reset statistics
    ResetMutationStats();
    
    LogVerbose("🧬 Light Mutations System initialized for level {} with {}% chance",
              currlevel, g_lightMutations.mutationChance);
}

int CalculateMutationChance()
{
    if (!g_lightMutations.enabled || currlevel < LightMutations::START_LEVEL) {
        return 0;
    }
    
    // Base chance increases with depth
    int baseChance = LightMutations::MUTATION_CHANCE;
    int depthBonus = (currlevel - LightMutations::START_LEVEL) / LightMutations::DEEP_LEVEL_BONUS;
    
    int totalChance = baseChance + depthBonus;
    
    // Cap at reasonable maximum
    return std::min(totalChance, 25); // Max 25% chance
}

void UpdateLightMutations()
{
    if (!g_lightMutations.enabled) return;
    
    g_lightMutations.currentLevel = currlevel;
    g_lightMutations.mutationChance = CalculateMutationChance();
    
    LogVerbose("🧬 Light Mutations updated for level {}: {}% chance",
              currlevel, g_lightMutations.mutationChance);
}

MutationType DetermineMutationType(int randomValue)
{
    // Distribution of mutation types (out of 100)
    if (randomValue < LightMutations::BERSERKER_CHANCE) {
        return MutationType::Berserker;
    } else if (randomValue < LightMutations::BERSERKER_CHANCE + LightMutations::TANK_CHANCE) {
        return MutationType::Tank;
    } else if (randomValue < LightMutations::BERSERKER_CHANCE + LightMutations::TANK_CHANCE + LightMutations::SWIFT_CHANCE) {
        return MutationType::Swift;
    } else if (randomValue < 15) {
        return MutationType::Resilient;
    } else if (randomValue < 25) {
        return MutationType::Frail;
    } else if (randomValue < 60) {
        return MutationType::MinorBoost;
    } else {
        return MutationType::MinorNerf;
    }
}

void ApplyMutationType(Monster &monster, MutationType mutationType)
{
    switch (mutationType) {
    case MutationType::Berserker:
        // Less HP, more damage - glass cannon
        monster.maxHitPoints = static_cast<int32_t>(monster.maxHitPoints * LightMutations::BERSERKER_HP_MULT);
        monster.hitPoints = monster.maxHitPoints;
        monster.minDamage = static_cast<uint8_t>(std::min(255, static_cast<int>(monster.minDamage * LightMutations::BERSERKER_DAMAGE_MULT)));
        monster.maxDamage = static_cast<uint8_t>(std::min(255, static_cast<int>(monster.maxDamage * LightMutations::BERSERKER_DAMAGE_MULT)));
        monster.flags |= MFLAG_BERSERK; // Visual indicator
        break;
        
    case MutationType::Tank:
        // More HP, less damage - defensive
        monster.maxHitPoints = static_cast<int32_t>(monster.maxHitPoints * LightMutations::TANK_HP_MULT);
        monster.hitPoints = monster.maxHitPoints;
        monster.minDamage = static_cast<uint8_t>(monster.minDamage * LightMutations::TANK_DAMAGE_MULT);
        monster.maxDamage = static_cast<uint8_t>(monster.maxDamage * LightMutations::TANK_DAMAGE_MULT);
        // No specific flag - just stats change
        break;
        
    case MutationType::Swift:
        // Conceptually faster (visual flag only)
        monster.flags |= MFLAG_SEARCH; // Reuse existing flag for visual distinction
        // Small damage boost to represent agility
        monster.minDamage = std::min(static_cast<uint8_t>(255), static_cast<uint8_t>(monster.minDamage + 2));
        monster.maxDamage = std::min(static_cast<uint8_t>(255), static_cast<uint8_t>(monster.maxDamage + 2));
        break;
        
    case MutationType::Resilient:
        // Better armor, slightly less HP
        monster.armorClass = std::min(static_cast<uint8_t>(255), static_cast<uint8_t>(monster.armorClass + GenerateRnd(LightMutations::MAX_AC_MUTATION) + 2));
        monster.maxHitPoints = static_cast<int32_t>(monster.maxHitPoints * 0.9f);
        monster.hitPoints = monster.maxHitPoints;
        break;
        
    case MutationType::Frail:
        // Worse armor, more HP to compensate
        monster.armorClass = std::max(0, static_cast<int>(monster.armorClass) - GenerateRnd(3) - 1);
        monster.maxHitPoints = static_cast<int32_t>(monster.maxHitPoints * 1.15f);
        monster.hitPoints = monster.maxHitPoints;
        break;
        
    case MutationType::MinorBoost:
        // Small random boosts
        {
            int hpBoost = GenerateRnd(LightMutations::MAX_HP_MUTATION - 5) + 5; // 5-25%
            int damageBoost = GenerateRnd(LightMutations::MAX_DAMAGE_MUTATION - 5) + 5; // 5-20%
            
            monster.maxHitPoints += (monster.maxHitPoints * hpBoost) / 100;
            monster.hitPoints = monster.maxHitPoints;
            
            int minDamageIncrease = (monster.minDamage * damageBoost) / 100;
            int maxDamageIncrease = (monster.maxDamage * damageBoost) / 100;
            monster.minDamage = std::min(static_cast<uint8_t>(255), static_cast<uint8_t>(monster.minDamage + minDamageIncrease));
            monster.maxDamage = std::min(static_cast<uint8_t>(255), static_cast<uint8_t>(monster.maxDamage + maxDamageIncrease));
        }
        break;
        
    case MutationType::MinorNerf:
        // Small random nerfs (but still viable)
        {
            int hpNerf = GenerateRnd(10) + 5; // 5-15% reduction
            int damageNerf = GenerateRnd(8) + 2; // 2-10% reduction
            
            monster.maxHitPoints -= (monster.maxHitPoints * hpNerf) / 100;
            monster.hitPoints = monster.maxHitPoints;
            
            // Ensure minimum viability
            if (monster.maxHitPoints < 64) monster.maxHitPoints = 64;
            monster.hitPoints = monster.maxHitPoints;
            
            monster.minDamage = std::max(static_cast<uint8_t>(1), static_cast<uint8_t>(monster.minDamage - (monster.minDamage * damageNerf) / 100));
            monster.maxDamage = std::max(static_cast<uint8_t>(1), static_cast<uint8_t>(monster.maxDamage - (monster.maxDamage * damageNerf) / 100));
        }
        break;
        
    default:
        return; // No mutation
    }
    
    // Update statistics
    g_lightMutations.mutationsByType[static_cast<int>(mutationType)]++;
}

bool ApplyLightMutations(Monster &monster)
{
    if (!g_lightMutations.enabled || !IsMutationsActive()) {
        return false;
    }
    
    g_lightMutations.totalMonstersProcessed++;
    
    // Don't mutate unique monsters or already special monsters
    if (monster.uniqueType != UniqueMonsterType::None) {
        return false;
    }
    
    // Don't mutate if already has special flags (preserve other systems)
    if (monster.flags & (MFLAG_BERSERK | MFLAG_SEARCH)) {
        return false;
    }
    
    // Check mutation chance
    if (GenerateRnd(100) >= g_lightMutations.mutationChance) {
        return false;
    }
    
    // Determine and apply mutation type
    int mutationRoll = GenerateRnd(100);
    MutationType mutationType = DetermineMutationType(mutationRoll);
    
    if (mutationType == MutationType::None) {
        return false;
    }
    
    ApplyMutationType(monster, mutationType);
    g_lightMutations.totalMutationsApplied++;
    
    LogVerbose("🧬 Applied {} mutation to monster at level {}",
              GetMutationTypeName(mutationType), currlevel);
    
    return true;
}

bool IsMutationsActive()
{
    return g_lightMutations.enabled && currlevel >= LightMutations::START_LEVEL;
}

int GetCurrentMutationChance()
{
    return g_lightMutations.mutationChance;
}

const LightMutationsState& GetLightMutationsState()
{
    return g_lightMutations;
}

void ResetMutationStats()
{
    g_lightMutations.totalMonstersProcessed = 0;
    g_lightMutations.totalMutationsApplied = 0;
    for (int i = 0; i < 8; i++) {
        g_lightMutations.mutationsByType[i] = 0;
    }
}

const char* GetMutationTypeName(MutationType type)
{
    switch (type) {
    case MutationType::None: return "None";
    case MutationType::MinorBoost: return "MinorBoost";
    case MutationType::MinorNerf: return "MinorNerf";
    case MutationType::Berserker: return "Berserker";
    case MutationType::Tank: return "Tank";
    case MutationType::Swift: return "Swift";
    case MutationType::Resilient: return "Resilient";
    case MutationType::Frail: return "Frail";
    default: return "Unknown";
    }
}

} // namespace devilution