/**
 * @file nightmare_menu_effects.cpp
 * @brief Nightmare Edition - Menu Visual Effects Implementation
 * 
 * IMPLEMENTACIÓN DE EFECTOS VISUALES DEL MENÚ
 * ==========================================
 * 
 * Sistema que crea efectos contemplativos visibles
 * en el menú principal de Nightmare Edition.
 */

#include "nightmare_menu_effects.h"

#include <cmath>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global de efectos del menú
NightmareMenuState nightmareMenu;

// Configuración de efectos
namespace {
	constexpr float MENU_FADE_DURATION = 2.0f;         // 2 segundos para fade completo
	constexpr float BUTTON_FADE_DURATION = 1.5f;       // 1.5 segundos por botón
	constexpr float BUTTON_FADE_STAGGER = 0.3f;        // 300ms entre botones
	constexpr uint32_t CLICK_DELAY_MS = 150;           // 150ms delay contemplativo
	constexpr float BREATHING_FREQUENCY = 0.02f;       // Muy lento
	constexpr float BREATHING_AMPLITUDE = 1.0f;        // ±1 píxel
	constexpr float GLOW_FREQUENCY = 0.03f;            // Muy sutil
	constexpr float GLOW_AMPLITUDE = 0.4f;             // Glow sutil
	constexpr int ALPHA_STEP = 32;                     // Saltos discretos de alpha
}

void InitNightmareMenuEffects()
{
	LogVerbose("Initializing Nightmare Menu Effects");
	
	nightmareMenu.enabled = true;
	nightmareMenu.menuEntering = false;
	nightmareMenu.menuExiting = false;
	nightmareMenu.menuAlpha = 0.0f;
	nightmareMenu.hoveredButton = -1;
	nightmareMenu.selectedButton = -1;
	nightmareMenu.lastUpdateTime = SDL_GetTicks();
	nightmareMenu.enterStartTime = 0;
	
	// Inicializar estados de botones
	for (int i = 0; i < 6; i++) {
		nightmareMenu.buttonAlpha[i] = 0.0f;
		nightmareMenu.buttonBreathingOffset[i] = 0;
		nightmareMenu.buttonGlow[i] = 0.0f;
		nightmareMenu.buttonDelayActive[i] = false;
		nightmareMenu.buttonClickTime[i] = 0;
	}
	
	LogVerbose("🌙 Nightmare Menu Effects initialized - Contemplative mode active");
}

void UpdateNightmareMenuEffects(float deltaTime)
{
	if (!nightmareMenu.enabled) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	nightmareMenu.lastUpdateTime = currentTime;
	
	// Actualizar fade de entrada del menú
	if (nightmareMenu.menuEntering) {
		float elapsed = (currentTime - nightmareMenu.enterStartTime) / 1000.0f;
		float progress = std::min(elapsed / MENU_FADE_DURATION, 1.0f);
		
		// Alpha discreto (saltos de 32) - anti-moderno
		int targetAlpha = static_cast<int>(progress * 255);
		int discreteAlpha = (targetAlpha / ALPHA_STEP) * ALPHA_STEP;
		nightmareMenu.menuAlpha = discreteAlpha / 255.0f;
		
		// Actualizar alpha de botones con stagger
		for (int i = 0; i < 6; i++) {
			float buttonDelay = i * BUTTON_FADE_STAGGER;
			float buttonElapsed = std::max(0.0f, elapsed - buttonDelay);
			float buttonProgress = std::min(buttonElapsed / BUTTON_FADE_DURATION, 1.0f);
			
			// Alpha discreto para cada botón
			int buttonTargetAlpha = static_cast<int>(buttonProgress * 255);
			int buttonDiscreteAlpha = (buttonTargetAlpha / ALPHA_STEP) * ALPHA_STEP;
			nightmareMenu.buttonAlpha[i] = buttonDiscreteAlpha / 255.0f;
		}
		
		// Terminar entrada cuando esté completo
		if (progress >= 1.0f) {
			nightmareMenu.menuEntering = false;
			LogVerbose("🌙 Menu enter sequence completed");
		}
	}
	
	// Actualizar efectos de breathing en botones con hover
	for (int i = 0; i < 6; i++) {
		if (nightmareMenu.hoveredButton == i) {
			// Breathing effect usando seno
			float time = currentTime / 1000.0f;
			float breathingValue = std::sin(time * BREATHING_FREQUENCY);
			nightmareMenu.buttonBreathingOffset[i] = static_cast<int>(breathingValue * BREATHING_AMPLITUDE);
			
			// Glow effect usando seno
			float glowValue = (std::sin(time * GLOW_FREQUENCY) + 1.0f) * 0.5f;
			nightmareMenu.buttonGlow[i] = glowValue * GLOW_AMPLITUDE;
		} else {
			// Fade out breathing y glow cuando no hay hover
			nightmareMenu.buttonBreathingOffset[i] = 0;
			nightmareMenu.buttonGlow[i] = std::max(0.0f, nightmareMenu.buttonGlow[i] - deltaTime * 2.0f);
		}
		
		// Actualizar delays contemplativos
		if (nightmareMenu.buttonDelayActive[i]) {
			uint32_t elapsed = currentTime - nightmareMenu.buttonClickTime[i];
			if (elapsed >= CLICK_DELAY_MS) {
				nightmareMenu.buttonDelayActive[i] = false;
				LogVerbose("⏳ Button {} contemplative delay completed", i);
			}
		}
	}
}

