// 🚨 EMERGENCY VISUAL DISABLE - TEMPORARY FIX
// This file contains emergency fixes to disable visual systems that might be causing corruption

// Add this to the beginning of UpdateSystemPalette in Source/engine/palette.cpp:

/*
void UpdateSystemPalette(std::span<const SDL_Color, 256> src)
{
	// 🚨 EMERGENCY: Disable all visual effects temporarily
	// Apply only basic brightness, skip all our custom systems
	
	ApplyGlobalBrightness(system_palette.data(), src.data());
	
	// Skip all custom systems temporarily:
	// - ApplyContextualPalette(system_palette.data());
	// - ApplyVisualFeedbackToPalette(system_palette.data());
	// - ApplyDynamicPaletteAdjustment(system_palette.data());
	// - ApplyContextualPaletteEnhancement(system_palette.data());
	// - ApplyTownCinematicEffects(system_palette.data());
	// - ApplyAtmosphericDepthSimulation(system_palette.data());
	
	SystemPaletteUpdated();
	RedrawEverything();
}
*/

// Alternative: Add emergency disable flags to each system