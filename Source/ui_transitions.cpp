/**
 * @file ui_transitions.cpp
 * @brief Nightmare Edition - UI Transitions Implementation
 * 
 * IMPLEMENTACIÓN DE TRANSICIONES CONTEMPLATIVAS
 * ============================================
 * 
 * Sistema que crea transiciones sutiles y contemplativas
 * para una experiencia de UI única en Nightmare Edition.
 */

#include "ui_transitions.h"

#include <cmath>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema de transiciones
UITransitionsState uiTransitions;

// Configuración del sistema
namespace {
	constexpr float DEFAULT_FADE_DURATION = 1.5f;      // 1.5 segundos - muy contemplativo
	constexpr float DEFAULT_BREATHING_SPEED = 0.02f;   // Muy lento
	constexpr float DEFAULT_GLOW_SPEED = 0.03f;        // Muy sutil
	constexpr uint32_t DEFAULT_DELAY_MS = 150;         // 150ms micro-delay
	constexpr float BREATHING_AMPLITUDE = 0.5f;        // Amplitud sutil
	constexpr float GLOW_AMPLITUDE = 0.6f;             // Glow sutil
}

void InitUITransitions()
{
	LogVerbose("Initializing Nightmare UI Transitions System");
	
	uiTransitions.enabled = true;
	uiTransitions.globalIntensity = 0.7f;  // Intensidad sutil por defecto
	uiTransitions.lastUpdateTime = SDL_GetTicks();
	
	// Inicializar todas las transiciones en estado idle
	uiTransitions.menuFade = {};
	uiTransitions.buttonHover = {};
	uiTransitions.panelTransition = {};
	uiTransitions.contemplativeDelay = {};
	
	LogVerbose("UI Transitions System initialized - Contemplative mode active");
}

void UpdateUITransitions(float deltaTime)
{
	if (!uiTransitions.enabled) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	uiTransitions.lastUpdateTime = currentTime;
	
	// Actualizar todas las transiciones activas
	UpdateSingleTransition(uiTransitions.menuFade, deltaTime);
	UpdateSingleTransition(uiTransitions.buttonHover, deltaTime);
	UpdateSingleTransition(uiTransitions.panelTransition, deltaTime);
	UpdateSingleTransition(uiTransitions.contemplativeDelay, deltaTime);
}

/**
 * @brief Actualiza una transición individual
 */
void UpdateSingleTransition(UITransition &transition, float deltaTime)
{
	if (transition.state == TransitionState::Idle) {
		return;
	}
	
	// Calcular progreso basado en tiempo transcurrido
	uint32_t currentTime = SDL_GetTicks();
	float elapsed = (currentTime - transition.startTime) / 1000.0f;
	
	switch (transition.state) {
		case TransitionState::FadeIn:
		case TransitionState::FadeOut:
			transition.progress = std::min(elapsed / transition.duration, 1.0f);
			if (transition.progress >= 1.0f && !transition.loop) {
				transition.state = TransitionState::Idle;
			}
			break;
			
		case TransitionState::Breathing:
			// Efecto de respiración continuo usando seno
			transition.progress = (std::sin(elapsed * DEFAULT_BREATHING_SPEED) + 1.0f) * 0.5f;
			break;
			
		case TransitionState::Glowing:
			// Efecto de glow continuo usando seno
			transition.progress = (std::sin(elapsed * DEFAULT_GLOW_SPEED) + 1.0f) * 0.5f;
			break;
			
		case TransitionState::Contemplating:
			transition.progress = std::min(elapsed / (transition.duration / 1000.0f), 1.0f);
			if (transition.progress >= 1.0f) {
				transition.state = TransitionState::Idle;
			}
			break;
			
		default:
			break;
	}
}

void StartContemplativeFade(UITransition &transition, bool fadeIn, float duration)
{
	transition.state = fadeIn ? TransitionState::FadeIn : TransitionState::FadeOut;
	transition.duration = duration;
	transition.progress = 0.0f;
	transition.startTime = SDL_GetTicks();
	transition.loop = false;
	
	LogVerbose("🌙 Starting contemplative fade: {} ({}s)", 
		fadeIn ? "in" : "out", duration);
}