void StartMenuEnterSequence()
{
	if (!nightmareMenu.enabled) {
		return;
	}
	
	nightmareMenu.menuEntering = true;
	nightmareMenu.menuExiting = false;
	nightmareMenu.menuAlpha = 0.0f;
	nightmareMenu.enterStartTime = SDL_GetTicks();
	
	// Reset alpha de todos los botones
	for (int i = 0; i < 6; i++) {
		nightmareMenu.buttonAlpha[i] = 0.0f;
	}
	
	LogVerbose("🌙 Menu enter sequence started - 2s contemplative fade");
}

void StartMenuExitSequence()
{
	if (!nightmareMenu.enabled) {
		return;
	}
	
	nightmareMenu.menuExiting = true;
	nightmareMenu.menuEntering = false;
	
	LogVerbose("🌙 Menu exit sequence started");
}

void HandleMenuButtonHover(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return;
	}
	
	nightmareMenu.hoveredButton = buttonIndex;
	
	LogVerbose("🫁 Menu button {} hover - breathing effect started", buttonIndex);
}

void HandleMenuButtonUnhover(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return;
	}
	
	if (nightmareMenu.hoveredButton == buttonIndex) {
		nightmareMenu.hoveredButton = -1;
	}
	
	LogVerbose("🫁 Menu button {} unhover - breathing effect stopping", buttonIndex);
}

bool HandleMenuButtonClick(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return true; // Procesar inmediatamente si efectos deshabilitados
	}
	
	// Iniciar delay contemplativo
	nightmareMenu.buttonDelayActive[buttonIndex] = true;
	nightmareMenu.buttonClickTime[buttonIndex] = SDL_GetTicks();
	
	LogVerbose("⏳ Menu button {} click - {}ms contemplative delay", buttonIndex, CLICK_DELAY_MS);
	
	return false; // No procesar inmediatamente
}

float GetMenuAlpha()
{
	return nightmareMenu.enabled ? nightmareMenu.menuAlpha : 1.0f;
}

float GetMenuButtonAlpha(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return 1.0f;
	}
	
	return nightmareMenu.buttonAlpha[buttonIndex];
}

int GetMenuButtonBreathingOffset(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return 0;
	}
	
	return nightmareMenu.buttonBreathingOffset[buttonIndex];
}

float GetMenuButtonGlow(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return 0.0f;
	}
	
	return nightmareMenu.buttonGlow[buttonIndex];
}

bool IsMenuButtonInDelay(int buttonIndex)
{
	if (!nightmareMenu.enabled || buttonIndex < 0 || buttonIndex >= 6) {
		return false;
	}
	
	return nightmareMenu.buttonDelayActive[buttonIndex];
}

void SetNightmareMenuEffectsEnabled(bool enabled)
{
	LogVerbose("Nightmare Menu Effects: {}", enabled ? "enabled" : "disabled");
	nightmareMenu.enabled = enabled;
	
	if (!enabled) {
		// Reset todo al estado normal
		nightmareMenu = {};
	}
}

} // namespace devilution