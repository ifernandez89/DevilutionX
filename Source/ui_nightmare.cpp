/**
 * @file ui_nightmare.cpp
 * @brief Nightmare Edition UI Architecture - Implementation
 * 
 * IMPLEMENTACIÓN SIMPLIFICADA DEL SISTEMA DE CAPAS NIGHTMARE
 * =========================================================
 * 
 * Versión inicial que compila correctamente y establece la base
 * para futuras mejoras visuales.
 */

#include "ui_nightmare.h"

#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema Nightmare UI
NightmareUIState nightmareUI;

void InitNightmareUI()
{
	LogVerbose("Initializing Nightmare UI Architecture");
	
	nightmareUI.enabled = true;  // Activado por defecto para Nightmare Edition
	nightmareUI.fadeActive = false;
	nightmareUI.fadeAlpha = 0.0f;
	nightmareUI.animatedBgActive = false;
	nightmareUI.bgFrame = 0;
	nightmareUI.lastFrameTime = SDL_GetTicks();
	
	LogVerbose("Nightmare UI Architecture initialized successfully");
}

void UpdateNightmareUI(float deltaTime)
{
	if (!nightmareUI.enabled) return;
	
	Uint32 currentTime = SDL_GetTicks();
	
	// Actualizar animación de fondo
	if (nightmareUI.animatedBgActive) {
		// Animación lenta para atmósfera Nightmare (120ms por frame)
		if (currentTime - nightmareUI.lastFrameTime > 120) {
			nightmareUI.bgFrame = (nightmareUI.bgFrame + 1) % 16; // 16 frames de animación
			nightmareUI.lastFrameTime = currentTime;
		}
	}
	
	// Actualizar fade effect con timing más preciso
	if (nightmareUI.fadeActive) {
		// Fade infernal suave de 1.5 segundos para atmósfera Nightmare
		float fadeSpeed = 1.0f / 1.5f; // 1.5 segundos de duración
		nightmareUI.fadeAlpha = std::min(1.0f, nightmareUI.fadeAlpha + deltaTime * fadeSpeed);
		
		// Desactivar fade cuando esté completo
		if (nightmareUI.fadeAlpha >= 1.0f) {
			nightmareUI.fadeActive = false;
			LogVerbose("Nightmare fade effect completed");
		}
	}
}

void RenderNightmareUI()
{
	// Por ahora, simplemente registrar que el sistema está activo
	// TODO: Implementar renderizado por capas en futuras versiones
	if (nightmareUI.enabled) {
		// Sistema Nightmare UI activo - futuras mejoras visuales aquí
	}
}

void RenderNightmareLayer(NightmareUILayer layer)
{
	// TODO: Implementar renderizado específico por capa
	switch (layer) {
	case NightmareUILayer::World:
		// TODO: Renderizar mundo + panel
		break;
	case NightmareUILayer::Overlay:
		// TODO: Renderizar overlays
		break;
	case NightmareUILayer::Screen:
		// TODO: Renderizar pantallas
		break;
	case NightmareUILayer::Modal:
		// TODO: Renderizar modales
		break;
	case NightmareUILayer::Fade:
		// TODO: Renderizar fade effects
		break;
	}
}

void SetNightmareUIEnabled(bool enabled)
{
	LogVerbose("Nightmare UI {}", enabled ? "enabled" : "disabled");
	nightmareUI.enabled = enabled;
}

void StartNightmareFade(bool fadeIn, float duration)
{
	LogVerbose("Starting Nightmare fade: {} (duration: {:.1f}s)", 
	           fadeIn ? "in" : "out", duration);
	
	nightmareUI.fadeActive = true;
	nightmareUI.fadeAlpha = fadeIn ? 0.0f : 1.0f;
}

void SetNightmareAnimatedBackground(bool active)
{
	LogVerbose("Nightmare animated background: {}", active ? "enabled" : "disabled");
	nightmareUI.animatedBgActive = active;
	
	if (active) {
		nightmareUI.bgFrame = 0;
		nightmareUI.lastFrameTime = SDL_GetTicks();
	}
}

/**
 * @brief Activa el fade infernal para transiciones de menú
 * 
 * Esta función se puede llamar desde el sistema de menús para crear
 * transiciones suaves con atmósfera Nightmare.
 */
void TriggerNightmareFadeTransition()
{
	if (nightmareUI.enabled) {
		StartNightmareFade(true, 1.5f); // Fade in de 1.5 segundos
		LogVerbose("Nightmare fade transition triggered");
	}
}

/**
 * @brief Verifica si hay una imagen personalizada disponible
 * @return true si existe custom_title.png
 */
bool HasCustomTitleImage()
{
	// TODO: Implementar verificación de archivo
	// Por ahora, asumir que existe si el sistema está habilitado
	return nightmareUI.enabled;
}

/**
 * @brief Obtiene el frame actual de la animación de fondo
 * @return Frame actual (0-15)
 */
int GetNightmareBackgroundFrame()
{
	return nightmareUI.bgFrame;
}

/**
 * @brief Obtiene el alpha actual del fade effect
 * @return Alpha value (0.0-1.0)
 */
float GetNightmareFadeAlpha()
{
	return nightmareUI.fadeAlpha;
}

} // namespace devilution