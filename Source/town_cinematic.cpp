#include "town_cinematic.h"
#include "utils/log.hpp"
#include "engine/palette.h"
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

static TownCinematicState g_townCinematic;

void InitTownCinematic() {
	g_townCinematic = TownCinematicState{};
	g_townCinematic.systemEnabled = true;
	g_townCinematic.currentAtmosphere = TownAtmosphere::AFTERNOON;
	g_townCinematic.atmosphereTimer = SDL_GetTicks();
	
	LogVerbose("🏰 FASE D3.1 - Town Cinematográfica initialized");
}

void UpdateTownCinematic() {
	if (!g_townCinematic.systemEnabled || !g_townCinematic.isInTown) {
		return;
	}
	
	// Update fade system
	if (g_townCinematic.fadeActive) {
		uint32_t currentTime = SDL_GetTicks();
		uint32_t elapsed = currentTime - g_townCinematic.fadeStartTime;
		
		if (elapsed >= g_townCinematic.fadeDuration) {
			g_townCinematic.fadeActive = false;
			g_townCinematic.fadeProgress = g_townCinematic.fadeIn ? 1.0f : 0.0f;
		} else {
			float progress = static_cast<float>(elapsed) / g_townCinematic.fadeDuration;
			
			// Smooth ease-in-out curve
			progress = progress * progress * (3.0f - 2.0f * progress);
			
			g_townCinematic.fadeProgress = g_townCinematic.fadeIn ? progress : (1.0f - progress);
		}
	}
	
	// Update atmosphere cycling
	UpdateTownAtmosphere();
}

void OnEnterTown() {
	if (!g_townCinematic.systemEnabled) return;
	
	g_townCinematic.isInTown = true;
	StartTownFade(true, 2000); // 2 second fade in
	
	LogVerbose("🏰 Town Cinematográfica: Entering Town with fade-in");
}

void OnExitTown() {
	if (!g_townCinematic.systemEnabled) return;
	
	g_townCinematic.isInTown = false;
	// Note: Exit fade handled by level transition system
	
	LogVerbose("🏰 Town Cinematográfica: Exiting Town");
}

void StartTownFade(bool fadeIn, uint32_t duration) {
	if (!g_townCinematic.systemEnabled) return;
	
	g_townCinematic.fadeActive = true;
	g_townCinematic.fadeIn = fadeIn;
	g_townCinematic.fadeStartTime = SDL_GetTicks();
	g_townCinematic.fadeDuration = duration;
	g_townCinematic.fadeProgress = fadeIn ? 0.0f : 1.0f;
}

bool IsTownFadeActive() {
	return g_townCinematic.fadeActive;
}

float GetTownFadeAlpha() {
	if (!g_townCinematic.fadeActive) {
		return g_townCinematic.isInTown ? 1.0f : 0.0f;
	}
	return g_townCinematic.fadeProgress;
}

void UpdateTownAtmosphere() {
	if (!g_townCinematic.systemEnabled || !g_townCinematic.isInTown) return;
	
	uint32_t currentTime = SDL_GetTicks();
	if (currentTime - g_townCinematic.atmosphereTimer >= g_townCinematic.atmosphereInterval) {
		// Cycle to next atmosphere
		int current = static_cast<int>(g_townCinematic.currentAtmosphere);
		current = (current + 1) % 4;
		g_townCinematic.currentAtmosphere = static_cast<TownAtmosphere>(current);
		g_townCinematic.atmosphereTimer = currentTime;
		
		LogVerbose("🏰 Town atmosphere changed to: {}", current);
	}
}

TownAtmosphere GetCurrentTownAtmosphere() {
	return g_townCinematic.currentAtmosphere;
}

