/**
 * @file nightmare_menu_contemplative.cpp
 * @brief Nightmare Edition - Contemplative Menu Implementation
 * 
 * MENÚ COMO ANTESALA DEL INFIERNO - IMPLEMENTACIÓN
 * ===============================================
 * 
 * "El menú debe sentirse como estar parado frente a una catedral cerrada.
 *  No te invita. Te advierte."
 */

#include "nightmare_menu_contemplative.h"

#include <cmath>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"
#include "engine/dx.h"

namespace devilution {

// Estado global del menú contemplativo
ContemplativeMenuState contemplativeMenu;

void InitContemplativeMenu()
{
	LogVerbose("Initializing Contemplative Menu System");
	
	contemplativeMenu.enabled = true;
	contemplativeMenu.startTime = SDL_GetTicks();
	contemplativeMenu.lastUpdateTime = contemplativeMenu.startTime;
	
	// 🌫️ Inicializar fondo vivo
	contemplativeMenu.background.fogFrame = 0;
	contemplativeMenu.background.lastFogTime = contemplativeMenu.startTime;
	contemplativeMenu.background.fogSeed = rand() % 1000;  // Variación sutil
	contemplativeMenu.background.pulseBrightness = 0.0f;
	contemplativeMenu.background.lastPulseTime = contemplativeMenu.startTime;
	
	// 📜 Inicializar texto emergente
	contemplativeMenu.text.textVisible = false;
	contemplativeMenu.text.textStartTime = contemplativeMenu.startTime + MenuConfig::TEXT_DELAY;
	contemplativeMenu.text.textAlpha = 0;
	contemplativeMenu.text.currentOption = 0;
	contemplativeMenu.text.lastTextUpdate = contemplativeMenu.startTime;
	
	// 🌊 Inicializar efectos sutiles
	contemplativeMenu.effects.shimmerOffset = 0;
	contemplativeMenu.effects.lastShimmerTime = contemplativeMenu.startTime;
	contemplativeMenu.effects.darkeningActive = true;
	contemplativeMenu.effects.darkeningLevel = 0.0f;
	
	LogVerbose("Contemplative Menu initialized - Antesala del Infierno ready");
}

void UpdateContemplativeMenu(uint32_t currentTick)
{
	if (!contemplativeMenu.enabled) {
		return;
	}
	
	contemplativeMenu.lastUpdateTime = currentTick;
	
	// 🌫️ Actualizar fondo vivo (muy lento)
	if (currentTick - contemplativeMenu.background.lastFogTime >= MenuConfig::FOG_FRAME_DELAY) {
		contemplativeMenu.background.fogFrame = (contemplativeMenu.background.fogFrame + 1) % 6;
		contemplativeMenu.background.lastFogTime = currentTick;
	}
	
	// 💡 Actualizar pulsación de luz (muy sutil)
	uint32_t pulseTime = currentTick % MenuConfig::PULSE_CYCLE_TIME;
	float pulsePhase = (float)pulseTime / MenuConfig::PULSE_CYCLE_TIME;
	contemplativeMenu.background.pulseBrightness = std::sin(pulsePhase * 2.0f * M_PI) * 0.1f;  // ±10%
	
	// 📜 Actualizar texto emergente (discreto, no smooth)
	if (currentTick >= contemplativeMenu.text.textStartTime && !contemplativeMenu.text.textVisible) {
		contemplativeMenu.text.textVisible = true;
		LogVerbose("🌑 Menu text emerging - Catedral opens");
	}
	
	if (contemplativeMenu.text.textVisible && 
	    currentTick - contemplativeMenu.text.lastTextUpdate >= 100) {  // 100ms entre saltos
		if (contemplativeMenu.text.textAlpha < 255) {
			contemplativeMenu.text.textAlpha = std::min(255u, 
				(uint32_t)contemplativeMenu.text.textAlpha + MenuConfig::TEXT_ALPHA_STEP);
			contemplativeMenu.text.lastTextUpdate = currentTick;
		}
	}
	
	// 🌊 Actualizar shimmer sutil
	if (currentTick - contemplativeMenu.effects.lastShimmerTime >= MenuConfig::SHIMMER_DELAY) {
		float shimmerPhase = (currentTick * 0.001f);
		contemplativeMenu.effects.shimmerOffset = (int)(std::sin(shimmerPhase) * 2.0f);  // ±2 píxeles
		contemplativeMenu.effects.lastShimmerTime = currentTick;
	}
	
	// 🌑 Actualizar oscurecimiento gradual
	uint32_t timeSinceStart = currentTick - contemplativeMenu.startTime;
	if (timeSinceStart < 3000) {  // Primeros 3 segundos
		contemplativeMenu.effects.darkeningLevel = 
			(float)timeSinceStart / 3000.0f * MenuConfig::MAX_DARKENING;
	} else {
		contemplativeMenu.effects.darkeningLevel = MenuConfig::MAX_DARKENING;
	}
}

void RenderContemplativeMenu()
{
	if (!contemplativeMenu.enabled) {
		return;
	}
	
	// 🩸 Render por capas determinísticas (orden fijo)
	DrawMenuLayer(MenuVisualLayer::BACKGROUND);
	DrawMenuLayer(MenuVisualLayer::FOG);
	DrawMenuLayer(MenuVisualLayer::SHADOW);
	DrawMenuLayer(MenuVisualLayer::TEXT);
}

void DrawMenuLayer(MenuVisualLayer layer)
{
	switch (layer) {
	case MenuVisualLayer::BACKGROUND:
		DrawMenuBackground();
		break;
	case MenuVisualLayer::FOG:
		DrawFogOverlay(contemplativeMenu.background.fogFrame, 24);
		break;
	case MenuVisualLayer::SHADOW:
		DrawMenuShadow();
		break;
	case MenuVisualLayer::TEXT:
		DrawEmergingText();
		break;
	}
}

void DrawMenuBackground()
{
	// 🌑 FONDO CONTEMPLATIVO COMPLETAMENTE LIMPIO
	// No renderizar nada - dejar que la imagen de fondo se vea sin interferencias
}

void DrawFogOverlay(int frame, uint8_t alpha)
{
	// 🌫️ NIEBLA DESHABILITADA - Dejar imagen de fondo limpia
	// No renderizar nada para mantener la imagen de fondo sin interferencias
}

void DrawMenuShadow()
{
	// 🌑 Oscurecimiento gradual (Nightmare effect)
	const Surface &out = GlobalBackBuffer();
	
	if (contemplativeMenu.effects.darkeningLevel <= 0.0f) {
		return;
	}
	
	// Aplicar oscurecimiento sutil en área del menú
	int menuX = out.w() / 6;
	int menuY = out.h() / 6;
	int menuW = out.w() * 2 / 3;
	int menuH = out.h() * 2 / 3;
	
	for (int y = menuY; y < menuY + menuH; y += 6) {  // Cada 6 píxeles
		for (int x = menuX; x < menuX + menuW; x += 8) {  // Cada 8 píxeles
			// Patrón de oscurecimiento
			int shadowPattern = (x + y) % 17;
			
			if (shadowPattern < (int)(contemplativeMenu.effects.darkeningLevel * 17)) {
				if (x < out.w() && y < out.h()) {
					// Oscurecer píxel (color más bajo)
					uint8_t shadowColor = 2;
					out.SetPixel({x, y}, shadowColor);
				}
			}
		}
	}
}

void DrawEmergingText()
{
	// 📜 Texto que emerge con alpha discreto
	if (!contemplativeMenu.text.textVisible || contemplativeMenu.text.textAlpha == 0) {
		return;
	}
	
	// TODO: Integrar con sistema de texto real de DevilutionX
	// Por ahora, simular con píxeles
	const Surface &out = GlobalBackBuffer();
	
	int textX = out.w() / 2 - 50;
	int textY = out.h() / 2;
	
	// Simular texto con rectángulos (placeholder)
	uint8_t textColor = std::min(255u, (uint32_t)contemplativeMenu.text.textAlpha);
	textColor = std::max(32u, (uint32_t)textColor);  // Mínimo visible
	
	// "Single Player" simulado
	for (int i = 0; i < 20; i += 2) {
		if (textX + i < out.w() && textY < out.h()) {
			out.SetPixel({textX + i, textY}, textColor);
			out.SetPixel({textX + i, textY + 1}, textColor);
		}
	}
	
	// "Multiplayer" simulado
	textY += 20;
	for (int i = 0; i < 18; i += 2) {
		if (textX + i < out.w() && textY < out.h()) {
			out.SetPixel({textX + i, textY}, textColor);
			out.SetPixel({textX + i, textY + 1}, textColor);
		}
	}
}

void SetContemplativeMenuEnabled(bool enabled)
{
	contemplativeMenu.enabled = enabled;
	LogVerbose("Contemplative Menu: {}", enabled ? "enabled" : "disabled");
}

void TriggerMenuEntry()
{
	// Reiniciar estado para nueva entrada al menú
	uint32_t currentTime = SDL_GetTicks();
	
	contemplativeMenu.startTime = currentTime;
	contemplativeMenu.background.fogSeed = rand() % 1000;  // Nueva variación
	contemplativeMenu.text.textVisible = false;
	contemplativeMenu.text.textAlpha = 0;
	contemplativeMenu.text.textStartTime = currentTime + MenuConfig::TEXT_DELAY;
	contemplativeMenu.effects.darkeningLevel = 0.0f;
	
	LogVerbose("🌑 Menu entry triggered - Catedral awaits");
}

void ResetMenuState()
{
	contemplativeMenu.enabled = false;
	contemplativeMenu.text.textVisible = false;
	contemplativeMenu.text.textAlpha = 0;
	contemplativeMenu.effects.darkeningLevel = 0.0f;
	
	LogVerbose("🌑 Menu state reset");
}

} // namespace devilution