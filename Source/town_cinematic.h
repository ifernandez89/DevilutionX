#pragma once

#include <cstdint>

#ifdef USE_SDL3
#include <SDL3/SDL_pixels.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// 🏰 FASE D3.1 - Town Cinematográfica System
// Provides atmospheric fade effects and dynamic palettes for Town

enum class TownAtmosphere : uint8_t {
	MORNING = 0,    // Bright, welcoming
	AFTERNOON = 1,  // Standard, neutral
	EVENING = 2,    // Warm, golden
	NIGHT = 3       // Dark, mysterious
};

struct TownCinematicState {
	// Fade system
	bool fadeActive = false;
	bool fadeIn = true;
	float fadeProgress = 0.0f;
	uint32_t fadeStartTime = 0;
	uint32_t fadeDuration = 1500; // 1.5 seconds
	
	// Atmosphere cycling
	TownAtmosphere currentAtmosphere = TownAtmosphere::AFTERNOON;
	uint32_t atmosphereTimer = 0;
	uint32_t atmosphereInterval = 45000; // 45 seconds per cycle
	
	// Integration flags
	bool isInTown = false;
	bool systemEnabled = true;
};

// Core API
void InitTownCinematic();
void UpdateTownCinematic();
void OnEnterTown();
void OnExitTown();

// Fade system
void StartTownFade(bool fadeIn, uint32_t duration = 1500);
bool IsTownFadeActive();
float GetTownFadeAlpha();

// Atmosphere system
void UpdateTownAtmosphere();
TownAtmosphere GetCurrentTownAtmosphere();
void ApplyTownAtmospherePalette(uint8_t* palette);
void ApplyTownCinematicEffects(SDL_Color* palette);

// Utility
bool IsTownCinematicEnabled();
void SetTownCinematicEnabled(bool enabled);

} // namespace devilution