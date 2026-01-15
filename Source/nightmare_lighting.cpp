/**
 * @file nightmare_lighting.cpp
 * @brief Nightmare Edition - Atmospheric Lighting Implementation
 * 
 * IMPLEMENTACIÓN DE ILUMINACIÓN ATMOSFÉRICA
 * =========================================
 * 
 * Sistema que agrega efectos de parpadeo orgánico a las fuentes de luz.
 */

#include "nightmare_lighting.h"

#include <cstdlib>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"
#include "lighting.h"
#include "interfac.h"  // 🛡️ For g_isLevelTransition

namespace devilution {

// Estado global del sistema
NightmareLightingState g_nightmareLighting;

// Configuración por defecto
namespace {
	constexpr int MAX_FLICKER_RADIUS = 2;      // Máxima variación de radio (±2)
	constexpr uint32_t UPDATE_INTERVAL = 50;   // Actualizar cada 50ms
}

void InitNightmareLighting()
{
	LogVerbose("🔥 Initializing Nightmare Atmospheric Lighting System");
	
	g_nightmareLighting.initialized = true;
	g_nightmareLighting.config.enabled = true;
	g_nightmareLighting.activeLightCount = 0;
	g_nightmareLighting.lastUpdateTime = SDL_GetTicks();
	
	// Inicializar array de luces
	for (auto &light : g_nightmareLighting.lights) {
		light.lightId = -1;
		light.type = AtmosphericLightType::NONE;
		light.enabled = false;
	}
	
	LogVerbose("🔥 Nightmare Atmospheric Lighting initialized");
	LogVerbose("🕯️ Torch flicker: enabled");
	LogVerbose("🕯️ Candle flicker: enabled");
	LogVerbose("🔥 Fire flicker: enabled");
	LogVerbose("✨ Magic pulse: enabled");
}

void UpdateNightmareLighting()
{
	if (!g_nightmareLighting.initialized) {
		return;
	}
	
	// 🛡️ TRANSITION SAFETY: Skip ALL processing during level transitions
	if (g_isLevelTransition) {
		return;
	}
	
	// 🛡️ SAFETY CHECK - Don't update during level transitions
	// ActiveLightCount == 0 indicates lighting system is being reinitialized
	if (ActiveLightCount == 0) {
		return;
	}
	
	// FORCE ALWAYS ENABLED for visibility
	g_nightmareLighting.config.enabled = true;
	
	uint32_t currentTime = SDL_GetTicks();
	
	// Solo actualizar cada UPDATE_INTERVAL ms para performance
	if (currentTime - g_nightmareLighting.lastUpdateTime < UPDATE_INTERVAL) {
		return;
	}
	
	g_nightmareLighting.lastUpdateTime = currentTime;
	
	// Actualizar cada luz atmosférica activa
	for (int i = 0; i < g_nightmareLighting.activeLightCount; i++) {
		AtmosphericLight &light = g_nightmareLighting.lights[i];
		
		if (!light.enabled || light.lightId == -1) {
			continue;
		}
		
		// 🛡️ SAFETY CHECK - Verify light ID is still valid
		if (light.lightId < 0 || light.lightId >= MAXLIGHTS) {
			light.enabled = false;
			light.lightId = -1;
			continue;
		}
		
		// Verificar si es tiempo de cambiar el parpadeo
		if (currentTime >= light.lastFlickerTime + light.flickerDelay) {
			// Calcular nueva variación según el tipo de luz
			int8_t newFlicker = 0;
			uint32_t newDelay = 0;
			
			switch (light.type) {
			case AtmosphericLightType::TORCH:
				// Antorchas: parpadeo medio, orgánico
				newFlicker = (rand() % 5) - 2;  // -2 a +2
				newDelay = g_nightmareLighting.config.minFlickerDelay + 
				          (rand() % (g_nightmareLighting.config.maxFlickerDelay - g_nightmareLighting.config.minFlickerDelay));
				break;
				
			case AtmosphericLightType::CANDLE:
				// Velas: parpadeo más rápido y sutil
				newFlicker = (rand() % 3) - 1;  // -1 a +1
				newDelay = g_nightmareLighting.config.minFlickerDelay / 2 + 
				          (rand() % (g_nightmareLighting.config.minFlickerDelay));
				break;
				
			case AtmosphericLightType::FIRE:
				// Fuego: parpadeo intenso
				newFlicker = (rand() % 5) - 2;  // -2 a +2
				newDelay = g_nightmareLighting.config.minFlickerDelay / 3 + 
				          (rand() % (g_nightmareLighting.config.minFlickerDelay / 2));
				break;
				
			case AtmosphericLightType::MAGIC:
				// Magia: pulso suave y lento
				static int magicPhase = 0;
				magicPhase = (magicPhase + 1) % 20;
				newFlicker = (magicPhase < 10) ? 1 : -1;  // Pulso suave
				newDelay = g_nightmareLighting.config.maxFlickerDelay;
				break;
				
			default:
				continue;
			}
			
			// Aplicar intensidad global
			newFlicker = static_cast<int8_t>(newFlicker * g_nightmareLighting.config.flickerIntensity);
			
			// Limitar variación
			newFlicker = std::clamp(newFlicker, static_cast<int8_t>(-MAX_FLICKER_RADIUS), static_cast<int8_t>(MAX_FLICKER_RADIUS));
			
			// Actualizar luz si cambió
			if (newFlicker != light.currentFlicker) {
				light.currentFlicker = newFlicker;
				
				// Calcular nuevo radio
				uint8_t newRadius = light.baseRadius + newFlicker;
				newRadius = std::clamp(newRadius, static_cast<uint8_t>(1), static_cast<uint8_t>(15));
				
				// Aplicar cambio al sistema de iluminación
				ChangeLightRadius(light.lightId, newRadius);
			}
			
			light.lastFlickerTime = currentTime;
			light.flickerDelay = newDelay;
		}
	}
}

int RegisterAtmosphericLight(int lightId, AtmosphericLightType type, uint8_t baseRadius)
{
	if (!g_nightmareLighting.initialized || lightId < 0) {
		return -1;
	}
	
	// Buscar slot libre
	for (int i = 0; i < static_cast<int>(g_nightmareLighting.lights.size()); i++) {
		if (g_nightmareLighting.lights[i].lightId == -1) {
			AtmosphericLight &light = g_nightmareLighting.lights[i];
			
			light.lightId = lightId;
			light.type = type;
			light.baseRadius = baseRadius;
			light.currentFlicker = 0;
			light.lastFlickerTime = SDL_GetTicks();
			light.flickerDelay = g_nightmareLighting.config.minFlickerDelay + 
			                    (rand() % (g_nightmareLighting.config.maxFlickerDelay - g_nightmareLighting.config.minFlickerDelay));
			light.enabled = true;
			
			// Actualizar contador si es necesario
			if (i >= g_nightmareLighting.activeLightCount) {
				g_nightmareLighting.activeLightCount = i + 1;
			}
			
			LogVerbose("🔥 Registered atmospheric light: ID={}, type={}, radius={}", 
				lightId, static_cast<int>(type), baseRadius);
			
			return i;
		}
	}
	
	LogVerbose("⚠️ Failed to register atmospheric light: no free slots");
	return -1;
}

void UnregisterAtmosphericLight(int atmosphericIndex)
{
	if (atmosphericIndex < 0 || atmosphericIndex >= static_cast<int>(g_nightmareLighting.lights.size())) {
		return;
	}
	
	AtmosphericLight &light = g_nightmareLighting.lights[atmosphericIndex];
	
	LogVerbose("🔥 Unregistering atmospheric light: ID={}", light.lightId);
	
	light.lightId = -1;
	light.type = AtmosphericLightType::NONE;
	light.enabled = false;
	
	// Actualizar contador si es necesario
	if (atmosphericIndex == g_nightmareLighting.activeLightCount - 1) {
		// Buscar el último activo
		int newCount = 0;
		for (int i = 0; i < static_cast<int>(g_nightmareLighting.lights.size()); i++) {
			if (g_nightmareLighting.lights[i].lightId != -1) {
				newCount = i + 1;
			}
		}
		g_nightmareLighting.activeLightCount = newCount;
	}
}

void SetNightmareLightingEnabled(bool enabled)
{
	g_nightmareLighting.config.enabled = enabled;
	LogVerbose("🔥 Nightmare Atmospheric Lighting: {}", enabled ? "enabled" : "disabled");
}

void SetTorchFlickerEnabled(bool enabled)
{
	g_nightmareLighting.config.torchFlicker = enabled;
	LogVerbose("🕯️ Torch flicker: {}", enabled ? "enabled" : "disabled");
}

void SetFlickerIntensity(float intensity)
{
	g_nightmareLighting.config.flickerIntensity = std::clamp(intensity, 0.0f, 2.0f);
	LogVerbose("🔥 Flicker intensity set to: {:.2f}", g_nightmareLighting.config.flickerIntensity);
}

void CleanupNightmareLighting()
{
	LogVerbose("🔥 Cleaning up Nightmare Atmospheric Lighting");
	
	// 🛡️ PORTAL CRASH FIX: Complete reset of all nightmare lighting state
	g_nightmareLighting.initialized = false;
	g_nightmareLighting.activeLightCount = 0;
	g_nightmareLighting.lastUpdateTime = 0;
	
	// Reset ALL lights, not just active ones
	for (auto &light : g_nightmareLighting.lights) {
		light.lightId = -1;
		light.type = AtmosphericLightType::NONE;
		light.enabled = false;
		light.baseRadius = 0;
		light.currentFlicker = 0;
		light.lastFlickerTime = 0;
		light.flickerDelay = 0;
	}
	
	LogVerbose("🔥 Nightmare Lighting cleanup complete - activeLightCount reset to 0");
}

} // namespace devilution