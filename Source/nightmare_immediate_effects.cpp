/**
 * @file nightmare_immediate_effects.cpp
 * @brief Nightmare Edition - Immediate Visible Effects Implementation
 * 
 * IMPLEMENTACIÓN DE EFECTOS INMEDIATOS
 * ===================================
 * 
 * Sistema que crea efectos visibles inmediatos.
 */

#include "nightmare_immediate_effects.h"

#include "nightmare_atmosphere.h"
#include "utils/log.hpp"

namespace devilution {

// Estado de efectos inmediatos
namespace {
	float currentDarkening = 0.0f;
	float currentDesaturation = 0.0f;
	float currentRedTint = 0.0f;
	bool visualFogActive = false;
	bool visualDarkeningActive = false;
}

void ApplyImmediateDarkening(float intensity)
{
	currentDarkening = intensity;
	
	// Aplicar al sistema de atmósfera nightmare
	if (intensity > 0.0f) {
		// Aumentar desaturación para crear efecto de oscurecimiento
		nightmareAtmosphere.desaturationLevel = 0.06f + (intensity * 0.3f);
		LogVerbose("🌙 Immediate darkening applied: {:.2f} (desaturation: {:.2f})", 
			intensity, nightmareAtmosphere.desaturationLevel);
	} else {
		nightmareAtmosphere.desaturationLevel = 0.06f;  // Valor base
		LogVerbose("🌙 Darkening reset to base level");
	}
}

void ApplyImmediateDesaturation(float intensity)
{
	currentDesaturation = intensity;
	
	// Aplicar directamente al sistema de atmósfera
	nightmareAtmosphere.desaturationLevel = 0.06f + (intensity * 0.4f);
	
	LogVerbose("🎨 Immediate desaturation applied: {:.2f} (total: {:.2f})", 
		intensity, nightmareAtmosphere.desaturationLevel);
}

void ApplyImmediateRedTint(float intensity)
{
	currentRedTint = intensity;
	
	LogVerbose("🔴 Immediate red tint applied: {:.2f}", intensity);
	// TODO: Implementar tinte rojizo real
}

void ResetImmediateEffects()
{
	LogVerbose("🔄 === RESETTING ALL NIGHTMARE EFFECTS ===");
	
	currentDarkening = 0.0f;
	currentDesaturation = 0.0f;
	currentRedTint = 0.0f;
	
	// Reset atmósfera a valores base
	nightmareAtmosphere.desaturationLevel = 0.06f;  // 6% base
	
	LogVerbose("🔄 All immediate effects reset to base values");
	LogVerbose("🎨 Desaturation level: {:.2f}", nightmareAtmosphere.desaturationLevel);
	LogVerbose("🔄 === RESET COMPLETE ===");
}

void ToggleVisualFog()
{
	visualFogActive = !visualFogActive;
	
	LogVerbose("🌫️ === VISUAL FOG TOGGLE ===");
	LogVerbose("🌫️ New state: {}", visualFogActive ? "ENABLED" : "DISABLED");
	
	if (visualFogActive) {
		// Aplicar efecto de niebla aumentando desaturación
		ApplyImmediateDesaturation(0.3f);
		LogVerbose("🌫️ Visual fog ENABLED - increased desaturation");
	} else {
		// Quitar efecto de niebla
		ResetImmediateEffects();
		LogVerbose("🌫️ Visual fog DISABLED - effects reset");
	}
	
	LogVerbose("🌫️ === FOG TOGGLE COMPLETE ===");
}

void ToggleVisualDarkening()
{
	visualDarkeningActive = !visualDarkeningActive;
	
	if (visualDarkeningActive) {
		// Aplicar oscurecimiento fuerte
		ApplyImmediateDarkening(0.5f);
		LogVerbose("🌙 Visual darkening ENABLED - strong atmosphere");
	} else {
		// Quitar oscurecimiento
		ResetImmediateEffects();
		LogVerbose("🌙 Visual darkening DISABLED - effects reset");
	}
}

void ShowActiveEffectsInfo()
{
	LogVerbose("🎭 === ACTIVE VISUAL EFFECTS ===");
	LogVerbose("🌙 Darkening: {:.2f}", currentDarkening);
	LogVerbose("🎨 Desaturation: {:.2f}", currentDesaturation);
	LogVerbose("🔴 Red Tint: {:.2f}", currentRedTint);
	LogVerbose("🌫️ Visual Fog: {}", visualFogActive ? "ACTIVE" : "inactive");
	LogVerbose("🌙 Visual Darkening: {}", visualDarkeningActive ? "ACTIVE" : "inactive");
	LogVerbose("🎨 Current Atmosphere Desaturation: {:.2f}", nightmareAtmosphere.desaturationLevel);
	LogVerbose("🎭 === END EFFECTS INFO ===");
}

} // namespace devilution