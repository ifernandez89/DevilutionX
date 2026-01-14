/**
 * @file diagnostic_mode.h
 * @brief Nightmare Diagnostic Mode - Observability without interference
 * 
 * PHILOSOPHY:
 * - 0 impacto cuando está apagado
 * - Solo en dev builds
 * - Overlays modulares
 * - Draw-only, sin side-effects
 * 
 * USAGE:
 *   ToggleDiagnosticMode(); // Ctrl+F12
 *   DrawDiagnosticOverlays();
 */

#pragma once

#include <string>

namespace devilution {

/**
 * Diagnostic overlay types
 */
enum class DiagnosticOverlay {
	None = 0,
	SpawnPressure = 1 << 0,    // Heatmap de presión de spawns
	EliteRolls = 1 << 1,       // Tooltip de elite monsters
	AIState = 1 << 2,          // Estado de IA de monstruos
	LightingCost = 1 << 3,     // Costo de iluminación por tile
	Performance = 1 << 4,      // FPS, frame time, budgets
	All = 0xFFFF
};

/**
 * Diagnostic mode state
 */
struct DiagnosticState {
	bool enabled = false;
	int activeOverlays = 0;  // Bitmask de DiagnosticOverlay
	
	// Statistics
	int framesActive = 0;
	int overlayDrawCalls = 0;
};

/**
 * Initialize diagnostic mode
 * Called once at startup
 */
void InitDiagnosticMode();

/**
 * Toggle diagnostic mode on/off
 * Ctrl+F12 in dev builds
 */
void ToggleDiagnosticMode();

/**
 * Check if diagnostic mode is enabled
 */
bool IsDiagnosticModeEnabled();

/**
 * Enable/disable specific overlay
 */
void SetOverlayEnabled(DiagnosticOverlay overlay, bool enabled);

/**
 * Check if specific overlay is enabled
 */
bool IsOverlayEnabled(DiagnosticOverlay overlay);

/**
 * Toggle specific overlay
 */
void ToggleOverlay(DiagnosticOverlay overlay);

/**
 * Draw all enabled diagnostic overlays
 * Called every frame when diagnostic mode is active
 */
void DrawDiagnosticOverlays();

/**
 * Draw spawn pressure heatmap
 * Shows monster spawn density per tile
 */
void DrawSpawnPressureOverlay();

/**
 * Draw elite monster tooltips
 * Shows elite stats when hovering
 */
void DrawEliteRollsOverlay();

/**
 * Draw AI state visualization
 * Shows monster AI state with colors
 */
void DrawAIStateOverlay();

/**
 * Draw lighting cost overlay
 * Shows lighting cost per tile
 */
void DrawLightingCostOverlay();

/**
 * Draw performance overlay
 * Shows FPS, frame time, budgets
 */
void DrawPerformanceOverlay();

/**
 * Get diagnostic state (for debugging)
 */
const DiagnosticState& GetDiagnosticState();

/**
 * Reset diagnostic statistics
 */
void ResetDiagnosticStats();

} // namespace devilution
