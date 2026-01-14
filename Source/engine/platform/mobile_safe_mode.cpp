/**
 * @file mobile_safe_mode.cpp
 * @brief Nightmare Mobile Safe Mode - Implementation
 */

#include "engine/platform/mobile_safe_mode.h"
#include "engine/platform/platform.h"
#include "engine/platform/dynamic_scaling.h"
#include "utils/log.hpp"

#include <cstdlib>
#include <ctime>
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
		gMobileConfig.particleReduction = 50;
		gMobileConfig.decalReduction = 50;
		gMobileConfig.shadowQuality = 1;
		gMobileConfig.uiScale = 2.0f;
		gMobileConfig.clickTargetSize = 64;
		gMobileConfig.uiContrast = 1.5f;
		gMobileConfig.simplifyLighting = true;
		gMobileConfig.reduceShadows = true;
		gMobileConfig.disableParticles = false;
		gMobileConfig.reduceFPS = false;
		gMobileConfig.targetFPS = 60;
		gMobileConfig.thermalThrottling = false;
		gMobileConfig.throttleLevel = 0;
		
		gOriginalConfig = gMobileConfig;
		ApplyMobileSafeModeAdjustments();
		
		LogVerbose("Mobile Safe Mode: ACTIVE");
	} else {
		gMobileSafeModeActive = false;
		LogVerbose("Mobile Safe Mode: INACTIVE");
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
	
	if (isStressed && !gMobileConfig.thermalThrottling) {
		gMobileConfig.thermalThrottling = true;
		gMobileConfig.throttleLevel++;
		
		if (gMobileConfig.throttleLevel >= 1) {
			gMobileConfig.particleReduction = 75;
			gMobileConfig.decalReduction = 75;
		}
		
		if (gMobileConfig.throttleLevel >= 2) {
			gMobileConfig.shadowQuality = 0;
			gMobileConfig.disableParticles = true;
		}
		
		if (gMobileConfig.throttleLevel >= 3) {
			gMobileConfig.reduceFPS = true;
			gMobileConfig.targetFPS = 30;
		}
		
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
	return (rand() % 100) < chance;
}

bool ShouldSpawnDecalOnMobile()
{
	if (!gMobileSafeModeActive) return true;
	
	int reduction = gMobileConfig.decalReduction;
	int chance = 100 - reduction;
	return (rand() % 100) < chance;
}

} // namespace devilution
