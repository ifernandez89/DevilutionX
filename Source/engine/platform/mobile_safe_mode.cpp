/**
 * @file mobile_safe_mode.cpp
 * @brief Nightmare Mobile Safe Mode - Implementation
 * 
 * FIXES APLICADOS (Enero 14, 2026):
 * - Reemplazado rand() con GenerateRnd() para thread-safety
 * - Agregado reset de throttleLevel cuando performance mejora
 * - Simplificado MobileSafeModeConfig (eliminados campos no usados)
 */

#include "engine/platform/mobile_safe_mode.h"
#include "engine/platform/platform.h"
#include "engine/platform/dynamic_scaling.h"
#include "engine/random.hpp"
#include "utils/log.hpp"

#include <algorithm>

namespace devilution {

namespace {

// Global state
bool gMobileSafeModeActive = false;
MobileSafeModeConfig gMobileConfig;
MobileSafeModeConfig gOriginalConfig;

bool IsDeviceMobile()
{
	auto caps = GetPlatformCapabilities();
	return caps.platformClass == PlatformClass::Mobile;
}

bool IsTouchOnly()
{
	auto caps = GetPlatformCapabilities();
	return caps.hasTouch && !caps.hasMouse && !caps.hasKeyboard;
}

bool IsLowPowerDevice()
{
	auto caps = GetPlatformCapabilities();
	return caps.lowPowerCPU || caps.limitedRAM;
}

} // anonymous namespace

void InitMobileSafeMode()
{
	bool shouldActivate = IsDeviceMobile() || IsTouchOnly() || IsLowPowerDevice();
	
	if (shouldActivate) {
		gMobileSafeModeActive = true;
		// Only set fields that exist in simplified config
		gMobileConfig.particleReduction = 50;
		gMobileConfig.decalReduction = 50;
		gMobileConfig.disableParticles = false;
		gMobileConfig.thermalThrottling = false;
		gMobileConfig.throttleLevel = 0;
		
		gOriginalConfig = gMobileConfig;
		ApplyMobileSafeModeAdjustments();
		
		LogVerbose("Mobile Safe Mode: ACTIVE");
		LogVerbose("  Particle Reduction: {}%", gMobileConfig.particleReduction);
		LogVerbose("  Decal Reduction: {}%", gMobileConfig.decalReduction);
	} else {
		gMobileSafeModeActive = false;
		LogVerbose("Mobile Safe Mode: INACTIVE (desktop detected)");
	}
}

bool IsMobileSafeModeActive()
{
	return gMobileSafeModeActive;
}

void SetMobileSafeModeEnabled(bool enabled)
{
	if (enabled == gMobileSafeModeActive) return;
	
	gMobileSafeModeActive = enabled;
	
	if (enabled) {
		ApplyMobileSafeModeAdjustments();
	} else {
		RevertMobileSafeModeAdjustments();
	}
}

const MobileSafeModeConfig& GetMobileSafeModeConfig()
{
	return gMobileConfig;
}

void UpdateMobileSafeModeConfig(const MobileSafeModeConfig& config)
{
	gMobileConfig = config;
	if (gMobileSafeModeActive) {
		ApplyMobileSafeModeAdjustments();
	}
}

void DetectThermalThrottling()
{
	if (!gMobileSafeModeActive) return;
	
	bool isStressed = IsPerformanceStressed();
	
	if (isStressed) {
		// Performance is bad - increase throttle level
		if (gMobileConfig.throttleLevel < 3) {
			gMobileConfig.throttleLevel++;
			gMobileConfig.thermalThrottling = true;
			
			// Level 1: Reduce particles/decals to 75%
			if (gMobileConfig.throttleLevel >= 1) {
				gMobileConfig.particleReduction = 75;
				gMobileConfig.decalReduction = 75;
			}
			
			// Level 2: Disable particles completely
			if (gMobileConfig.throttleLevel >= 2) {
				gMobileConfig.disableParticles = true;
			}
			
			// Level 3: Maximum reduction (90%)
			if (gMobileConfig.throttleLevel >= 3) {
				gMobileConfig.particleReduction = 90;
				gMobileConfig.decalReduction = 90;
			}
			
			LogVerbose("Thermal Throttling: Level {} (stressed)", gMobileConfig.throttleLevel);
			ApplyMobileSafeModeAdjustments();
		}
	} else if (gMobileConfig.throttleLevel > 0) {
		// FIX: Performance improved - decrease throttle level gradually
		gMobileConfig.throttleLevel--;
		
		// Restore settings based on new level
		if (gMobileConfig.throttleLevel == 0) {
			// Back to normal
			gMobileConfig.particleReduction = 50;
			gMobileConfig.decalReduction = 50;
			gMobileConfig.disableParticles = false;
			gMobileConfig.thermalThrottling = false;
		} else if (gMobileConfig.throttleLevel == 1) {
			// Level 1: 75% reduction
			gMobileConfig.particleReduction = 75;
			gMobileConfig.decalReduction = 75;
			gMobileConfig.disableParticles = false;
		} else if (gMobileConfig.throttleLevel == 2) {
			// Level 2: Particles disabled
			gMobileConfig.particleReduction = 75;
			gMobileConfig.decalReduction = 75;
			gMobileConfig.disableParticles = true;
		}
		
		LogVerbose("Thermal Throttling: Level {} (recovering)", gMobileConfig.throttleLevel);
		ApplyMobileSafeModeAdjustments();
	}
}

void ApplyMobileSafeModeAdjustments()
{
	if (!gMobileSafeModeActive) return;
	LogVerbose("Mobile Safe Mode adjustments applied");
}

void RevertMobileSafeModeAdjustments()
{
	gMobileConfig = gOriginalConfig;
	LogVerbose("Mobile Safe Mode adjustments reverted");
}

int GetMobileParticleCount(int baseCount)
{
	if (!gMobileSafeModeActive) return baseCount;
	
	int reduction = gMobileConfig.particleReduction;
	int scaledCount = (baseCount * (100 - reduction)) / 100;
	return std::max(1, scaledCount);
}

int GetMobileDecalCount(int baseCount)
{
	if (!gMobileSafeModeActive) return baseCount;
	
	int reduction = gMobileConfig.decalReduction;
	int scaledCount = (baseCount * (100 - reduction)) / 100;
	return std::max(1, scaledCount);
}

bool ShouldSpawnParticleOnMobile()
{
	if (!gMobileSafeModeActive) return true;
	if (gMobileConfig.disableParticles) return false;
	
	int reduction = gMobileConfig.particleReduction;
	int chance = 100 - reduction;
	// FIX: Use DevilutionX's thread-safe random instead of rand()
	return GenerateRnd(100) < chance;
}

bool ShouldSpawnDecalOnMobile()
{
	if (!gMobileSafeModeActive) return true;
	
	int reduction = gMobileConfig.decalReduction;
	int chance = 100 - reduction;
	// FIX: Use DevilutionX's thread-safe random instead of rand()
	return GenerateRnd(100) < chance;
}

} // namespace devilution
