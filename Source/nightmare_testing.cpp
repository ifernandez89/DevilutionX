/**
 * @file nightmare_testing.cpp
 * @brief Nightmare Edition - Testing & Demo Implementation
 * 
 * IMPLEMENTACIÓN DEL SISTEMA DE TESTING
 * ====================================
 * 
 * Sistema para probar todos los efectos Nightmare.
 */

#include "nightmare_testing.h"

#include "nightmare_menu_effects.h"
#include "nightmare_weather.h"
#include "ui_contemplative.h"
#include "ui_transitions.h"
#include "utils/log.hpp"

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// Estado del sistema de testing
namespace {
	bool testingInitialized = false;
	uint32_t lastDemoTime = 0;
	int currentTestButton = 0;
	bool rainToggleState = false;
	bool fogToggleState = true;  // Niebla habilitada por defecto
}

void InitNightmareTesting()
{
	LogVerbose("🧪 Initializing Nightmare Testing System");
	
	testingInitialized = true;
	lastDemoTime = SDL_GetTicks();
	currentTestButton = 0;
	
	// Iniciar demostración automática
	StartMenuEffectsDemo();
	StartWeatherEffectsDemo();
	
	LogVerbose("🧪 Nightmare Testing System initialized");
	LogVerbose("🎭 Menu effects demo started");
	LogVerbose("🌧️ Weather effects demo started");
}

void UpdateNightmareTesting(float deltaTime)
{
	if (!testingInitialized) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	
	// Demo automático cada 5 segundos
	if (currentTime - lastDemoTime > 5000) {
		// Simular hover en diferentes botones
		TestMenuButtonHover(currentTestButton);
		
		// Cambiar al siguiente botón
		currentTestButton = (currentTestButton + 1) % 6;
		
		lastDemoTime = currentTime;
		
		// Mostrar estado cada 30 segundos
		static uint32_t lastStatusTime = 0;
		if (currentTime - lastStatusTime > 30000) {
			ShowNightmareSystemsStatus();
			lastStatusTime = currentTime;
		}
	}
}

void StartMenuEffectsDemo()
{
	LogVerbose("🎭 Starting Menu Effects Demo");
	
	// Iniciar secuencia de entrada del menú
	StartMenuEnterSequence();
	
	// Simular hover en el primer botón después de 2 segundos
	// (esto se manejará en UpdateNightmareTesting)
	
	LogVerbose("🎭 Menu effects demo active - watch for breathing and glow effects");
}

void StartWeatherEffectsDemo()
{
	LogVerbose("🌧️ Starting Weather Effects Demo");
	
	// Habilitar niebla por defecto
	SetFogEnabled(true);
	SetFogIntensity(0.7f);
	
	// Lluvia deshabilitada por defecto (se puede activar con toggle)
	SetRainEnabled(false);
	SetRainIntensity(0.5f);
	
	LogVerbose("🌫️ Fog enabled - watch console for fog frame updates");
	LogVerbose("🌧️ Rain disabled - use toggle to enable");
}

void TestMenuButtonHover(int buttonIndex)
{
	if (buttonIndex < 0 || buttonIndex >= 6) {
		return;
	}
	
	// Simular hover en el botón
	HandleMenuButtonHover(buttonIndex);
	
	LogVerbose("🧪 Testing hover on menu button {} - breathing effect should be active", buttonIndex);
	
	// Programar unhover después de 3 segundos
	// (En una implementación real, esto se manejaría con un timer)
}

void TestMenuButtonClick(int buttonIndex)
{
	if (buttonIndex < 0 || buttonIndex >= 6) {
		return;
	}
	
	// Simular click en el botón
	bool processImmediately = HandleMenuButtonClick(buttonIndex);
	
	LogVerbose("🧪 Testing click on menu button {} - delay: {}", 
		buttonIndex, processImmediately ? "none" : "150ms");
}

void ToggleRainForTesting()
{
	rainToggleState = !rainToggleState;
	SetRainEnabled(rainToggleState);
	
	LogVerbose("🧪 Rain toggled: {} - watch console for rain updates", 
		rainToggleState ? "ON" : "OFF");
}

void ToggleFogForTesting()
{
	fogToggleState = !fogToggleState;
	SetFogEnabled(fogToggleState);
	
	LogVerbose("🧪 Fog toggled: {} - watch console for fog frame updates", 
		fogToggleState ? "ON" : "OFF");
}

void ShowNightmareSystemsStatus()
{
	LogVerbose("🌙 === NIGHTMARE SYSTEMS STATUS ===");
	
	// Estado del menú
	float menuAlpha = GetMenuAlpha();
	LogVerbose("🎭 Menu Effects:");
	LogVerbose("  - Menu Alpha: {:.2f}", menuAlpha);
	LogVerbose("  - Current Test Button: {}", currentTestButton);
	
	// Estado de botones
	for (int i = 0; i < 6; i++) {
		float buttonAlpha = GetMenuButtonAlpha(i);
		int breathingOffset = GetMenuButtonBreathingOffset(i);
		float glow = GetMenuButtonGlow(i);
		bool inDelay = IsMenuButtonInDelay(i);
		
		if (buttonAlpha > 0.0f || breathingOffset != 0 || glow > 0.0f || inDelay) {
			LogVerbose("  - Button {}: alpha={:.2f}, breathing={}, glow={:.2f}, delay={}", 
				i, buttonAlpha, breathingOffset, glow, inDelay ? "yes" : "no");
		}
	}
	
	// Estado climático
	LogVerbose("🌦️ Weather Effects:");
	LogVerbose("  - Rain: {}", rainToggleState ? "ENABLED" : "disabled");
	LogVerbose("  - Fog: {}", fogToggleState ? "ENABLED" : "disabled");
	
	// Estado de transiciones
	LogVerbose("🌙 Transition Systems:");
	LogVerbose("  - UI Transitions: ACTIVE");
	LogVerbose("  - Contemplative UI: ACTIVE");
	
	LogVerbose("🌙 === END STATUS ===");
}

} // namespace devilution