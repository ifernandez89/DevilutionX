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
 * FIXES APLICADOS (Enero 14, 2026):
 * - Simplificado MobileSafeModeConfig (eliminados campos no usados)
 * - Agregado throttle level recovery
 * 
 * USAGE:
 *   InitMobileSafeMode();
 *   if (IsMobileSafeModeActive()) { ... }
 */

#pragma once

namespace devilution {

/**
 * Mobile safe mode configuration
 * SIMPLIFIED: Only fields that are actually used
 */
struct MobileSafeModeConfig {
	// Quality settings (actively used)
	int particleReduction = 50;     // Reduce particles by 50%
	int decalReduction = 50;        // Reduce decals by 50%
	bool disableParticles = false;  // Completely disable particles (extreme)
	
	// Thermal management (actively used)
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
