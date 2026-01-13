#pragma once

/**
 * @file light_mutations.h
 * 
 * 🧬 LIGHT MUTATIONS SYSTEM (FEATURE #6)
 * 
 * OBJETIVO:
 * Crear mutaciones leves y aleatorias en monstruos para añadir variedad
 * y sorpresa sin romper el balance del juego.
 * 
 * REGLAS:
 * - Mutaciones sutiles: cambios menores en stats (±10-20%)
 * - Probabilidad baja: 8-12% de monstruos mutados
 * - Solo stats básicos: HP, damage, AC, speed
 * - No cambiar resistencias mágicas (preservar balance)
 * - Usar flags existentes para indicadores visuales
 * - Compatible con otros sistemas (depth variants, elite)
 * 
 * IMPLEMENTACIÓN:
 * - Aplicación en InitMonster después de otros sistemas
 * - Mutaciones aleatorias con seed consistente
 * - Flags para identificación visual
 * - Clamp de valores para prevenir extremos
 */

#include <cstdint>

namespace devilution {

// ============================================================================
// 🧬 LIGHT MUTATIONS CONSTANTS
// ============================================================================

namespace LightMutations {
    constexpr int START_LEVEL = 2;              // Level where mutations begin
    constexpr int MUTATION_CHANCE = 10;         // 10% base chance (1 in 10)
    constexpr int DEEP_LEVEL_BONUS = 8;         // Deeper levels = higher chance
    
    // Mutation intensity ranges (percentages)
    constexpr int MIN_HP_MUTATION = -15;        // -15% to +25% HP
    constexpr int MAX_HP_MUTATION = 25;
    constexpr int MIN_DAMAGE_MUTATION = -10;    // -10% to +20% damage
    constexpr int MAX_DAMAGE_MUTATION = 20;
    constexpr int MIN_AC_MUTATION = -3;         // -3 to +5 AC points
    constexpr int MAX_AC_MUTATION = 5;
    
    // Special mutation types
    constexpr int BERSERKER_CHANCE = 3;         // 3% chance for berserker mutation
    constexpr int TANK_CHANCE = 4;              // 4% chance for tank mutation
    constexpr int SWIFT_CHANCE = 3;             // 3% chance for swift mutation
    
    // Mutation multipliers for special types
    constexpr float BERSERKER_HP_MULT = 0.8f;   // 80% HP but more damage
    constexpr float BERSERKER_DAMAGE_MULT = 1.4f; // 140% damage
    constexpr float TANK_HP_MULT = 1.5f;        // 150% HP but less damage
    constexpr float TANK_DAMAGE_MULT = 0.7f;    // 70% damage
    constexpr float SWIFT_SPEED_BONUS = 1.2f;   // 20% faster (conceptual)
}

// ============================================================================
// 🧬 MUTATION TYPES
// ============================================================================

enum class MutationType : uint8_t {
    None = 0,
    MinorBoost,      // Small stat increases
    MinorNerf,       // Small stat decreases
    Berserker,       // Less HP, more damage
    Tank,            // More HP, less damage
    Swift,           // Faster movement (visual only)
    Resilient,       // Better AC
    Frail            // Worse AC, slightly more HP
};

// ============================================================================
// 🧬 LIGHT MUTATIONS SYSTEM STATE
// ============================================================================

struct LightMutationsState {
    bool enabled = true;
    int currentLevel = 0;
    int mutationChance = 0;
    
    // Statistics for debugging
    int totalMonstersProcessed = 0;
    int totalMutationsApplied = 0;
    int mutationsByType[8] = {0}; // Count by MutationType
};

// ============================================================================
// 🧬 LIGHT MUTATIONS API
// ============================================================================

/**
 * Initialize the light mutations system
 */
void InitLightMutations();

/**
 * Update the light mutations system (call when level changes)
 */
void UpdateLightMutations();

/**
 * Apply light mutations to a monster
 * @param monster The monster to potentially mutate
 * @return true if monster was mutated, false otherwise
 */
bool ApplyLightMutations(struct Monster &monster);

/**
 * Calculate mutation chance for current level
 * @return mutation chance percentage (0-100)
 */
int CalculateMutationChance();

/**
 * Determine mutation type based on random roll
 * @param randomValue Random value 0-99
 * @return MutationType to apply
 */
MutationType DetermineMutationType(int randomValue);

/**
 * Apply specific mutation type to monster
 * @param monster The monster to mutate
 * @param mutationType The type of mutation to apply
 */
void ApplyMutationType(struct Monster &monster, MutationType mutationType);

/**
 * Check if mutations should be applied at current level
 * @return true if mutations are active, false otherwise
 */
bool IsMutationsActive();

/**
 * Get current mutation chance percentage
 * @return current mutation chance (0-100)
 */
int GetCurrentMutationChance();

/**
 * Get current system state (for debugging)
 */
const LightMutationsState& GetLightMutationsState();

/**
 * Reset mutation statistics
 */
void ResetMutationStats();

/**
 * Get mutation type name for debugging
 * @param type The mutation type
 * @return string name of the mutation type
 */
const char* GetMutationTypeName(MutationType type);

} // namespace devilution