/**
 * @file dynamic_scaling.cpp
 * @brief Nightmare Dynamic Scaling - Implementation
 * 
 * FIXES APLICADOS (Enero 14, 2026):
 * - Consolidado particle y decal budget (eran idénticos)
 * - Agregada función de inicialización del sistema completo
 */

#include "engine/platform/dynamic_scaling.h"
#include "engine/platform/platform.h"
#include "utils/log.hpp"

namespace devilution {

namespace {

// Global state - SIMPLIFIED: Single budget for both particles and decals
ParticleBudget gBudget;
bool gDynamicScalingEnabled = true;

// Constants
constexpr float TARGET_FRAME_TIME = 16.6f;  // 60 FPS
constexpr float STRESS_THRESHOLD = 20.0f;   // 50 FPS
constexpr int STABLE_FRAMES_REQUIRED = 60;  // 1 second @ 60fps
constexpr int UNSTABLE_FRAMES_REQUIRED = 10; // ~166ms
constexpr int BUDGET_STEP_DOWN = 5;         // Reduce by 5% when stressed
constexpr int BUDGET_STEP_UP = 1;           // Increase by 1% when stable

/**
 * Apply platform-specific budget limits
 */
void ApplyPlatformLimits(ParticleBudget& budget)
{
	auto caps = GetPlatformCapabilities();
	
	// Set max based on platform
	budget.max = caps.maxParticles;
	
	// Ensure current doesn't exceed max
	if (budget.current > budget.max) {
		budget.current = budget.max;
	}
	
	// Set min based on platform (50% of max, minimum 50)
	budget.min = std::max(50, budget.max / 2);
}

/**
 * Update budget based on frame time
 */
void UpdateBudget(ParticleBudget& budget, float frameTime)
{
	// Update average frame time (exponential moving average)
	budget.avgFrameTime = budget.avgFrameTime * 0.95f + frameTime * 0.05f;
	
	// Check if frame time is bad
	if (frameTime > STRESS_THRESHOLD) {
		// Performance stress detected
		budget.unstableFrames++;
		budget.stableFrames = 0;
		
		// Reduce budget if consistently bad
		if (budget.unstableFrames >= UNSTABLE_FRAMES_REQUIRED) {
			int newBudget = budget.current - BUDGET_STEP_DOWN;
			if (newBudget >= budget.min) {
				budget.current = newBudget;
				budget.reductions++;
				budget.unstableFrames = 0;
				
				LogVerbose("Dynamic Scaling: Reduced budget to {}%", budget.current);
			}
		}
	} else if (frameTime < TARGET_FRAME_TIME) {
		// Performance is good
		budget.stableFrames++;
		budget.unstableFrames = 0;
		
		// Increase budget if consistently good
		if (budget.stableFrames >= STABLE_FRAMES_REQUIRED) {
			int newBudget = budget.current + BUDGET_STEP_UP;
			if (newBudget <= budget.max) {
				budget.current = newBudget;
				budget.increases++;
				budget.stableFrames = 0;
				
				LogVerbose("Dynamic Scaling: Increased budget to {}%", budget.current);
			}
		}
	} else {
		// Frame time is acceptable, reset counters
		budget.stableFrames = 0;
		budget.unstableFrames = 0;
	}
}

} // anonymous namespace

/**
 * Initialize dynamic scaling system
 */
void InitDynamicScaling()
{
	auto caps = GetPlatformCapabilities();
	
	// Initialize unified budget (particles and decals share same budget)
	gBudget.current = caps.maxParticles;
	gBudget.max = caps.maxParticles;
	gBudget.min = std::max(50, caps.maxParticles / 2);
	gBudget.stableFrames = 0;
	gBudget.unstableFrames = 0;
	gBudget.reductions = 0;
	gBudget.increases = 0;
	gBudget.avgFrameTime = 16.6f;
	
	// Enable by default in release, disable in debug for testing
#ifdef _DEBUG
	gDynamicScalingEnabled = false;
	LogVerbose("Dynamic Scaling: DISABLED (debug build)");
#else
	gDynamicScalingEnabled = true;
	LogVerbose("Dynamic Scaling: ENABLED");
#endif
	
	LogVerbose("Dynamic Scaling initialized:");
	LogVerbose("  Budget: {}% (min: {}, max: {})", 
	          gBudget.current, gBudget.min, gBudget.max);
}

/**
 * Update dynamic scaling based on frame time
 */
void UpdateDynamicScaling(float frameTime)
{
	if (!gDynamicScalingEnabled) {
		return;
	}
	
	// Update unified budget
	UpdateBudget(gBudget, frameTime);
}

/**
 * Get current particle budget
 */
int GetParticleBudget()
{
	return gBudget.current;
}

/**
 * Get current decal budget
 * NOTE: Returns same as particle budget (consolidated)
 */
int GetDecalBudget()
{
	return gBudget.current;
}

/**
 * Check if system is in performance stress
 */
bool IsPerformanceStressed()
{
	return gBudget.avgFrameTime > STRESS_THRESHOLD;
}

/**
 * Get dynamic scaling statistics
 */
const ParticleBudget& GetScalingStats()
{
	return gBudget;
}

/**
 * Reset dynamic scaling to defaults
 */
void ResetDynamicScaling()
{
	auto caps = GetPlatformCapabilities();
	
	gBudget.current = caps.maxParticles;
	gBudget.stableFrames = 0;
	gBudget.unstableFrames = 0;
	gBudget.reductions = 0;
	gBudget.increases = 0;
	gBudget.avgFrameTime = 16.6f;
	
	LogVerbose("Dynamic Scaling reset to defaults");
}

/**
 * Enable/disable dynamic scaling
 */
void SetDynamicScalingEnabled(bool enabled)
{
	gDynamicScalingEnabled = enabled;
	LogVerbose("Dynamic Scaling: {}", enabled ? "ENABLED" : "DISABLED");
}

/**
 * Check if dynamic scaling is enabled
 */
bool IsDynamicScalingEnabled()
{
	return gDynamicScalingEnabled;
}

} // namespace devilution