void ApplyTownAtmospherePalette(uint8_t* palette) {
	if (!g_townCinematic.systemEnabled || !g_townCinematic.isInTown || !palette) {
		return;
	}
	
	// Apply atmosphere-specific color transformations
	switch (g_townCinematic.currentAtmosphere) {
		case TownAtmosphere::MORNING:
			// Bright, slightly blue-tinted
			for (int i = 0; i < 256 * 3; i += 3) {
				palette[i] = std::min(255, static_cast<int>(palette[i] * 1.1f));     // R
				palette[i+1] = std::min(255, static_cast<int>(palette[i+1] * 1.05f)); // G
				palette[i+2] = std::min(255, static_cast<int>(palette[i+2] * 1.15f)); // B
			}
			break;
			
		case TownAtmosphere::AFTERNOON:
			// Standard, no modification
			break;
			
		case TownAtmosphere::EVENING:
			// Warm, golden tint
			for (int i = 0; i < 256 * 3; i += 3) {
				palette[i] = std::min(255, static_cast<int>(palette[i] * 1.2f));     // R
				palette[i+1] = std::min(255, static_cast<int>(palette[i+1] * 1.1f)); // G
				palette[i+2] = static_cast<int>(palette[i+2] * 0.8f);               // B
			}
			break;
			
		case TownAtmosphere::NIGHT:
			// Dark, mysterious
			for (int i = 0; i < 256 * 3; i += 3) {
				palette[i] = static_cast<int>(palette[i] * 0.7f);     // R
				palette[i+1] = static_cast<int>(palette[i+1] * 0.7f); // G
				palette[i+2] = static_cast<int>(palette[i+2] * 0.9f); // B
			}
			break;
	}
	
	// Apply fade effect if active
	if (g_townCinematic.fadeActive) {
		float alpha = GetTownFadeAlpha();
		for (int i = 0; i < 256 * 3; i++) {
			palette[i] = static_cast<uint8_t>(palette[i] * alpha);
		}
	}
}

// 🎬 FASE D3.1 - TOWN CINEMATOGRÁFICA EFFECTS
// Integration function for palette pipeline
void ApplyTownCinematicEffects(SDL_Color* palette) {
	if (!g_townCinematic.systemEnabled || !g_townCinematic.isInTown || !palette) {
		return;
	}
	
	// Apply atmosphere-specific color transformations
	switch (g_townCinematic.currentAtmosphere) {
		case TownAtmosphere::MORNING:
			// Bright, slightly blue-tinted
			for (int i = 0; i < 256; i++) {
				palette[i].r = std::min(255, static_cast<int>(palette[i].r * 1.1f));
				palette[i].g = std::min(255, static_cast<int>(palette[i].g * 1.05f));
				palette[i].b = std::min(255, static_cast<int>(palette[i].b * 1.15f));
			}
			break;
			
		case TownAtmosphere::AFTERNOON:
			// Standard, no modification
			break;
			
		case TownAtmosphere::EVENING:
			// Warm, golden tint
			for (int i = 0; i < 256; i++) {
				palette[i].r = std::min(255, static_cast<int>(palette[i].r * 1.2f));
				palette[i].g = std::min(255, static_cast<int>(palette[i].g * 1.1f));
				palette[i].b = static_cast<uint8_t>(palette[i].b * 0.8f);
			}
			break;
			
		case TownAtmosphere::NIGHT:
			// Dark, mysterious
			for (int i = 0; i < 256; i++) {
				palette[i].r = static_cast<uint8_t>(palette[i].r * 0.7f);
				palette[i].g = static_cast<uint8_t>(palette[i].g * 0.7f);
				palette[i].b = static_cast<uint8_t>(palette[i].b * 0.9f);
			}
			break;
	}
	
	// Apply fade effect if active
	if (g_townCinematic.fadeActive) {
		float alpha = GetTownFadeAlpha();
		for (int i = 0; i < 256; i++) {
			palette[i].r = static_cast<uint8_t>(palette[i].r * alpha);
			palette[i].g = static_cast<uint8_t>(palette[i].g * alpha);
			palette[i].b = static_cast<uint8_t>(palette[i].b * alpha);
		}
	}
}

bool IsTownCinematicEnabled() {
	return g_townCinematic.systemEnabled;
}

void SetTownCinematicEnabled(bool enabled) {
	g_townCinematic.systemEnabled = enabled;
	if (!enabled) {
		g_townCinematic.fadeActive = false;
		g_townCinematic.isInTown = false;
	}
}

} // namespace devilution