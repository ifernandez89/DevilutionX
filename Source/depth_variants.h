#pragma once

/**
 * @file depth_variants.h
 * 
 * 🎯 DEPTH VARIANTS SYSTEM (FEATURE #5)
 * 
 * OBJETIVO:
 * Crear variantes de monstruos basadas en la profundidad del dungeon
 * para aumentar progresivamente la dificultad y variedad.
 * 
 * REGLAS:
 * - Escalado progresivo de HP, damage y AC por nivel
 * - Cada 4 niveles = bonus significativo
 * - Mantener balance sin romper el juego
 * - No tocar resistencias mágicas (preservar balance)
 * - Clamp valores para evitar overflow
 * 
 * IMPLEMENTACIÓN:
 * - Modificación de stats en InitMonster()
 * - Escalado basado en currlevel
 * - Preservar sistema existente de dificultad
 * - Compatible con Nightmare/Hell modes
 */

#include <cstdint>

namespace devilution {

// ============================================================================
// 🎯 DEPTH VARIANTS CONSTANTS
// ============================================================================

namespace DepthVariants {
    constexpr int START_LEVEL = 3;              // Level where variants begin
    constexpr int BONUS_EVERY_LEVELS = 4;       // Bonus every 4 levels
    constexpr int MAX_DEPTH_BONUS = 12;         // Maximum depth bonus levels
    
    // HP scaling constants
    constexpr int HP_PER_LEVEL = 5;             // HP bonus per level (in 64ths)
    constexpr int HP_BONUS_MULTIPLIER = 64;     // HP format multiplier
    constexpr int MAX_HP_BONUS = 2000 * 64;     // Maximum HP bonus
    
    // Damage scaling constants
    constexpr int DAMAGE_PER_LEVEL = 1;         // Damage bonus per 2 levels
    constexpr int MAX_DAMAGE_BONUS = 20;        // Maximum damage bonus
    
    // AC scaling constants
    constexpr int AC_PER_LEVEL = 1;             // AC bonus per 3 levels
    constexpr int MAX_AC_BONUS = 15;            // Maximum AC bonus
    
    // Elite monster chance (very rare)
    constexpr int ELITE_CHANCE = 200;           // 1 in 200 chance (0.5%)
    constexpr int ELITE_HP_MULTIPLIER = 3;      // Elite HP multiplier
    constexpr int ELITE_DAMAGE_MULTIPLIER = 2;  // Elite damage multiplier
}

// ============================================================================
// 🎯 DEPTH VARIANTS SYSTEM STATE
// ============================================================================

struct DepthVariantsState {
    bool enabled = true;
    int currentLevel = 0;
    int lastCalculatedLevel = 0;
    
    // Cached bonuses for performance
    int hpBonus = 0;
    int damageBonus = 0;
    int acBonus = 0;
    int depthTier = 0;
};

// ============================================================================
// 🎯 DEPTH VARIANTS API
// ============================================================================

/**
 * Initialize the depth variants system
 */
void InitDepthVariants();

/**
 * Update the depth variants system (call when level changes)
 */
void UpdateDepthVariants();

/**
 * Calculate depth bonuses for current level
 */
void CalculateDepthBonuses();

/**
 * Apply depth scaling to a monster
 * @param monster The monster to scale
 */
void ApplyDepthScaling(struct Monster &monster);

/**
 * Apply elite monster transformation (rare)
 * @param monster The monster to potentially make elite
 * @return true if monster became elite, false otherwise
 */
bool ApplyEliteTransformation(struct Monster &monster);

/**
 * Check if depth variants should be applied at current level
 * @return true if variants are active, false otherwise
 */
bool IsDepthVariantsActive();

/**
 * Get current depth tier (0-3, higher = deeper)
 * @return current depth tier
 */
int GetCurrentDepthTier();

/**
 * Get current HP bonus for monsters
 * @return HP bonus in internal format (64ths)
 */
int GetCurrentHPBonus();

/**
 * Get current damage bonus for monsters
 * @return damage bonus
 */
int GetCurrentDamageBonus();

/**
 * Get current AC bonus for monsters
 * @return AC bonus
 */
int GetCurrentACBonus();

/**
 * Get current system state (for debugging)
 */
const DepthVariantsState& GetDepthVariantsState();

/**
 * Force recalculation of depth bonuses
 */
void RecalculateDepthBonuses();

} // namespace devilution