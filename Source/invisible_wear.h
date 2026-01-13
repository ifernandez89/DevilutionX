#pragma once

/**
 * @file invisible_wear.h
 * 
 * 🎯 INVISIBLE WEAR SYSTEM (FEATURE #4)
 * 
 * OBJETIVO:
 * Crear presión de recursos mediante desgaste invisible que reduce
 * sutilmente la "generosidad" del juego a mayor profundidad.
 * 
 * REGLAS:
 * - Menos gold drops en niveles profundos
 * - Town Portal + scrolls ligeramente más caros
 * - Reparar cuesta un poco más
 * - Todo <10% por depth grande para mantener balance
 * - Multiplicador por currlevel, no UI, no mensajes
 * 
 * IMPLEMENTACIÓN:
 * - Multiplicadores invisibles basados en nivel
 * - Afecta drops, precios, costos de reparación
 * - Progresivo y sutil - el jugador lo siente pero no lo ve
 * - No rompe la economía, solo aumenta presión gradual
 */

#include <cstdint>

namespace devilution {

// ============================================================================
// 💰 INVISIBLE WEAR CONSTANTS
// ============================================================================

namespace InvisibleWear {
    constexpr int START_LEVEL = 5;              // Level where wear begins
    constexpr float MAX_WEAR_FACTOR = 0.90f;    // Maximum 10% reduction at deepest levels
    constexpr float MIN_WEAR_FACTOR = 1.00f;    // No wear at early levels
    constexpr int DEEP_LEVEL_THRESHOLD = 16;    // Level 16 = maximum wear
    
    // Specific wear categories
    constexpr float GOLD_DROP_WEAR = 0.85f;     // Gold drops reduced to 85% in deep levels
    constexpr float SCROLL_PRICE_WEAR = 1.15f;  // Scrolls cost 15% more in deep levels
    constexpr float REPAIR_COST_WEAR = 1.12f;   // Repair costs 12% more in deep levels
    constexpr float PORTAL_COST_WEAR = 1.10f;   // Town Portal costs 10% more in deep levels
}

// ============================================================================
// 💰 INVISIBLE WEAR SYSTEM STATE
// ============================================================================

struct InvisibleWearState {
    bool enabled = true;
    float currentWearFactor = 1.0f;
    int lastCalculatedLevel = 0;
    
    // Cached multipliers for performance
    float goldDropMultiplier = 1.0f;
    float scrollPriceMultiplier = 1.0f;
    float repairCostMultiplier = 1.0f;
    float portalCostMultiplier = 1.0f;
};

// ============================================================================
// 💰 INVISIBLE WEAR API
// ============================================================================

/**
 * Initialize the invisible wear system
 */
void InitInvisibleWear();

/**
 * Update the invisible wear system (call when level changes)
 */
void UpdateInvisibleWear();

/**
 * Calculate wear factor for current level
 * @return wear factor (1.0 = no wear, <1.0 = reduced generosity)
 */
float CalculateWearFactor();

/**
 * Apply wear to gold drop amount
 * @param originalAmount The original gold amount
 * @return modified gold amount with wear applied
 */
int ApplyGoldDropWear(int originalAmount);

/**
 * Apply wear to scroll prices
 * @param originalPrice The original scroll price
 * @return modified price with wear applied
 */
int ApplyScrollPriceWear(int originalPrice);

/**
 * Apply wear to repair costs
 * @param originalCost The original repair cost
 * @return modified cost with wear applied
 */
int ApplyRepairCostWear(int originalCost);

/**
 * Apply wear to Town Portal costs
 * @param originalCost The original portal cost
 * @return modified cost with wear applied
 */
int ApplyPortalCostWear(int originalCost);

/**
 * Check if wear should be applied at current level
 * @return true if wear is active, false otherwise
 */
bool IsWearActive();

/**
 * Get current wear factor for debugging
 * @return current wear factor
 */
float GetCurrentWearFactor();

/**
 * Force recalculation of wear multipliers
 */
void RecalculateWearMultipliers();

/**
 * Get current system state (for debugging)
 */
const InvisibleWearState& GetInvisibleWearState();

} // namespace devilution