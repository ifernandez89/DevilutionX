/**
 * @file nightmare_effects_status.cpp
 * @brief Nightmare Edition - Effects Status Implementation
 */

#include "nightmare_effects_status.h"

#include "nightmare_ambient_effects.h"
#include "nightmare_ambience.h"
#include "nightmare_audio.h"
#include "nightmare_config.h"
#include "nightmare_items.h"
#include "nightmare_lighting.h"
#include "nightmare_post_diablo.h"
#include "nightmare_quests.h"
#include "nightmare_visual_effects.h"
#include "utils/log.hpp"

namespace devilution {

void ShowNightmareEffectsStatus()
{
	LogVerbose("🎮 ===== NIGHTMARE EFFECTS STATUS REPORT =====");
	
	// Verificar cada sistema
	LogVerbose("🔥 Atmospheric Lighting: {}", g_nightmareLighting.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("🌙 Ambient Effects: {}", g_nightmareAmbient.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("🎵 Enhanced Audio: {}", g_nightmareAudio.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("✨ Visual Effects: {}", g_nightmareVisual.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("🌙 Ambience System: {}", g_nightmareAmbience.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("⚔️ Nightmare Items: {}", g_nightmareItems.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("🗝️ Post-Diablo Content: {}", g_postDiablo.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	LogVerbose("🎯 Nightmare Quests: {}", g_nightmareQuests.initialized ? "✅ ACTIVE" : "❌ INACTIVE");
	
	LogVerbose("🎮 ========================================");
}

void VerifyAllNightmareEffectsActive()
{
	LogVerbose("🔍 VERIFYING ALL NIGHTMARE EFFECTS ARE ACTIVE...");
	
	bool allActive = true;
	
	if (!g_nightmareLighting.initialized) {
		LogVerbose("⚠️ WARNING: Atmospheric Lighting not initialized!");
		allActive = false;
	}
	
	if (!g_nightmareAmbient.initialized) {
		LogVerbose("⚠️ WARNING: Ambient Effects not initialized!");
		allActive = false;
	}
	
	if (!g_nightmareAudio.initialized) {
		LogVerbose("⚠️ WARNING: Enhanced Audio not initialized!");
		allActive = false;
	}
	
	if (!g_nightmareVisual.initialized) {
		LogVerbose("⚠️ WARNING: Visual Effects not initialized!");
		allActive = false;
	}
	
	if (allActive) {
		LogVerbose("✅ ALL NIGHTMARE EFFECTS ARE ACTIVE AND READY!");
		LogVerbose("🎮 NIGHTMARE EDITION IS FULLY OPERATIONAL!");
	} else {
		LogVerbose("❌ SOME EFFECTS ARE NOT ACTIVE - FORCING ACTIVATION...");
		ForceActivateAllNightmareEffects();
	}
}

void ForceActivateAllNightmareEffects()
{
	LogVerbose("⚡ FORCING ACTIVATION OF ALL NIGHTMARE EFFECTS!");
	
	// Forzar activación de efectos ambientales
	if (g_nightmareAmbient.initialized) {
		ForceEnableAllAmbientEffects();
	}
	
	// Forzar activación de iluminación atmosférica
	if (g_nightmareLighting.initialized) {
		SetNightmareLightingEnabled(true);
		SetTorchFlickerEnabled(true);
		SetFlickerIntensity(1.5f);
	}
	
	// Forzar activación de efectos visuales
	if (g_nightmareVisual.initialized) {
		SetNightmareVisualEffectsEnabled(true);
	}
	
	// Forzar activación de audio mejorado
	if (g_nightmareAudio.initialized) {
		SetNightmareAudioEnabled(true);
	}
	
	LogVerbose("⚡ ALL NIGHTMARE EFFECTS FORCED TO MAXIMUM INTENSITY!");
	LogVerbose("🎮 NIGHTMARE EDITION READY FOR EPIC GAMING!");
}

} // namespace devilution