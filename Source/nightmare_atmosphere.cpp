/**
 * @file nightmare_atmosphere.cpp
 * @brief Nightmare Edition - Atmospheric Effects Implementation
 * 
 * IMPLEMENTACIÓN DE ATMÓSFERA NIGHTMARE
 * ====================================
 * 
 * Efectos sutiles que modifican la percepción sin ser obvios:
 * - Desaturación leve para sensación más oscura
 * - Viento visual en sombras
 * - Respiración sutil en NPCs
 */

#include "nightmare_atmosphere.h"

#include <cmath>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema atmosférico
NightmareAtmosphereState nightmareAtmosphere;

// Configuración del sistema
namespace {
	constexpr float DEFAULT_DESATURATION = 0.06f;  // 6% desaturación
	constexpr float WIND_FREQUENCY = 0.001f;       // Muy lento
	constexpr float BREATHING_FREQUENCY = 0.002f;  // Respiración lenta
}

void InitNightmareAtmosphere()
{
	LogVerbose("Initializing Nightmare Atmosphere System");
	
	nightmareAtmosphere.enabled = true;
	nightmareAtmosphere.desaturationLevel = 0.15f;  // 🌙 OSCURECIMIENTO PERMANENTE: 15% desaturación (era 6%)
	nightmareAtmosphere.windTick = SDL_GetTicks();
	nightmareAtmosphere.shadowOffset = 0;
	nightmareAtmosphere.breathingActive = true;
	
	LogVerbose("Nightmare Atmosphere initialized - PERMANENT darkening active (15% desaturation)");
}

void UpdateNightmareAtmosphere(uint32_t currentTick)
{
	if (!nightmareAtmosphere.enabled) {
		return;
	}
	
	nightmareAtmosphere.windTick = currentTick;
	
	// Calcular offset de sombra por viento (muy sutil)
	float windPhase = currentTick * WIND_FREQUENCY;
	float windValue = std::sin(windPhase);
	
	if (windValue > 0.7f) {
		nightmareAtmosphere.shadowOffset = 1;
	} else if (windValue < -0.7f) {
		nightmareAtmosphere.shadowOffset = -1;
	} else {
		nightmareAtmosphere.shadowOffset = 0;
	}
}

void ApplyNightmareDesaturation(uint8_t &r, uint8_t &g, uint8_t &b)
{
	if (!nightmareAtmosphere.enabled || nightmareAtmosphere.desaturationLevel <= 0.0f) {
		return;
	}
	
	// Log periódico para mostrar que la desaturación está activa
	static uint32_t lastLogTime = 0;
	uint32_t currentTime = SDL_GetTicks();
	
	if (currentTime - lastLogTime > 5000) {  // Log cada 5 segundos
		LogVerbose("🌙 DARKENING ACTIVE: Applying {:.1f}% desaturation - PERMANENT MODE", 
			nightmareAtmosphere.desaturationLevel * 100.0f);
		lastLogTime = currentTime;
	}
	
	// Calcular luminancia (percepción de brillo)
	float luminance = 0.299f * r + 0.587f * g + 0.114f * b;
	
	// Aplicar desaturación mezclando con luminancia
	float factor = nightmareAtmosphere.desaturationLevel;
	
	r = static_cast<uint8_t>(r * (1.0f - factor) + luminance * factor);
	g = static_cast<uint8_t>(g * (1.0f - factor) + luminance * factor);
	b = static_cast<uint8_t>(b * (1.0f - factor) + luminance * factor);
}

int GetWindShadowOffset()
{
	if (!nightmareAtmosphere.enabled) {
		return 0;
	}
	
	return nightmareAtmosphere.shadowOffset;
}

int GetNPCBreathingOffset(int npcId)
{
	if (!nightmareAtmosphere.enabled || !nightmareAtmosphere.breathingActive) {
		return 0;
	}
	
	// Usar ID del NPC para crear variación entre NPCs
	float breathingPhase = (nightmareAtmosphere.windTick + npcId * 1000) * BREATHING_FREQUENCY;
	float breathingValue = std::sin(breathingPhase);
	
	// Respiración muy sutil (±1 píxel)
	return static_cast<int>(breathingValue);
}

void SetNightmareAtmosphereEnabled(bool enabled)
{
	LogVerbose("Nightmare Atmosphere: {}", enabled ? "enabled" : "disabled");
	nightmareAtmosphere.enabled = enabled;
	
	if (!enabled) {
		// Reset valores
		nightmareAtmosphere.shadowOffset = 0;
	}
}

bool IsNightmareAtmosphereEnabled()
{
	return nightmareAtmosphere.enabled;
}

void SetDesaturationLevel(float level)
{
	// Limitar entre 0% y 20% para evitar efectos extremos
	nightmareAtmosphere.desaturationLevel = std::clamp(level, 0.0f, 0.2f);
	
	LogVerbose("Nightmare desaturation level set to {:.1f}%", 
	           nightmareAtmosphere.desaturationLevel * 100.0f);
}

} // namespace devilution