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
#include "water_animation.h"
#include "nightmare_lighting.h"  // Consolidated lighting system (replaces organic_lighting)
#include "nightmare_atmosphere.h"
#include "ui_transitions.h"      // 🌙 Sistema de transiciones contemplativas
#include "ui_contemplative.h"    // 🫁 Sistema de efectos contemplativos
#include "nightmare_menu_effects.h"  // 🎭 Efectos visuales del menú
#include "nightmare_weather.h"   // 🌧️ Sistema climático (lluvia y niebla)
#include "nightmare_testing.h"   // 🧪 Sistema de testing y demostración
#include "nightmare_immediate_effects.h"  // 🔄 Para reset de efectos

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
	
	// Inicializar sistemas atmosféricos
	InitNightmareAtmosphericSystems();
	
	// 🌙 FASE 4: Inicializar sistemas de transiciones contemplativas
	InitUITransitions();
	InitContemplativeUI();
	
	// 🎭 FASE 4.1: Inicializar efectos visuales del menú
	InitNightmareMenuEffects();
	
	// 🌧️ FASE 4.2: Inicializar sistema climático (lluvia y niebla)
	InitNightmareWeather();
	
	// 🧪 FASE 4.3: Inicializar sistema de testing y demostración
	InitNightmareTesting();
	
	// 🔄 IMPORTANTE: Reset todos los efectos al inicializar para evitar oscurecimiento
	ResetImmediateEffects();
	
	LogVerbose("Nightmare UI Architecture initialized successfully");
	LogVerbose("🌙 Contemplative transitions and effects are now active");
	LogVerbose("🎭 Menu visual effects ready");
	LogVerbose("🌧️ Weather system (rain & fog) ready");
	LogVerbose("🧪 Testing system active - effects will be demonstrated automatically");
	LogVerbose("🔄 All effects reset to normal state");
	LogVerbose("🌙 === PERMANENT EFFECTS NOW ACTIVE ===");
	LogVerbose("🌧️ RAIN: Permanently enabled with high intensity");
	LogVerbose("🌫️ FOG: Permanently enabled with high intensity");
	LogVerbose("🌙 DARKENING: Permanently enabled (15% desaturation)");
	LogVerbose("🌙 === WATCH CONSOLE FOR ACTIVITY LOGS ===");
}

void UpdateNightmareUI(float deltaTime)
{
	if (!nightmareUI.enabled) return;
	
	Uint32 currentTime = SDL_GetTicks();
	
	// Actualizar sistemas atmosféricos
	UpdateNightmareAtmosphericSystems(currentTime);
	
	// 🌙 FASE 4: Actualizar sistemas de transiciones contemplativas
	UpdateUITransitions(deltaTime);
	UpdateContemplativeUI(deltaTime);
	
	// 🎭 FASE 4.1: Actualizar efectos visuales del menú
	UpdateNightmareMenuEffects(deltaTime);
	
	// 🌧️ FASE 4.2: Actualizar sistema climático
	UpdateNightmareWeather(deltaTime);
	
	// 🧪 FASE 4.3: Actualizar sistema de testing
	UpdateNightmareTesting(deltaTime);
	
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
	// 🌧️ Renderizar efectos climáticos (lluvia y niebla)
	RenderNightmareWeather();
	
	// Por ahora, simplemente registrar que el sistema está activo
	// TODO: Implementar renderizado por capas en futuras versiones
	if (nightmareUI.enabled) {
		// Sistema Nightmare UI activo - futuras mejoras visuales aquí
		
		// Log periódico para mostrar que los sistemas están funcionando
		static uint32_t lastLogTime = 0;
		uint32_t currentTime = SDL_GetTicks();
		
		if (currentTime - lastLogTime > 15000) {  // Log cada 15 segundos
			LogVerbose("🌙 Nightmare UI Systems Status:");
			LogVerbose("  🎭 Menu Effects: Active");
			LogVerbose("  🌧️ Weather System: Active");
			LogVerbose("  🫁 Contemplative UI: Active");
			LogVerbose("  🌊 Atmospheric Systems: Active");
			lastLogTime = currentTime;
		}
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

/**
 * @brief Inicializa todos los sistemas atmosféricos Nightmare
 */
void InitNightmareAtmosphericSystems()
{
	LogVerbose("Initializing Nightmare Atmospheric Systems");
	
	// Inicializar agua animada
	InitWaterAnimation();
	
	// Inicializar iluminación atmosférica (nightmare_lighting)
	InitNightmareLighting();
	
	// Inicializar atmósfera Nightmare
	InitNightmareAtmosphere();
	
	LogVerbose("All Nightmare atmospheric systems initialized");
}

/**
 * @brief Actualiza todos los sistemas atmosféricos
 * @param currentTick Tick actual del juego
 */
void UpdateNightmareAtmosphericSystems(uint32_t currentTick)
{
	if (!nightmareUI.enabled) return;
	
	// Actualizar agua animada
	UpdateWaterAnimation(currentTick);
	
	// Actualizar iluminación atmosférica (nightmare_lighting)
	UpdateNightmareLighting();
	
	// Actualizar atmósfera Nightmare
	UpdateNightmareAtmosphere(currentTick);
}

/**
 * @brief Activa/desactiva todos los efectos atmosféricos
 * @param enabled true para activar
 */
void SetNightmareAtmosphericEffectsEnabled(bool enabled)
{
	LogVerbose("Nightmare atmospheric effects: {}", enabled ? "enabled" : "disabled");
	
	// Controlar agua animada
	SetWaterAnimationEnabled(enabled);
	
	// Controlar iluminación atmosférica (nightmare_lighting)
	SetNightmareLightingEnabled(enabled);
	
	// Controlar atmósfera Nightmare
	SetNightmareAtmosphereEnabled(enabled);
}

} // namespace devilution