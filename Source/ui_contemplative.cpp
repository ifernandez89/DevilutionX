/**
 * @file ui_contemplative.cpp
 * @brief Nightmare Edition - Contemplative UI Effects Implementation
 * 
 * IMPLEMENTACIÓN DE EFECTOS CONTEMPLATIVOS
 * =======================================
 * 
 * Sistema que crea una experiencia de UI única y contemplativa
 * para Nightmare Edition.
 */

#include "ui_contemplative.h"

#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema contemplativo
ContemplativeUIState contemplativeUI;

// Configuración del sistema
namespace {
	constexpr uint32_t HOVER_DELAY_MS = 100;           // 100ms delay para hover
	constexpr uint32_t CLICK_DELAY_MS = 150;           // 150ms delay para click
	constexpr uint32_t SELECTION_DELAY_MS = 200;       // 200ms delay para selección
	constexpr float HOVER_BREATHING_INTENSITY = 0.3f;  // Intensidad de breathing en hover
	constexpr float SELECTION_GLOW_INTENSITY = 0.4f;   // Intensidad de glow en selección
	constexpr float PRESS_FADE_INTENSITY = 0.8f;       // Intensidad de fade en press
}

void InitContemplativeUI()
{
	LogVerbose("Initializing Nightmare Contemplative UI System");
	
	contemplativeUI.enabled = true;
	contemplativeUI.contemplationLevel = 0.7f;  // Nivel contemplativo por defecto
	
	// Inicializar todos los elementos contemplativos
	contemplativeUI.mainMenuButtons = {};
	contemplativeUI.gameMenuButtons = {};
	contemplativeUI.inventoryPanels = {};
	contemplativeUI.dialogBoxes = {};
	
	LogVerbose("Contemplative UI System initialized - Contemplation level: {:.2f}", 
		contemplativeUI.contemplationLevel);
}

void UpdateContemplativeUI(float deltaTime)
{
	if (!contemplativeUI.enabled) {
		return;
	}
	
	// Actualizar todas las transiciones de elementos contemplativos
	UpdateSingleTransition(contemplativeUI.mainMenuButtons.transition, deltaTime);
	UpdateSingleTransition(contemplativeUI.gameMenuButtons.transition, deltaTime);
	UpdateSingleTransition(contemplativeUI.inventoryPanels.transition, deltaTime);
	UpdateSingleTransition(contemplativeUI.dialogBoxes.transition, deltaTime);
}

void HandleContemplativeHover(ContemplativeElement &element)
{
	if (!contemplativeUI.enabled || element.isHovered) {
		return;
	}
	
	element.isHovered = true;
	element.lastInteraction = SDL_GetTicks();
	
	// Iniciar efecto de breathing sutil en hover
	StartBreathingEffect(element.transition, HOVER_BREATHING_INTENSITY * contemplativeUI.contemplationLevel);
	
	LogVerbose("🫁 Contemplative hover started - breathing effect active");
}

void HandleContemplativeUnhover(ContemplativeElement &element)
{
	if (!contemplativeUI.enabled || !element.isHovered) {
		return;
	}
	
	element.isHovered = false;
	element.lastInteraction = SDL_GetTicks();
	
	// Detener efecto de breathing
	element.transition.state = TransitionState::Idle;
	
	LogVerbose("🫁 Contemplative hover ended - breathing effect stopped");
}

bool HandleContemplativeClick(ContemplativeElement &element)
{
	if (!contemplativeUI.enabled) {
		return true;  // Procesar click inmediatamente si el sistema está deshabilitado
	}
	
	element.isPressed = true;
	element.lastInteraction = SDL_GetTicks();
	
	// Iniciar delay contemplativo antes de procesar el click
	StartContemplativeDelay(element.transition, CLICK_DELAY_MS);
	
	LogVerbose("⏳ Contemplative click started - {}ms delay", CLICK_DELAY_MS);
	
	// El click se procesará cuando el delay termine
	return false;
}

void HandleContemplativeSelection(ContemplativeElement &element)
{
	if (!contemplativeUI.enabled) {
		return;
	}
	
	element.isSelected = true;
	element.lastInteraction = SDL_GetTicks();
	
	// Iniciar efecto de glow sutil en selección
	StartSubtleGlow(element.transition, SELECTION_GLOW_INTENSITY * contemplativeUI.contemplationLevel);
	
	LogVerbose("✨ Contemplative selection - subtle glow active");
}

int GetContemplativeRenderOffset(const ContemplativeElement &element)
{
	if (!contemplativeUI.enabled) {
		return 0;
	}
	
	// Obtener offset de breathing si está activo
	if (element.isHovered && element.transition.state == TransitionState::Breathing) {
		return GetBreathingOffset(element.transition);
	}
	
	return 0;
}

float GetContemplativeAlpha(const ContemplativeElement &element)
{
	if (!contemplativeUI.enabled) {
		return 1.0f;
	}
	
	// Aplicar fade si está en transición
	if (element.transition.state == TransitionState::FadeIn || 
		element.transition.state == TransitionState::FadeOut) {
		return GetFadeAlpha(element.transition);
	}
	
	// Aplicar fade sutil si está siendo presionado
	if (element.isPressed && element.transition.state == TransitionState::Contemplating) {
		float pressAlpha = 1.0f - (element.transition.progress * 0.2f);  // Fade muy sutil
		return std::clamp(pressAlpha, 0.8f, 1.0f);
	}
	
	return 1.0f;
}

float GetContemplativeGlow(const ContemplativeElement &element)
{
	if (!contemplativeUI.enabled) {
		return 0.0f;
	}
	
	// Obtener intensidad de glow si está activo
	if (element.isSelected && element.transition.state == TransitionState::Glowing) {
		return GetGlowIntensity(element.transition);
	}
	
	return 0.0f;
}

bool IsInContemplativeDelay(const ContemplativeElement &element)
{
	if (!contemplativeUI.enabled) {
		return false;
	}
	
	return element.transition.state == TransitionState::Contemplating && 
		   !IsContemplativeDelayComplete(element.transition);
}

void SetContemplationLevel(float level)
{
	contemplativeUI.contemplationLevel = std::clamp(level, 0.0f, 1.0f);
	LogVerbose("Contemplation level set to: {:.2f}", contemplativeUI.contemplationLevel);
	
	// Ajustar la intensidad global de transiciones
	SetGlobalTransitionIntensity(contemplativeUI.contemplationLevel);
}

void SetContemplativeUIEnabled(bool enabled)
{
	LogVerbose("Contemplative UI: {}", enabled ? "enabled" : "disabled");
	contemplativeUI.enabled = enabled;
	
	if (!enabled) {
		// Reset todos los elementos al estado normal
		contemplativeUI.mainMenuButtons = {};
		contemplativeUI.gameMenuButtons = {};
		contemplativeUI.inventoryPanels = {};
		contemplativeUI.dialogBoxes = {};
	}
}

} // namespace devilution