/**
 * @file platform.h
 * @brief Nightmare Portability Layer - Platform abstraction and capabilities
 * 
 * PHILOSOPHY:
 * - Data-driven, not hardcoded
 * - O(1) runtime detection
 * - Zero gameplay impact
 * - Fail-soft always
 * 
 * USAGE:
 *   auto caps = GetPlatformCapabilities();
 *   int maxParticles = caps.maxParticles;
 */

#pragma once

#include <string>

namespace devilution {

/**
 * Platform classification for runtime adaptation
 */
enum class PlatformClass {
	Desktop,    // PC, Mac, Linux
	Handheld,   // Steam Deck, ROG Ally, etc.
	Mobile,     // Android, iOS (future)
	Unknown
};

/**
 * Platform capabilities - detected at runtime
 * Used for adaptive performance and UI scaling
 */
struct PlatformCapabilities {
	// Platform identification
	PlatformClass platformClass = PlatformClass::Desktop;
	std::string platformName = "Unknown";
	
	// Performance capabilities
	int maxParticles = 100;        // 100% = full quality
	int maxDecals = 100;           // Blood, scorch marks, etc.
	int audioVoiceQuality = 100;   // Audio quality percentage
	
	// Input capabilities
	bool hasMouse = true;
	bool hasKeyboard = true;
	bool hasTouch = false;
	bool hasGamepad = false;
	
	// Hardware characteristics
	bool lowPowerCPU = false;      // Detected via performance
	bool lowPowerGPU = false;      // Detected via frame time
	bool limitedRAM = false;       // < 4GB
	bool thermalThrottling = false; // Detected at runtime
	
	// UI scaling
	float uiScale = 1.0f;          // 1.0 = normal, 1.5 = large, 2.0 = extra-large
	bool needsLargeClickTargets = false;
	
	// Display
	int screenWidth = 0;
	int screenHeight = 0;
	bool isHighDPI = false;
};

/**
 * Build preset configuration
 * Defines default values per platform
 */
struct BuildPreset {
	const char *name;
	PlatformClass platformClass;
	int particleQuality;    // 50-100%
	int decalQuality;       // 50-100%
	int audioQuality;       // 50-100%
	float uiScale;
	bool mobileSafeMode;
};

// Predefined build presets
namespace Presets {
	constexpr BuildPreset PC = {
		"Nightmare-PC",
		PlatformClass::Desktop,
		100, // particles
		100, // decals
		100, // audio
		1.0f, // UI scale
		false // mobile safe mode
	};
	
	constexpr BuildPreset Handheld = {
		"Nightmare-Handheld",
		PlatformClass::Handheld,
		70,  // particles
		70,  // decals
		80,  // audio
		1.5f, // UI scale
		false // mobile safe mode
	};
	
	constexpr BuildPreset Mobile = {
		"Nightmare-Mobile",
		PlatformClass::Mobile,
		50,  // particles
		50,  // decals
		60,  // audio
		2.0f, // UI scale
		true  // mobile safe mode
	};
}

/**
 * Detect platform capabilities at runtime
 * Called once at startup
 */
PlatformCapabilities DetectPlatformCapabilities();

/**
 * Get current platform capabilities
 * Cached after first detection
 */
const PlatformCapabilities& GetPlatformCapabilities();

/**
 * Force platform class (for testing)
 * Only available in debug builds
 */
void ForcePlatformClass(PlatformClass platformClass);

/**
 * Apply build preset
 * Overrides detected capabilities with preset values
 */
void ApplyBuildPreset(const BuildPreset& preset);

/**
 * Get current build preset name
 */
const char* GetBuildPresetName();

} // namespace devilution
