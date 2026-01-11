#include "invisible_wear.h"
#include "diablo.h"
#include "utils/log.hpp"
#include "engine/random.hpp"

namespace devilution {

// ============================================================================
// 💰 GLOBAL STATE
// ============================================================================

static InvisibleWearState g_invisibleWear;

// ============================================================================
// 💰 IMPLEMENTATION
// ============================================================================

void InitInvisibleWear()
{
    g_invisibleWear = InvisibleWearState{};
    g_invisibleWear.enabled = true;
    g_invisibleWear.currentWearFactor = 1.0f;
    g_invisibleWear.lastCalculatedLevel = 0;
    
    // Initialize multipliers
    RecalculateWearMultipliers();
    
    LogVerbose("💰 Invisible Wear System initialized");
}

float CalculateWearFactor()
{
    if (!g_invisibleWear.enabled) return 1.0f;
    
    // No wear before start level
    if (currlevel < InvisibleWear::START_LEVEL) {
        return InvisibleWear::MIN_WEAR_FACTOR;
    }
    
    // Calculate progressive wear based on level
    int wearLevels = currlevel - InvisibleWear::START_LEVEL;
    int maxWearLevels = InvisibleWear::DEEP_LEVEL_THRESHOLD - InvisibleWear::START_LEVEL;
    
    // Clamp to maximum wear levels
    if (wearLevels > maxWearLevels) {
        wearLevels = maxWearLevels;
    }
    
    // Linear interpolation between min and max wear
    float wearProgress = static_cast<float>(wearLevels) / static_cast<float>(maxWearLevels);
    float wearFactor = InvisibleWear::MIN_WEAR_FACTOR - 
                      (wearProgress * (InvisibleWear::MIN_WEAR_FACTOR - InvisibleWear::MAX_WEAR_FACTOR));
    
    return wearFactor;
}

void RecalculateWearMultipliers()
{
    if (!g_invisibleWear.enabled) {
        g_invisibleWear.goldDropMultiplier = 1.0f;
        g_invisibleWear.scrollPriceMultiplier = 1.0f;
        g_invisibleWear.repairCostMultiplier = 1.0f;
        g_invisibleWear.portalCostMultiplier = 1.0f;
        return;
    }
    
    float baseFactor = CalculateWearFactor();
    g_invisibleWear.currentWearFactor = baseFactor;
    
    // Calculate specific multipliers based on wear factor
    if (currlevel >= InvisibleWear::START_LEVEL) {
        // Progressive wear - more wear at deeper levels
        float wearIntensity = 1.0f - baseFactor; // 0.0 to 0.1 (0% to 10% wear)
        
        // Gold drops: reduce by up to 15% at deepest levels
        g_invisibleWear.goldDropMultiplier = 1.0f - (wearIntensity * 0.15f);
        
        // Scroll prices: increase by up to 15% at deepest levels
        g_invisibleWear.scrollPriceMultiplier = 1.0f + (wearIntensity * 0.15f);
        
        // Repair costs: increase by up to 12% at deepest levels
        g_invisibleWear.repairCostMultiplier = 1.0f + (wearIntensity * 0.12f);
        
        // Portal costs: increase by up to 10% at deepest levels
        g_invisibleWear.portalCostMultiplier = 1.0f + (wearIntensity * 0.10f);
    } else {
        // No wear at early levels
        g_invisibleWear.goldDropMultiplier = 1.0f;
        g_invisibleWear.scrollPriceMultiplier = 1.0f;
        g_invisibleWear.repairCostMultiplier = 1.0f;
        g_invisibleWear.portalCostMultiplier = 1.0f;
    }
    
    g_invisibleWear.lastCalculatedLevel = currlevel;
    
    LogVerbose("💰 Wear multipliers updated for level {}: Gold={:.3f}, Scrolls={:.3f}, Repair={:.3f}, Portal={:.3f}",
              currlevel, 
              g_invisibleWear.goldDropMultiplier,
              g_invisibleWear.scrollPriceMultiplier,
              g_invisibleWear.repairCostMultiplier,
              g_invisibleWear.portalCostMultiplier);
}

void UpdateInvisibleWear()
{
    if (!g_invisibleWear.enabled) return;
    
    // Only recalculate if level changed
    if (g_invisibleWear.lastCalculatedLevel != currlevel) {
        RecalculateWearMultipliers();
    }
}

int ApplyGoldDropWear(int originalAmount)
{
    if (!g_invisibleWear.enabled || !IsWearActive()) {
        return originalAmount;
    }
    
    float modifiedAmount = static_cast<float>(originalAmount) * g_invisibleWear.goldDropMultiplier;
    int result = static_cast<int>(modifiedAmount);
    
    // Ensure we never reduce to 0 (minimum 1 gold)
    if (result < 1 && originalAmount > 0) {
        result = 1;
    }
    
    return result;
}

int ApplyScrollPriceWear(int originalPrice)
{
    if (!g_invisibleWear.enabled || !IsWearActive()) {
        return originalPrice;
    }
    
    float modifiedPrice = static_cast<float>(originalPrice) * g_invisibleWear.scrollPriceMultiplier;
    return static_cast<int>(modifiedPrice);
}

int ApplyRepairCostWear(int originalCost)
{
    if (!g_invisibleWear.enabled || !IsWearActive()) {
        return originalCost;
    }
    
    float modifiedCost = static_cast<float>(originalCost) * g_invisibleWear.repairCostMultiplier;
    return static_cast<int>(modifiedCost);
}

int ApplyPortalCostWear(int originalCost)
{
    if (!g_invisibleWear.enabled || !IsWearActive()) {
        return originalCost;
    }
    
    float modifiedCost = static_cast<float>(originalCost) * g_invisibleWear.portalCostMultiplier;
    return static_cast<int>(modifiedCost);
}

bool IsWearActive()
{
    return g_invisibleWear.enabled && currlevel >= InvisibleWear::START_LEVEL;
}

float GetCurrentWearFactor()
{
    return g_invisibleWear.currentWearFactor;
}

const InvisibleWearState& GetInvisibleWearState()
{
    return g_invisibleWear;
}

} // namespace devilution