/**
 * @file organic_lighting.cpp
 * @brief Nightmare Edition - Organic Lighting Implementation
 * 
 * IMPLEMENTACIÓN DE LUCES ORGÁNICAS
 * =================================
 * 
 * Micro-parpadeo irregular para hogueras y antorchas que simula
 * la respiración natural del fuego sin afectar la jugabilidad.
 */

#include "organic_lighting.h"

#include <cstdlib>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema de luces orgánicas
OrganicLightingState organicLighting;

// Configuración del sistema
namespace {
	constexpr uint32_t MIN_FLICKER_DELAY_MS = 800;   // Mínimo 800ms entre parpadeos
	constexpr uint32_t MAX_FLICKER_DELAY_MS = 2000;  // Máximo 2 segundos
	constexpr int FLICKER_PROBABILITY = 30;          // 30% probabilidad de parpadeo
}

void InitOrganicLighting()
{
	LogVerbose("Initializing Nightmare Organic Lighting System");
	
	organicLighting.enabled = true;
	organicLighting.lastFlickerTick = SDL_GetTicks();
	organicLighting.flickerValue = 0;
	
	// Calcular primer delay aleatorio
	organicLighting.nextFlickerDelay = MIN_FLICKER_DELAY_MS + 
		(rand() % (MAX_FLICKER_DELAY_MS - MIN_FLICKER_DELAY_MS));
	
	LogVerbose("Organic Lighting System initialized - Fire breathes");
}

void UpdateOrganicLighting(uint32_t currentTick)
{
	if (!organicLighting.enabled) {
		return;
	}
	
	// Verificar si es momento de actualizar el parpadeo
	if (currentTick - organicLighting.lastFlickerTick >= organicLighting.nextFlickerDelay) {
		
		// Decidir si parpadear o no (irregular)
		if (rand() % 100 < FLICKER_PROBABILITY) {
			// Generar nuevo valor de parpadeo (-1, 0, +1)
			organicLighting.flickerValue = (rand() % 3) - 1;
		} else {
			// No parpadear esta vez
			organicLighting.flickerValue = 0;
		}
		
		// Calcular próximo delay (irregular)
		organicLighting.nextFlickerDelay = MIN_FLICKER_DELAY_MS + 
			(rand() % (MAX_FLICKER_DELAY_MS - MIN_FLICKER_DELAY_MS));
		
		organicLighting.lastFlickerTick = currentTick;
	}
}

int GetLightFlickerModifier()
{
	if (!organicLighting.enabled) {
		return 0;
	}
	
	return organicLighting.flickerValue;
}

bool ShouldLightFlicker(int lightType)
{
	// TODO: Definir qué tipos de luz deben parpadear
	// Por ahora, asumir que todas las luces del town pueden parpadear
	// En el futuro, filtrar por tipo (hogueras, antorchas, etc.)
	return organicLighting.enabled;
}

void SetOrganicLightingEnabled(bool enabled)
{
	LogVerbose("Organic Lighting: {}", enabled ? "enabled" : "disabled");
	organicLighting.enabled = enabled;
	
	if (!enabled) {
		// Reset al estado inicial
		organicLighting.flickerValue = 0;
	}
}

bool IsOrganicLightingEnabled()
{
	return organicLighting.enabled;
}

} // namespace devilution