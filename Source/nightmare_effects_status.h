/**
 * @file nightmare_effects_status.h
 * @brief Nightmare Edition - Effects Status Display
 * 
 * SISTEMA DE VERIFICACIÓN DE EFECTOS NIGHTMARE
 * ===========================================
 * 
 * Muestra el estado de todos los efectos para verificar que funcionan.
 */

#pragma once

namespace devilution {

/**
 * @brief Muestra el estado de todos los efectos nightmare
 */
void ShowNightmareEffectsStatus();

/**
 * @brief Verifica que todos los efectos estén activos
 */
void VerifyAllNightmareEffectsActive();

/**
 * @brief Fuerza la activación de TODOS los efectos nightmare
 */
void ForceActivateAllNightmareEffects();

} // namespace devilution