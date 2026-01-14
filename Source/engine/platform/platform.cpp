/**
 * @file platform.cpp
 * @brief Nightmare Portability Layer - Implementation
 */

#include "engine/platform/platform.h"

#include <SDL.h>
#include "utils/log.hpp"

namespace devilution {

namespace {

// Cached platform capabilities
PlatformCapabilities gPlatformCaps;
bool gCapsInitialized = false;
const BuildPreset *gCurrentPreset = &Presets::PC;

/**
 * Detect if running on handheld device
 * Heuristics: screen size, touch support, battery
 */
bool IsHandheldDevice()
{
	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
		int width = displayMode.w;
		int height = displayMode.h;
		
		// Typical handheld resolutions: 1280x800, 1920x1080, 1920x1200
		if (width <= 1920 && height <= 1200) {
			// Check if has touch support (Steam Deck, etc.)
			if (SDL_GetNumTouchDevices() > 0) {
				return true;
			}
		}
	}
	
	return false;
}

/**
 * Detect if running on mobile device
 */
bool IsMobileDevice()
{
#if defined(__ANDROID__) || defined(__IPHONEOS__)
	return true;
#else
	return false;
#endif
}

/**
 * Detect platform class
 */
PlatformClass DetectPlatformClass()
{
	if (IsMobileDevice()) {
		return PlatformClass::Mobile;
	}
	
	if (IsHandheldDevice()) {
		return PlatformClass::Handheld;
	}
	
	return PlatformClass::Desktop;
}

/**
 * Get platform name string
 */
std::string GetPlatformName()
{
	const char *platform = SDL_GetPlatform();
	if (platform != nullptr) {
		return std::string(platform);
	}
	return "Unknown";
}

/**
 * Detect available RAM
 */
bool HasLimitedRAM()
{
	// SDL doesn't provide RAM info, assume false for now
	// Can be enhanced with platform-specific code
	return false;
}

/**
 * Detect screen resolution and DPI
 */
void DetectDisplayInfo(PlatformCapabilities& caps)
{
	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
		caps.screenWidth = displayMode.w;
		caps.screenHeight = displayMode.h;
		
		// High DPI detection (> 1080p)
		caps.isHighDPI = (displayMode.w > 1920 || displayMode.h > 1080);
	}
}

/**
 * Detect input capabilities
 */
void DetectInputCapabilities(PlatformCapabilities& caps)
{
	// Touch support
	caps.hasTouch = (SDL_GetNumTouchDevices() > 0);
	
	// Gamepad support
	caps.hasGamepad = (SDL_NumJoysticks() > 0);
	
	// Mouse and keyboard (assume true on desktop)
	caps.hasMouse = !caps.hasTouch;
	caps.hasKeyboard = !caps.hasTouch;
}

} // anonymous namespace

/**
 * Detect platform capabilities at runtime
 */
PlatformCapabilities DetectPlatformCapabilities()
{
	PlatformCapabilities caps;
	
	// Detect platform class
	caps.platformClass = DetectPlatformClass();
	caps.platformName = GetPlatformName();
	
	// Detect display info
	DetectDisplayInfo(caps);
	
	// Detect input capabilities
	DetectInputCapabilities(caps);
	
	// Detect hardware characteristics
	caps.limitedRAM = HasLimitedRAM();
	
	// Set defaults based on platform class
	switch (caps.platformClass) {
		case PlatformClass::Desktop:
			caps.maxParticles = 100;
			caps.maxDecals = 100;
			caps.audioVoiceQuality = 100;
			caps.uiScale = 1.0f;
			caps.needsLargeClickTargets = false;
			break;
			
		case PlatformClass::Handheld:
			caps.maxParticles = 70;
			caps.maxDecals = 70;
			caps.audioVoiceQuality = 80;
			caps.uiScale = 1.5f;
			caps.needsLargeClickTargets = true;
			break;
			
		case PlatformClass::Mobile:
			caps.maxParticles = 50;
			caps.maxDecals = 50;
			caps.audioVoiceQuality = 60;
			caps.uiScale = 2.0f;
			caps.needsLargeClickTargets = true;
			break;
			
		case PlatformClass::Unknown:
			// Conservative defaults
			caps.maxParticles = 70;
			caps.maxDecals = 70;
			caps.audioVoiceQuality = 80;
			caps.uiScale = 1.0f;
			caps.needsLargeClickTargets = false;
			break;
	}
	
	LogVerbose("Platform detected: {} ({})", caps.platformName, static_cast<int>(caps.platformClass));
	LogVerbose("  Screen: {}x{}", caps.screenWidth, caps.screenHeight);
	LogVerbose("  Particles: {}%", caps.maxParticles);
	LogVerbose("  UI Scale: {}", caps.uiScale);
	
	return caps;
}

/**
 * Get current platform capabilities (cached)
 */
const PlatformCapabilities& GetPlatformCapabilities()
{
	if (!gCapsInitialized) {
		gPlatformCaps = DetectPlatformCapabilities();
		gCapsInitialized = true;
	}
	
	return gPlatformCaps;
}

/**
 * Force platform class (debug only)
 */
void ForcePlatformClass(PlatformClass platformClass)
{
#ifdef _DEBUG
	gPlatformCaps.platformClass = platformClass;
	
	// Re-apply defaults for forced platform
	switch (platformClass) {
		case PlatformClass::Desktop:
			ApplyBuildPreset(Presets::PC);
			break;
		case PlatformClass::Handheld:
			ApplyBuildPreset(Presets::Handheld);
			break;
		case PlatformClass::Mobile:
			ApplyBuildPreset(Presets::Mobile);
			break;
		default:
			break;
	}
	
	LogVerbose("Platform forced to: {}", static_cast<int>(platformClass));
#endif
}

/**
 * Apply build preset
 */
void ApplyBuildPreset(const BuildPreset& preset)
{
	gCurrentPreset = &preset;
	
	// Override capabilities with preset values
	gPlatformCaps.platformClass = preset.platformClass;
	gPlatformCaps.maxParticles = preset.particleQuality;
	gPlatformCaps.maxDecals = preset.decalQuality;
	gPlatformCaps.audioVoiceQuality = preset.audioQuality;
	gPlatformCaps.uiScale = preset.uiScale;
	
	LogVerbose("Build preset applied: {}", preset.name);
}

/**
 * Get current build preset name
 */
const char* GetBuildPresetName()
{
	return gCurrentPreset->name;
}

} // namespace devilution
