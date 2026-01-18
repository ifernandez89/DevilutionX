/**
 * @file palette.cpp
 *
 * Implementation of functions for handling the engines color palette.
 */
#include "engine/palette.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <span>

#ifdef USE_SDL3
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "abstractions/vfs_wrappers.h"  // 🌐 VFS for WebAssembly compatibility
#include "engine/backbuffer_state.hpp"
#include "engine/demomode.h"
#include "engine/dx.h"
#include "engine/load_file.hpp"
#include "engine/random.hpp"
#include "headless_mode.hpp"
#include "hwcursor.hpp"
#include "options.h"
#include "utils/display.h"
#include "phase4_render_logging.h"
#include "utils/palette_blending.hpp"
#include "visual_feedback.h"
#include "contextual_palette.h"
#include "town_cinematic.h"
#include "utils/sdl_compat.h"
#include "utils/str_cat.hpp"

namespace devilution {

std::array<SDL_Color, 256> logical_palette;
std::array<SDL_Color, 256> system_palette;

namespace {

/** Specifies whether the palette has max brightness. */
bool sgbFadedIn = true;

void LoadBrightness()
{
	int brightnessValue = *GetOptions().Graphics.brightness;
	brightnessValue = std::clamp(brightnessValue, 0, 100);
	GetOptions().Graphics.brightness.SetValue(brightnessValue - brightnessValue % 5);
}

/**
 * @brief Cycle the given range of colors in the palette
 * @param from First color index of the range
 * @param to First color index of the range
 */
void CycleColors(int from, int to)
{
	std::rotate(logical_palette.begin() + from, logical_palette.begin() + from + 1, logical_palette.begin() + to + 1);
	std::rotate(system_palette.begin() + from, system_palette.begin() + from + 1, system_palette.begin() + to + 1);

	for (auto &palette : paletteTransparencyLookup) {
		std::rotate(std::begin(palette) + from, std::begin(palette) + from + 1, std::begin(palette) + to + 1);
	}

	std::rotate(&paletteTransparencyLookup[from][0], &paletteTransparencyLookup[from + 1][0], &paletteTransparencyLookup[to + 1][0]);

#if DEVILUTIONX_PALETTE_TRANSPARENCY_BLACK_16_LUT
	UpdateTransparencyLookupBlack16(from, to);
#endif
}

/**
 * @brief Cycle the given range of colors in the palette in reverse direction
 * @param from First color index of the range
 * @param to Last color index of the range
 */
void CycleColorsReverse(int from, int to)
{
	std::rotate(logical_palette.begin() + from, logical_palette.begin() + to, logical_palette.begin() + to + 1);
	std::rotate(system_palette.begin() + from, system_palette.begin() + to, system_palette.begin() + to + 1);

	for (auto &palette : paletteTransparencyLookup) {
		std::rotate(std::begin(palette) + from, std::begin(palette) + to, std::begin(palette) + to + 1);
	}

	std::rotate(&paletteTransparencyLookup[from][0], &paletteTransparencyLookup[to][0], &paletteTransparencyLookup[to + 1][0]);

#if DEVILUTIONX_PALETTE_TRANSPARENCY_BLACK_16_LUT
	UpdateTransparencyLookupBlack16(from, to);
#endif
}

// When brightness==0, then a==0 (identity mapping)
// When brightness==100, then a==-MaxAdjustment (maximum brightening)
constexpr float CalculateToneMappingParameter(int brightness)
{
	// Maximum adjustment factor (tweak this constant to change the effect strength)
	constexpr float MaxAdjustment = 2.0F;
	return -(brightness / 100.0f) * MaxAdjustment;
}

constexpr uint8_t MapTone(float a, uint8_t color)
{
	const auto x = static_cast<float>(color / 255.0f);
	// Our quadratic tone mapping: f(x) = a*x^2 + (1-a)*x.
	const float y = std::clamp(a * x * x + (1.0f - a) * x, 0.0f, 1.0f);
	return static_cast<uint8_t>(y * 255.0f + 0.5f);
}

void ApplyGlobalBrightnessSingleColor(SDL_Color &dst, const SDL_Color &src)
{
	const float a = CalculateToneMappingParameter(*GetOptions().Graphics.brightness);
	dst.r = MapTone(a, src.r);
	dst.g = MapTone(a, src.g);
	dst.b = MapTone(a, src.b);
}

} // namespace

// 🛡️ PALETTE CORRUPTION FIX: Flag to skip contextual effects during transitions
// This prevents palette corruption when leveltype is in an invalid state
bool g_skipContextualPaletteEffects = false;

void ApplyGlobalBrightness(SDL_Color *dst, const SDL_Color *src)
{
	// Get the brightness slider value (0 = neutral, 100 = max brightening)
	const int brightnessSlider = *GetOptions().Graphics.brightness;

	// Precompute a lookup table for speed.
	const float a = CalculateToneMappingParameter(brightnessSlider);
	uint8_t toneMap[256];
	for (int i = 0; i < 256; i++) {
		toneMap[i] = MapTone(a, i);
	}

	// Apply the lookup table to each color channel in the palette.
	for (int i = 0; i < 256; i++) {
		dst[i].r = toneMap[src[i].r];
		dst[i].g = toneMap[src[i].g];
		dst[i].b = toneMap[src[i].b];
	}
	
	// 🛡️ PALETTE CORRUPTION FIX: Contextual effects DISABLED to prevent corruption
	// The contextual palette effects were causing visual corruption during transitions.
	// Keeping only basic brightness adjustment for stability.
}

void ApplyFadeLevel(unsigned fadeval, SDL_Color *dst, const SDL_Color *src)
{
	for (int i = 0; i < 256; i++) {
		dst[i].r = (fadeval * src[i].r) / 256;
		dst[i].g = (fadeval * src[i].g) / 256;
		dst[i].b = (fadeval * src[i].b) / 256;
	}
}

// Applies a tone mapping curve based on the brightness slider value.
// The brightness value is in the range [0, 100] where 0 is neutral (no change)
// and 100 produces maximum brightening.
void UpdateSystemPalette(std::span<const SDL_Color, 256> src)
{
	// 🚨 SELECTIVE VISUAL EFFECTS CONTROL
	// Control which visual effects are enabled to isolate corruption source
	static bool enableBasicBrightness = true;        // Always safe
	static bool enableContextualPalette = false;     // V2 - Contextual palette system
	static bool enableVisualFeedback = false;        // V3 - Visual feedback effects  
	static bool enableDynamicAdjustment = false;     // Dynamic palette adjustments
	static bool enableContextualEnhancement = false; // Contextual enhancements
	static bool enableTownCinematic = false;         // D3 - Town cinematic effects
	static bool enableAtmosphericDepth = false;      // D1 - Atmospheric depth simulation
	
	// Step 1: Always apply basic brightness (this is safe)
	if (enableBasicBrightness) {
		ApplyGlobalBrightness(system_palette.data(), src.data());
	} else {
		// Copy source palette directly if brightness is disabled
		std::copy(src.begin(), src.end(), system_palette.begin());
	}
	
	// Step 2: 🎨 CONTEXTUAL PALETTE (V2) - Test this first
	if (enableContextualPalette) {
		ApplyContextualPalette(system_palette.data());
	}
	
	// Step 3: 🎮 VISUAL FEEDBACK (V3) - Test this second  
	if (enableVisualFeedback) {
		ApplyVisualFeedbackToPalette(system_palette.data());
	}
	
	// Step 4: Dynamic adjustments - Test this third
	if (enableDynamicAdjustment) {
		ApplyDynamicPaletteAdjustment(system_palette.data());
	}
	
	// Step 5: Contextual enhancements - Test this fourth
	if (enableContextualEnhancement) {
		ApplyContextualPaletteEnhancement(system_palette.data());
	}
	
	// Step 6: 🎬 TOWN CINEMATIC (D3) - Test this fifth
	if (enableTownCinematic) {
		ApplyTownCinematicEffects(system_palette.data());
	}
	
	// Step 7: 🌫️ ATMOSPHERIC DEPTH (D1) - Test this last
	if (enableAtmosphericDepth) {
		ApplyAtmosphericDepthSimulation(system_palette.data());
	}
	
	// Update system and redraw
	SystemPaletteUpdated();
	RedrawEverything();
}

void SystemPaletteUpdated(int first, int ncolor)
{
	if (HeadlessMode)
		return;

	assert(Palette);
	if (!SDLC_SetSurfaceAndPaletteColors(PalSurface, Palette.get(), system_palette.data() + first, first, ncolor)) {
		ErrSdl();
	}
}

void palette_init()
{
	LoadBrightness();
}

void LoadPalette(const char *path)
{
	assert(path != nullptr);
	if (HeadlessMode) {
		PHASE4_LOG("🚫 Headless mode - skipping palette load");
		return;
	}

	LogVerbose("Loading palette from {}", path);
	PHASE4_RENDER_PALETTE(path, 256);
	PHASE4_LOG("🎨 Loading palette: " + std::string(path));
	std::array<Color, 256> palData;
	LoadFileInMemVFS(path, palData);
	for (unsigned i = 0; i < palData.size(); i++) {
		logical_palette[i] = palData[i].toSDL();
	}
	PHASE4_RENDER_SYSTEM_CHECK("Palette System", "Palette loaded successfully");
}

void LoadPaletteAndInitBlending(const char *path)
{
	assert(path != nullptr);
	if (HeadlessMode) return;
	LoadPalette(path);
	if (leveltype == DTYPE_CAVES || leveltype == DTYPE_CRYPT) {
		GenerateBlendedLookupTable(logical_palette.data(), /*skipFrom=*/1, /*skipTo=*/31);
	} else if (leveltype == DTYPE_NEST) {
		GenerateBlendedLookupTable(logical_palette.data(), /*skipFrom=*/1, /*skipTo=*/15);
	} else {
		GenerateBlendedLookupTable(logical_palette.data());
	}
}

void LoadRndLvlPal(dungeon_type l)
{
	if (HeadlessMode)
		return;

	if (l == DTYPE_TOWN) {
		LoadPaletteAndInitBlending("levels\\towndata\\town.pal");
		return;
	}

	if (l == DTYPE_CRYPT) {
		LoadPaletteAndInitBlending("nlevels\\l5data\\l5base.pal");
		return;
	}

	int rv = RandomIntBetween(1, 4);
	char szFileName[27];
	if (l == DTYPE_NEST) {
		if (!*GetOptions().Graphics.alternateNestArt) {
			rv++;
		}
		*BufCopy(szFileName, R"(nlevels\l6data\l6base)", rv, ".pal") = '\0';
	} else {
		char nbuf[3];
		const char *end = BufCopy(nbuf, static_cast<int>(l));
		const std::string_view n = std::string_view(nbuf, end - nbuf);
		*BufCopy(szFileName, "levels\\l", n, "data\\l", n, "_", rv, ".pal") = '\0';
	}
	LoadPaletteAndInitBlending(szFileName);
}

void IncreaseBrightness()
{
	const int brightnessValue = *GetOptions().Graphics.brightness;
	if (brightnessValue < 100) {
		const int newBrightness = std::min(brightnessValue + 5, 100);
		GetOptions().Graphics.brightness.SetValue(newBrightness);
		UpdateSystemPalette(logical_palette);
	}
}

void DecreaseBrightness()
{
	const int brightnessValue = *GetOptions().Graphics.brightness;
	if (brightnessValue > 0) {
		const int newBrightness = std::max(brightnessValue - 5, 0);
		GetOptions().Graphics.brightness.SetValue(newBrightness);
		UpdateSystemPalette(logical_palette);
	}
}

int UpdateBrightness(int brightness)
{
	if (brightness >= 0) {
		GetOptions().Graphics.brightness.SetValue(brightness);
		UpdateSystemPalette(logical_palette);
	}

	return *GetOptions().Graphics.brightness;
}

void BlackPalette()
{
	for (SDL_Color &c : system_palette) {
		c.r = c.g = c.b = 0;
	}
	SystemPaletteUpdated();
}

void PaletteFadeIn(int fr, const std::array<SDL_Color, 256> &srcPalette)
{
	if (HeadlessMode)
		return;
	if (demo::IsRunning())
		fr = 0;

	std::array<SDL_Color, 256> palette;

#ifndef USE_SDL1
	for (SDL_Color &color : palette) {
		color.a = SDL_ALPHA_OPAQUE;
	}
#endif

	ApplyGlobalBrightness(palette.data(), srcPalette.data());

	if (fr > 0) {
		const uint32_t tc = SDL_GetTicks();
		fr *= 3;
		uint32_t prevFadeValue = 255;
		for (uint32_t i = 0; i < 256; i = fr * (SDL_GetTicks() - tc) / 50) {
			if (i == prevFadeValue) {
				SDL_Delay(1);
				continue;
			}
			ApplyFadeLevel(i, system_palette.data(), palette.data());
			SystemPaletteUpdated();

			// We can skip hardware cursor update for fade level 0 (everything is black).
			if (i != 0 && IsHardwareCursor()) {
				ReinitializeHardwareCursor();
			}

			prevFadeValue = i;

			BltFast(nullptr, nullptr);
			RenderPresent();
		}
	}
	system_palette = palette;
	SystemPaletteUpdated();
	RedrawEverything();
	if (IsHardwareCursor()) ReinitializeHardwareCursor();

	if (fr <= 0) {
		BltFast(nullptr, nullptr);
		RenderPresent();
	}

	sgbFadedIn = true;
}

void PaletteFadeOut(int fr, const std::array<SDL_Color, 256> &srcPalette)
{
	if (!sgbFadedIn || HeadlessMode)
		return;
	if (demo::IsRunning())
		fr = 0;

	if (fr > 0) {
		SDL_Color palette[256];
		ApplyGlobalBrightness(palette, srcPalette.data());

		const uint32_t tc = SDL_GetTicks();
		fr *= 3;
		uint32_t prevFadeValue = 0;
		for (uint32_t i = 0; i < 256; i = fr * (SDL_GetTicks() - tc) / 50) {
			if (i == prevFadeValue) {
				SDL_Delay(1);
				continue;
			}
			ApplyFadeLevel(256 - i, system_palette.data(), palette);
			SystemPaletteUpdated();
			prevFadeValue = i;

			BltFast(nullptr, nullptr);
			RenderPresent();
		}
	}

	BlackPalette();
	if (IsHardwareCursor()) ReinitializeHardwareCursor();

	if (fr <= 0) {
		BltFast(nullptr, nullptr);
		RenderPresent();
	}

	sgbFadedIn = false;
}

void palette_update_caves()
{
	CycleColors(1, 31);
	SystemPaletteUpdated(1, 31);
}

/**
 * @brief Cycle the lava every other frame, and glow every frame
 * Lava has 15 colors and the glow 16, so the full animation has 240 frames before it loops
 */
void palette_update_crypt()
{
	static bool delayLava = false;

	if (!delayLava) {
		CycleColorsReverse(1, 15);
		delayLava = false;
	}

	CycleColorsReverse(16, 31);
	SystemPaletteUpdated(1, 31);
	delayLava = !delayLava;
}

/**
 * @brief Cycle the pond waves and bubles colors every 3rd frame
 * Bubles have 8 colors and waves 7, so the full animation has 56 frames before it loops
 */
void palette_update_hive()
{
	static uint8_t delay = 0;

	if (delay != 2) {
		delay++;
		return;
	}

	CycleColorsReverse(1, 8);
	CycleColorsReverse(9, 15);
	SystemPaletteUpdated(1, 15);
	delay = 0;
}

// 🎨 FASE V2 - PALETA CONTEXTUAL AVANZADA 🎨

/**
 * 🎯 V2.7 - DYNAMIC PALETTE ADJUSTMENT
 * Ajuste dinámico de paleta basado en el estado del juego
 */
void ApplyDynamicPaletteAdjustment(SDL_Color *palette)
{
	// 🎯 Ajustes basados en el estado del jugador y el ambiente
	
	// Si el jugador está en peligro (vida baja), aplicar tinte rojo sutil
	if (MyPlayer != nullptr && MyPlayer->_pHitPoints < MyPlayer->_pMaxHP / 4) {
		for (int i = 0; i < 256; i++) {
			// Tinte rojo muy sutil cuando la vida está baja
			palette[i].r = std::min(255, static_cast<int>(palette[i].r * 1.03f));
			palette[i].g = static_cast<uint8_t>(palette[i].g * 0.98f);
		}
	}
	
	// 🎯 Ajustes basados en la hora del día (si aplicable)
	// En town, simular diferentes momentos del día
	if (leveltype == DTYPE_TOWN) {
		// Simular atardecer post-apocalíptico
		for (int i = 0; i < 256; i++) {
			// Tinte anaranjado muy sutil para simular luz de atardecer contaminado
			if (palette[i].r > 50 && palette[i].g > 30) {
				palette[i].r = std::min(255, static_cast<int>(palette[i].r * 1.01f));
				palette[i].g = static_cast<uint8_t>(palette[i].g * 0.995f);
				palette[i].b = static_cast<uint8_t>(palette[i].b * 0.99f);
			}
		}
	}
}

/**
 * 🎯 V2.8 - CONTEXTUAL PALETTE ENHANCEMENT
 * Mejoras contextuales de paleta para situaciones específicas
 */
void ApplyContextualPaletteEnhancement(SDL_Color *palette)
{
	// 🎯 Mejoras específicas por tipo de nivel
	
	if (leveltype == dungeon_type::DTYPE_CATACOMBS) {
		// 🩸 Enhanced Blood Atmosphere para Catacombs
		for (int i = 0; i < 256; i++) {
			// Detectar colores que probablemente sean sangre
			if (palette[i].r > 80 && palette[i].r > palette[i].g * 1.5f && palette[i].r > palette[i].b * 1.5f) {
				// Intensificar el aspecto perturbador de la sangre
				palette[i].r = std::min(255, static_cast<int>(palette[i].r * 1.08f));
				palette[i].g = static_cast<uint8_t>(palette[i].g * 0.85f);
				palette[i].b = static_cast<uint8_t>(palette[i].b * 0.80f);
			}
		}
	} else if (leveltype == dungeon_type::DTYPE_HELL) {
		// 🔥 Atmósfera infernal intensificada
		for (int i = 0; i < 256; i++) {
			// Intensificar colores cálidos (fuego, lava, sangre)
			if (palette[i].r > 100 || (palette[i].r > 60 && palette[i].g > 30 && palette[i].b < 30)) {
				palette[i].r = std::min(255, static_cast<int>(palette[i].r * 1.12f));
				palette[i].g = static_cast<uint8_t>(palette[i].g * 0.90f);
				palette[i].b = static_cast<uint8_t>(palette[i].b * 0.75f);
			}
		}
	} else if (leveltype == dungeon_type::DTYPE_CAVES) {
		// 🪨 Atmósfera mineral opresiva
		for (int i = 0; i < 256; i++) {
			// Desaturar colores para crear sensación opresiva
			uint8_t avg = (palette[i].r + palette[i].g + palette[i].b) / 3;
			palette[i].r = static_cast<uint8_t>(palette[i].r * 0.7f + avg * 0.3f);
			palette[i].g = static_cast<uint8_t>(palette[i].g * 0.7f + avg * 0.3f);
			palette[i].b = static_cast<uint8_t>(palette[i].b * 0.7f + avg * 0.3f);
			
			// Aplicar tinte mineral
			palette[i].r = static_cast<uint8_t>(palette[i].r * 0.95f);
			palette[i].g = static_cast<uint8_t>(palette[i].g * 0.92f);
			palette[i].b = static_cast<uint8_t>(palette[i].b * 0.88f);
		}
	}
}

/**
 * 🎯 V2.9 - ATMOSPHERIC DEPTH SIMULATION
 * Simulación de profundidad atmosférica para mayor inmersión
 */
void ApplyAtmosphericDepthSimulation(SDL_Color *palette)
{
	// 🎯 Simular profundidad atmosférica basada en el tipo de nivel
	
	float depthFactor = 1.0f;
	float hazeFactor = 0.0f;
	
	// Calcular factores de profundidad por tipo de nivel
	if (leveltype == DTYPE_TOWN) {
		depthFactor = 0.98f;   // Superficie, mínima profundidad
		hazeFactor = 0.02f;    // Ligera bruma contaminada
	} else if (leveltype == DTYPE_CATACOMBS) {
		depthFactor = 0.85f;   // Profundidad media, atmósfera densa
		hazeFactor = 0.08f;    // Bruma de muerte y descomposición
	} else if (leveltype == DTYPE_CAVES) {
		depthFactor = 0.88f;   // Profundidad subterránea
		hazeFactor = 0.06f;    // Bruma mineral
	} else if (leveltype == DTYPE_HELL) {
		depthFactor = 0.75f;   // Máxima profundidad
		hazeFactor = 0.12f;    // Bruma infernal intensa
	} else {
		depthFactor = 0.92f;   // Cathedral - profundidad moderada
		hazeFactor = 0.04f;    // Bruma gótica sutil
	}
	
	// Aplicar efectos de profundidad atmosférica
	for (int i = 0; i < 256; i++) {
		// Oscurecer por profundidad
		palette[i].r = static_cast<uint8_t>(palette[i].r * depthFactor);
		palette[i].g = static_cast<uint8_t>(palette[i].g * depthFactor);
		palette[i].b = static_cast<uint8_t>(palette[i].b * depthFactor);
		
		// Aplicar bruma atmosférica (mezcla con gris)
		uint8_t hazeColor = 32; // Gris oscuro para la bruma
		palette[i].r = static_cast<uint8_t>(palette[i].r * (1.0f - hazeFactor) + hazeColor * hazeFactor);
		palette[i].g = static_cast<uint8_t>(palette[i].g * (1.0f - hazeFactor) + hazeColor * hazeFactor);
		palette[i].b = static_cast<uint8_t>(palette[i].b * (1.0f - hazeFactor) + hazeColor * hazeFactor);
	}
}

void SetLogicalPaletteColor(unsigned i, const SDL_Color &color)
{
	logical_palette[i] = color;
	ApplyGlobalBrightnessSingleColor(system_palette[i], logical_palette[i]);
	SystemPaletteUpdated(i, 1);
	UpdateBlendedLookupTableSingleColor(logical_palette.data(), i);
}

} // namespace devilution