void StartBreathingEffect(UITransition &transition, float intensity)
{
	transition.state = TransitionState::Breathing;
	transition.intensity = std::clamp(intensity, 0.0f, 1.0f);
	transition.progress = 0.0f;
	transition.startTime = SDL_GetTicks();
	transition.loop = true;
	
	LogVerbose("🫁 Starting breathing effect: intensity {:.2f}", intensity);
}

void StartSubtleGlow(UITransition &transition, float intensity)
{
	transition.state = TransitionState::Glowing;
	transition.intensity = std::clamp(intensity, 0.0f, 1.0f);
	transition.progress = 0.0f;
	transition.startTime = SDL_GetTicks();
	transition.loop = true;
	
	LogVerbose("✨ Starting subtle glow: intensity {:.2f}", intensity);
}

void StartContemplativeDelay(UITransition &transition, uint32_t delayMs)
{
	transition.state = TransitionState::Contemplating;
	transition.duration = static_cast<float>(delayMs);  // Almacenar en ms
	transition.progress = 0.0f;
	transition.startTime = SDL_GetTicks();
	transition.loop = false;
	
	LogVerbose("⏳ Starting contemplative delay: {}ms", delayMs);
}

float GetFadeAlpha(const UITransition &transition)
{
	if (transition.state == TransitionState::Idle) {
		return 1.0f;
	}
	
	float alpha = 1.0f;
	float adjustedProgress = transition.progress * uiTransitions.globalIntensity;
	
	switch (transition.state) {
		case TransitionState::FadeIn:
			// Curva suave de fade in (ease-out)
			alpha = 1.0f - std::pow(1.0f - adjustedProgress, 3.0f);
			break;
			
		case TransitionState::FadeOut:
			// Curva suave de fade out (ease-in)
			alpha = std::pow(1.0f - adjustedProgress, 3.0f);
			break;
			
		default:
			alpha = 1.0f;
			break;
	}
	
	return std::clamp(alpha, 0.0f, 1.0f);
}

int GetBreathingOffset(const UITransition &transition)
{
	if (transition.state != TransitionState::Breathing) {
		return 0;
	}
	
	// Breathing muy sutil: ±1 píxel máximo
	float breathingValue = (transition.progress - 0.5f) * 2.0f;  // -1 a +1
	float adjustedValue = breathingValue * transition.intensity * uiTransitions.globalIntensity;
	
	// Convertir a píxeles (máximo ±1)
	return static_cast<int>(std::round(adjustedValue * BREATHING_AMPLITUDE));
}

float GetGlowIntensity(const UITransition &transition)
{
	if (transition.state != TransitionState::Glowing) {
		return 0.0f;
	}
	
	// Glow sutil que oscila
	float glowValue = transition.progress;  // 0 a 1
	float adjustedValue = glowValue * transition.intensity * uiTransitions.globalIntensity;
	
	return std::clamp(adjustedValue * GLOW_AMPLITUDE, 0.0f, 1.0f);
}

bool IsContemplativeDelayComplete(const UITransition &transition)
{
	return transition.state == TransitionState::Idle || 
		   (transition.state == TransitionState::Contemplating && transition.progress >= 1.0f);
}

void SetUITransitionsEnabled(bool enabled)
{
	LogVerbose("UI Transitions: {}", enabled ? "enabled" : "disabled");
	uiTransitions.enabled = enabled;
	
	if (!enabled) {
		// Reset todas las transiciones al estado idle
		uiTransitions.menuFade.state = TransitionState::Idle;
		uiTransitions.buttonHover.state = TransitionState::Idle;
		uiTransitions.panelTransition.state = TransitionState::Idle;
		uiTransitions.contemplativeDelay.state = TransitionState::Idle;
	}
}

void SetGlobalTransitionIntensity(float intensity)
{
	uiTransitions.globalIntensity = std::clamp(intensity, 0.0f, 1.0f);
	LogVerbose("UI Transitions global intensity set to: {:.2f}", uiTransitions.globalIntensity);
}

} // namespace devilution