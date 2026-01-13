/**
 * @file nightmare_immediate_effects.h
 * @brief Nightmare Edition - Immediate Visible Effects
 * 
 * EFECTOS INMEDIATAMENTE VISIBLES
 * ==============================
 * 
 * Sistema que crea efectos visibles inmediatos para testing
 * y demostración de la funcionalidad Nightmare.
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Aplica un efecto de oscurecimiento inmediato
 * @param intensity Intensidad del oscurecimiento (0.0 - 1.0)
 */
void ApplyImmediateDarkening(float intensity);

/**
 * @brief Aplica un efecto de desaturación inmediato
 * @param intensity Intensidad de desaturación (0.0 - 1.0)
 */
void ApplyImmediateDesaturation(float intensity);

/**
 * @brief Aplica un efecto de tinte rojizo (atmósfera infernal)
 * @param intensity Intensidad del tinte (0.0 - 1.0)
 */
void ApplyImmediateRedTint(float intensity);

/**
 * @brief Resetea todos los efectos inmediatos
 */
void ResetImmediateEffects();

/**
 * @brief Toggle del efecto de niebla visual
 */
void ToggleVisualFog();

/**
 * @brief Toggle del efecto de oscurecimiento
 */
void ToggleVisualDarkening();

/**
 * @brief Muestra información de efectos activos
 */
void ShowActiveEffectsInfo();

} // namespace devilution