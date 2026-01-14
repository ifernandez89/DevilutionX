/**
 * @file dynamic_scaling.h
 * @brief Nightmare Dynamic Scaling - Adaptive performance scaling
 * 
 * PHILOSOPHY:
 * - Heurística ultra-simple (O(1))
 * - Invisible para el jugador
 * - Solo afecta visual, nunca gameplay
 * - Mantiene 60 FPS en cualquier hardware
 * 
 * USAGE:
 *   UpdateDynamicScaling(frameTime);
 *   int budget = GetParticleBudget();
 */

#pragma once

namespace devilution {

/**
 * Particle budget controller
 * Adapts particle count based on frame time
 */
struct ParticleBudget {
	int current = 100;      // Current budget (50-100%)
	int min = 50;           // Minimum budget (never go below)
	int max = 100;          // Maximum budget (platform dependent)
	int stableFrames = 0;   // Frames with good performance
	int unstableFrames = 0; // Frames with bad performance
	
	// Statistics
	int reductions = 0;     // Times budget was reduced
	int increases = 0;      // Times budget was increased
	float avgFrameTime = 16.6f; // Average frame time (ms)
};

/**
 * Initialize dynamic scaling system
 * Called once at startup after platform detection
 */
void InitDynamicScaling();

/**
 * Update dynamic scaling based on frame time
 * Called every frame
 * 
 * @param frameTime Frame time in milliseconds
 */
void UpdateDynamicScaling(float frameTime);

/**
 * Get current particle budget (50-100%)
 * Use this to scale particle spawning
 */
int GetParticleBudget();

/**
 * Get current decal budget (50-100%)
 * Use this to scale decal spawning
 */
int GetDecalBudget();

/**
 * Check if system is in performance stress
 * Returns true if frame time is consistently high
 */
bool IsPerformanceStressed();

/**
 * Get dynamic scaling statistics (debug)
 */
const ParticleBudget& GetScalingStats();

/**
 * Reset dynamic scaling to defaults
 * Useful for testing
 */
void ResetDynamicScaling();

/**
 * Enable/disable dynamic scaling
 * Disabled by default in debug builds for testing
 */
void SetDynamicScalingEnabled(bool enabled);

/**
 * Check if dynamic scaling is enabled
 */
bool IsDynamicScalingEnabled();

} // namespace devilution
