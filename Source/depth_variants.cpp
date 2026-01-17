#include "depth_variants.h"
#include "diablo.h"
#include "monster.h"
#include "utils/log.hpp"
#include "engine/random.hpp"
#include <algorithm>

namespace devilution {

// ============================================================================
// 🎯 GLOBAL STATE
// ============================================================================

static DepthVariantsState g_depthVariants;

// ============================================================================
// 🎯 IMPLEMENTATION
// ============================================================================

void InitDepthVariants()
{
    g_depthVariants = DepthVariantsState{};
    g_depthVariants.enabled = true;  // ✅ REACTIVADO
    g_depthVariants.currentLevel = currlevel;
    g_depthVariants.lastCalculatedLevel = 0;
    
    // Calculate initial bonuses
    CalculateDepthBonuses();
    
    LogVerbose("✅ Depth Variants System REACTIVATED for level {}", currlevel);
}

void CalculateDepthBonuses()
{
    g_depthVariants.currentLevel = currlevel;
    
    // No bonuses before start level
    if (currlevel < DepthVariants::START_LEVEL) {
        g_depthVariants.hpBonus = 0;
        g_depthVariants.damageBonus = 0;
        g_depthVariants.acBonus = 0;
        g_depthVariants.depthTier = 0;
        g_depthVariants.lastCalculatedLevel = currlevel;
        return;
    }
    
    // Calculate effective depth level
    int effectiveDepth = currlevel - DepthVariants::START_LEVEL;
    effectiveDepth = std::min(effectiveDepth, DepthVariants::MAX_DEPTH_BONUS);
    
    // Calculate depth tier (0-3)
    g_depthVariants.depthTier = effectiveDepth / DepthVariants::BONUS_EVERY_LEVELS;
    g_depthVariants.depthTier = std::min(g_depthVariants.depthTier, 3);
    
    // Calculate HP bonus (progressive scaling)
    g_depthVariants.hpBonus = effectiveDepth * DepthVariants::HP_PER_LEVEL * DepthVariants::HP_BONUS_MULTIPLIER;
    g_depthVariants.hpBonus = std::min(g_depthVariants.hpBonus, DepthVariants::MAX_HP_BONUS);
    
    // Calculate damage bonus (every 2 levels)
    g_depthVariants.damageBonus = effectiveDepth / 2;
    g_depthVariants.damageBonus = std::min(g_depthVariants.damageBonus, DepthVariants::MAX_DAMAGE_BONUS);
    
    // Calculate AC bonus (every 3 levels)
    g_depthVariants.acBonus = effectiveDepth / 3;
    g_depthVariants.acBonus = std::min(g_depthVariants.acBonus, DepthVariants::MAX_AC_BONUS);
    
    g_depthVariants.lastCalculatedLevel = currlevel;
    
    LogVerbose("🎯 Depth bonuses calculated for level {}: HP=+{}, Damage=+{}, AC=+{}, Tier={}",
              currlevel, 
              g_depthVariants.hpBonus / DepthVariants::HP_BONUS_MULTIPLIER,
              g_depthVariants.damageBonus,
              g_depthVariants.acBonus,
              g_depthVariants.depthTier);
}

void UpdateDepthVariants()
{
    // Only recalculate if level changed
    if (g_depthVariants.lastCalculatedLevel != currlevel) {
        CalculateDepthBonuses();
    }
}

void ApplyDepthScaling(Monster &monster)
{
    if (!IsDepthVariantsActive()) {
        return;
    }
    
    // Apply HP bonus
    if (g_depthVariants.hpBonus > 0) {
        monster.maxHitPoints += g_depthVariants.hpBonus;
        monster.hitPoints = monster.maxHitPoints;
        
        // Clamp to prevent overflow
        if (monster.maxHitPoints < 0) {  // Overflow check
            monster.maxHitPoints = INT32_MAX / 2;
            monster.hitPoints = monster.maxHitPoints;
        }
    }
    
    // Apply damage bonus
    if (g_depthVariants.damageBonus > 0) {
        monster.minDamage += g_depthVariants.damageBonus;
        monster.maxDamage += g_depthVariants.damageBonus;
        
        // Clamp damage values
        monster.minDamage = std::min(monster.minDamage, static_cast<uint8_t>(255));
        monster.maxDamage = std::min(monster.maxDamage, static_cast<uint8_t>(255));
    }
    
    // Apply AC bonus (better armor = higher AC)
    if (g_depthVariants.acBonus > 0) {
        monster.armorClass += g_depthVariants.acBonus;
        monster.armorClass = std::min(monster.armorClass, static_cast<uint8_t>(255));
    }
}

bool ApplyEliteTransformation(Monster &monster)
{
    if (!IsDepthVariantsActive()) {
        return false;
    }
    
    // Very rare chance for elite monsters
    if (GenerateRnd(DepthVariants::ELITE_CHANCE) != 0) {
        return false;
    }
    
    // Only apply to normal monsters (not uniques, not already special)
    if (monster.uniqueType != UniqueMonsterType::None) {
        return false;
    }
    
    // Elite transformation
    monster.maxHitPoints *= DepthVariants::ELITE_HP_MULTIPLIER;
    monster.hitPoints = monster.maxHitPoints;
    
    monster.minDamage *= DepthVariants::ELITE_DAMAGE_MULTIPLIER;
    monster.maxDamage *= DepthVariants::ELITE_DAMAGE_MULTIPLIER;
    
    // Use existing flag to mark as special
    monster.flags |= MFLAG_BERSERK;  // Visual indicator
    
    // Clamp values to prevent overflow
    if (monster.maxHitPoints < 0) {
        monster.maxHitPoints = INT32_MAX / 2;
        monster.hitPoints = monster.maxHitPoints;
    }
    monster.minDamage = std::min(monster.minDamage, static_cast<uint8_t>(255));
    monster.maxDamage = std::min(monster.maxDamage, static_cast<uint8_t>(255));
    
    LogVerbose("🎯 Elite monster created at level {} with {}x HP and {}x damage",
              currlevel, DepthVariants::ELITE_HP_MULTIPLIER, DepthVariants::ELITE_DAMAGE_MULTIPLIER);
    
    return true;
}

bool IsDepthVariantsActive()
{
    return currlevel >= DepthVariants::START_LEVEL;
}

int GetCurrentDepthTier()
{
    return g_depthVariants.depthTier;
}

int GetCurrentHPBonus()
{
    return g_depthVariants.hpBonus;
}

int GetCurrentDamageBonus()
{
    return g_depthVariants.damageBonus;
}

int GetCurrentACBonus()
{
    return g_depthVariants.acBonus;
}

const DepthVariantsState& GetDepthVariantsState()
{
    return g_depthVariants;
}

void RecalculateDepthBonuses()
{
    CalculateDepthBonuses();
}

} // namespace devilution