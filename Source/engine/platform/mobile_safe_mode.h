/**
 * @file mobile_safe_mode.h
 * @brief Nightmare Mobile Safe Mode - Automatic quality reduction for mobile
 * 
 * PHILOSOPHY:
 * - Activación automática en mobile
 * - Reduce calidad agresivamente
 * - Maximiza battery life
 * - Todo reversible
 * 
 * USAGE:
 *   InitMobileSafeMode();
 *   if (IsMobileSafeModeActive()) { ... }
 */

#pragma once

namespace devilution {

/**
 * Mobile safe mode configuration
 */
struct MobileSafeModeConfig {
	// Quality settings
	int particleReduction = 50;     // Reduce particles by 50%
	int decalReduction = 50;        // Reduce decals by 50%
	int shadowQuality = 0;          // 0 = disabled, 1 = low, 2 = medium, 3 = high
	
	// UI adjustments
	float uiScale = 2.0f;           // 2x UI scale
	int clickTargetSize = 64;       // Larger click targets (pixels)
	float uiContrast = 1.5f;        // Increased contrast
	
	// Performance settings
	bool simplifyLighting = true;   // Simplify lighting calculations
	bool reduceShadows = true;      // Reduce shadow quality
	bool disableParticles = false;  // Completely disable particles (extreme)
	
	// Battery optimization
	bool reduceFPS = false;         // Reduce to 30 FPS (extreme)
	int targetFPS = 60;             // Target FPS (30 or 60)
	
	// Thermal management
	bool thermalThrottling = false; // Detected thermal throttling
	int throttleLevel = 0;          // 0-3, higher = more aggressive
};

/**
 * Initialize mobile safe mode
 * Called once at startup after platform detection
 */
void InitMobileSafeMode();

/**
 * Check if mobile safe mode is active
 */
bool IsMobileSafeModeActive();

/**
 * Enable/disable mobile safe mode manually
 * Normally auto-detected
 */
void SetMobileSafeModeEnabled(bool enabled);

/**
 * Get current mobile safe mode configuration
 */
const MobileSafeModeConfig& GetMobileSafeModeConfig();

/**
 * Update mobile safe mode configuration
 * Allows runtime adjustments
 */
void UpdateMobileSafeModeConfig(const MobileSafeModeConfig& config);

/**
 * Detect thermal throttling
 * Called periodically to check device temperature
 */
void DetectThermalThrottling();

/**
 * Apply mobile safe mode adjustments
 * Called when mode is activated
 */
void ApplyMobileSafeModeAdjustments();

/**
 * Revert mobile safe mode adjustments
 * Called when mode is deactivated
 */
void RevertMobileSafeModeAdjustments();

/**
 * Get recommended particle count for mobile
 */
int GetMobileParticleCount(int baseCount);

/**
 * Get recommended decal count for mobile
 */
int GetMobileDecalCount(int baseCount);

/**
 * Check if should spawn particle on mobile
 */
bool ShouldSpawnParticleOnMobile();

/**
 * Check if should spawn decal on mobile
 */
bool ShouldSpawnDecalOnMobile();

} // namespace devilution
