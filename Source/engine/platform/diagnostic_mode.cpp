/**
 * @file diagnostic_mode.cpp
 * @brief Nightmare Diagnostic Mode - Implementation
 */

#include "engine/platform/diagnostic_mode.h"
#include "engine/platform/platform.h"
#include "engine/platform/dynamic_scaling.h"
#include "utils/log.hpp"

#ifdef _DEBUG
#include "engine/render/text_render.hpp"
#include "engine/dx.h"
#include "monster.h"
#include "player.h"
#include "lighting.h"
#endif

namespace devilution {

namespace {

// Global state
DiagnosticState gDiagnosticState;

#ifdef _DEBUG
/**
 * Draw text at screen position
 */
void DrawDiagnosticText(int x, int y, const char* text, uint8_t color = 0)
{
	// Simple text drawing - implementation depends on your text rendering system
	// This is a placeholder
	LogVerbose("Diagnostic: {} at ({}, {})", text, x, y);
}

/**
 * Draw colored rectangle (for heatmaps)
 */
void DrawDiagnosticRect(int x, int y, int w, int h, uint8_t color)
{
	// Simple rect drawing - implementation depends on your rendering system
	// This is a placeholder
}
#endif

} // anonymous namespace

/**
 * Initialize diagnostic mode
 */
void InitDiagnosticMode()
{
#ifdef _DEBUG
	gDiagnosticState.enabled = false;
	gDiagnosticState.activeOverlays = 0;
	gDiagnosticState.framesActive = 0;
	gDiagnosticState.overlayDrawCalls = 0;
	
	LogVerbose("Diagnostic Mode initialized (DEBUG BUILD)");
	LogVerbose("  Toggle: Ctrl+F12");
	LogVerbose("  Overlays: SpawnPressure, EliteRolls, AIState, LightingCost, Performance");
#else
	LogVerbose("Diagnostic Mode: DISABLED (release build)");
#endif
}

/**
 * Toggle diagnostic mode on/off
 */
void ToggleDiagnosticMode()
{
#ifdef _DEBUG
	gDiagnosticState.enabled = !gDiagnosticState.enabled;
	
	if (gDiagnosticState.enabled) {
		// Enable all overlays by default
		gDiagnosticState.activeOverlays = static_cast<int>(DiagnosticOverlay::All);
		LogVerbose("Diagnostic Mode: ENABLED");
	} else {
		gDiagnosticState.framesActive = 0;
		LogVerbose("Diagnostic Mode: DISABLED");
	}
#endif
}

/**
 * Check if diagnostic mode is enabled
 */
bool IsDiagnosticModeEnabled()
{
#ifdef _DEBUG
	return gDiagnosticState.enabled;
#else
	return false;
#endif
}

/**
 * Enable/disable specific overlay
 */
void SetOverlayEnabled(DiagnosticOverlay overlay, bool enabled)
{
#ifdef _DEBUG
	if (enabled) {
		gDiagnosticState.activeOverlays |= static_cast<int>(overlay);
	} else {
		gDiagnosticState.activeOverlays &= ~static_cast<int>(overlay);
	}
#endif
}

/**
 * Check if specific overlay is enabled
 */
bool IsOverlayEnabled(DiagnosticOverlay overlay)
{
#ifdef _DEBUG
	return (gDiagnosticState.activeOverlays & static_cast<int>(overlay)) != 0;
#else
	return false;
#endif
}

/**
 * Toggle specific overlay
 */
void ToggleOverlay(DiagnosticOverlay overlay)
{
#ifdef _DEBUG
	bool currentState = IsOverlayEnabled(overlay);
	SetOverlayEnabled(overlay, !currentState);
	
	LogVerbose("Overlay {} {}", static_cast<int>(overlay), !currentState ? "ENABLED" : "DISABLED");
#endif
}

/**
 * Draw all enabled diagnostic overlays
 */
void DrawDiagnosticOverlays()
{
#ifdef _DEBUG
	if (!gDiagnosticState.enabled) {
		return;
	}
	
	gDiagnosticState.framesActive++;
	
	// Draw overlays in order
	if (IsOverlayEnabled(DiagnosticOverlay::SpawnPressure)) {
		DrawSpawnPressureOverlay();
		gDiagnosticState.overlayDrawCalls++;
	}
	
	if (IsOverlayEnabled(DiagnosticOverlay::EliteRolls)) {
		DrawEliteRollsOverlay();
		gDiagnosticState.overlayDrawCalls++;
	}
	
	if (IsOverlayEnabled(DiagnosticOverlay::AIState)) {
		DrawAIStateOverlay();
		gDiagnosticState.overlayDrawCalls++;
	}
	
	if (IsOverlayEnabled(DiagnosticOverlay::LightingCost)) {
		DrawLightingCostOverlay();
		gDiagnosticState.overlayDrawCalls++;
	}
	
	if (IsOverlayEnabled(DiagnosticOverlay::Performance)) {
		DrawPerformanceOverlay();
		gDiagnosticState.overlayDrawCalls++;
	}
#endif
}

/**
 * Draw spawn pressure heatmap
 */
void DrawSpawnPressureOverlay()
{
#ifdef _DEBUG
	// Placeholder: Draw heatmap of monster density
	// Green = low density, Yellow = medium, Red = high
	
	// This would iterate through tiles and draw colored overlays
	// based on monster count in area
	
	DrawDiagnosticText(10, 10, "Spawn Pressure: Active", 2);
#endif
}

/**
 * Draw elite monster tooltips
 */
void DrawEliteRollsOverlay()
{
#ifdef _DEBUG
	// Placeholder: Draw tooltip when hovering over elite monsters
	// Shows: Elite +32% HP +27% DMG Seed: 0xA91F
	
	DrawDiagnosticText(10, 30, "Elite Rolls: Active", 2);
#endif
}

/**
 * Draw AI state visualization
 */
void DrawAIStateOverlay()
{
#ifdef _DEBUG
	// Placeholder: Draw colored outlines around monsters
	// Green = idle, Yellow = searching, Red = pursuing
	
	DrawDiagnosticText(10, 50, "AI State: Active", 2);
#endif
}

/**
 * Draw lighting cost overlay
 */
void DrawLightingCostOverlay()
{
#ifdef _DEBUG
	// Placeholder: Draw colored overlay per tile
	// Green = cheap, Yellow = medium, Red = expensive
	
	DrawDiagnosticText(10, 70, "Lighting Cost: Active", 2);
#endif
}

/**
 * Draw performance overlay
 */
void DrawPerformanceOverlay()
{
#ifdef _DEBUG
	// Draw performance stats
	auto caps = GetPlatformCapabilities();
	auto scalingStats = GetScalingStats();
	
	char buffer[256];
	
	// Platform info
	snprintf(buffer, sizeof(buffer), "Platform: %s", caps.platformName.c_str());
	DrawDiagnosticText(10, 100, buffer, 1);
	
	// Particle budget
	snprintf(buffer, sizeof(buffer), "Particle Budget: %d%%", scalingStats.current);
	DrawDiagnosticText(10, 120, buffer, 1);
	
	// Frame time
	snprintf(buffer, sizeof(buffer), "Avg Frame Time: %.2fms", scalingStats.avgFrameTime);
	DrawDiagnosticText(10, 140, buffer, 1);
	
	// Budget adjustments
	snprintf(buffer, sizeof(buffer), "Reductions: %d | Increases: %d", 
	         scalingStats.reductions, scalingStats.increases);
	DrawDiagnosticText(10, 160, buffer, 1);
	
	// Performance state
	const char* state = IsPerformanceStressed() ? "STRESSED" : "GOOD";
	snprintf(buffer, sizeof(buffer), "Performance: %s", state);
	DrawDiagnosticText(10, 180, buffer, IsPerformanceStressed() ? 4 : 2);
#endif
}

/**
 * Get diagnostic state
 */
const DiagnosticState& GetDiagnosticState()
{
	return gDiagnosticState;
}

/**
 * Reset diagnostic statistics
 */
void ResetDiagnosticStats()
{
#ifdef _DEBUG
	gDiagnosticState.framesActive = 0;
	gDiagnosticState.overlayDrawCalls = 0;
	
	LogVerbose("Diagnostic stats reset");
#endif
}

} // namespace devilution
