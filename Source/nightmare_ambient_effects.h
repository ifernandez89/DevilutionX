/**
 * @file nightmare_ambient_effects.h
 * @brief Nightmare Edition - Ambient Effects System
 * 
 * EFECTOS AMBIENTALES NIGHTMARE - SIMPLE Y EFECTIVO
 * =================================================
 * 
 * Efectos que puedes ver inmediatamente:
 * - Parpadeo de antorchas mejorado
 * - Sonidos atmosféricos
 * - Efectos de niebla sutil
 * - Iluminación dinámica
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estado de los efectos ambientales nightmare
 */
struct NightmareAmbientState {
	bool initialized = false;
	bool enabled = true;
	
	// Efectos activos
	bool torchFlicker = true;
	bool atmosphericSounds = true;
	bool subtleFog = true;
	bool dynamicLighting = true;
	
	// Configuración
	float effectIntensity = 1.0f;
	uint32_t lastUpdateTime = 0;
};

// Estado global
extern NightmareAmbientState g_nightmareAmbient;

/**
 * @brief Inicializa efectos ambientales nightmare
 */
void InitNightmareAmbientEffects();

/**
 * @brief Actualiza efectos ambientales nightmare
 */
void UpdateNightmareAmbientEffects();

/**
 * @brief Aplica efectos de parpadeo de antorchas
 */
void ApplyTorchFlickerEffects();

/**
 * @brief Reproduce sonidos atmosféricos
 */
void PlayAtmosphericSounds();

/**
 * @brief Aplica efectos de niebla sutil
 */
void ApplySubtleFogEffects();

/**
 * @brief Fuerza activación de todos los efectos
 */
void ForceEnableAllAmbientEffects();

/**
 * @brief Limpia efectos ambientales
 */
void CleanupNightmareAmbientEffects();

} // namespace devilution