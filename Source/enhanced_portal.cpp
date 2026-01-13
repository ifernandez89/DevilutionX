/**
 * @file enhanced_portal.cpp
 *
 * Implementation of Enhanced Portal System - improves visual feedback and atmosphere
 * of portals without modifying teleport logic, coordinates, or gameplay.
 */
#include "enhanced_portal.h"

#include <algorithm>
#include <cmath>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/random.hpp"
#include "quests.h"
#include "utils/log.hpp"
#include "player.h"

namespace devilution {

// Global instance
EnhancedPortalSystem g_enhancedPortal;

void EnhancedPortalSystem::Initialize()
{
	Log("Enhanced Portal System: Initializing...");
	
	m_enabled = true;
	m_lastUpdateTime = 0;
	m_currentIntensity = PortalIntensity::MEDIUM;
	m_colorModifier = 1.0f;
	m_intensityModifier = 1.0f;
	m_lastAudioTime = 0;
	m_fallbackMode = false;
	
	Log("Enhanced Portal System: Initialized successfully");
}

void EnhancedPortalSystem::Update()
{
	if (!m_enabled || m_fallbackMode) {
		return;
	}
	
	// SAFETY: Only update on timer intervals, not per frame
	const uint32_t currentTime = SDL_GetTicks();
	if (currentTime - m_lastUpdateTime < 500) { // Update every 500ms maximum
		return;
	}
	m_lastUpdateTime = currentTime;
	
	// Simple dynamic intensity based on time (creates subtle pulsing effect)
	const float timePhase = (currentTime % 4000) / 4000.0f; // 4 second cycle
	const float pulse = 0.8f + 0.2f * std::sin(timePhase * 2.0f * 3.14159f); // Gentle pulse 0.8-1.0
	
	m_intensityModifier = pulse;
	m_colorModifier = 0.9f + 0.1f * pulse; // Subtle color variation
}

PortalEnhancementType EnhancedPortalSystem::GetPortalEnhancementType(Point portalPosition) const
{
	if (!m_enabled || m_fallbackMode) {
		return PortalEnhancementType::NORMAL;
	}
	
	// MINIMAL LOGIC: Simple context detection
	
	// Check if any quest is active (quest portal gets warmer colors)
	for (const auto& quest : Quests) {
		if (quest._qactive == QUEST_ACTIVE) {
			return PortalEnhancementType::QUEST_ACTIVE;
		}
	}
	
	// Check if we're in a boss area (based on current level)
	if (currlevel == 4 || currlevel == 8 || currlevel == 12 || currlevel == 16) {
		return PortalEnhancementType::BOSS_AREA;
	}
	
	// 5% chance for special event portal (rare visual treat)
	if (GenerateRnd(100) < 5) {
		return PortalEnhancementType::SPECIAL_EVENT;
	}
	
	return PortalEnhancementType::NORMAL;
}

float EnhancedPortalSystem::GetColorModifier(PortalEnhancementType enhancementType) const
{
	if (!m_enabled || m_fallbackMode) {
		return 1.0f; // Fallback to normal
	}
	
	// MINIMAL ENHANCEMENT: Subtle color variations
	switch (enhancementType) {
	case PortalEnhancementType::QUEST_ACTIVE:
		return m_colorModifier * 1.1f; // Slightly warmer
	case PortalEnhancementType::BOSS_AREA:
		return m_colorModifier * 1.2f; // More intense
	case PortalEnhancementType::SPECIAL_EVENT:
		return m_colorModifier * 1.3f; // Unique appearance
	case PortalEnhancementType::NORMAL:
	default:
		return m_colorModifier; // Standard with pulse
	}
}

float EnhancedPortalSystem::GetIntensityModifier(PortalEnhancementType enhancementType) const
{
	if (!m_enabled || m_fallbackMode) {
		return 1.0f; // Fallback to normal
	}
	
	// MINIMAL ENHANCEMENT: Subtle intensity variations
	switch (enhancementType) {
	case PortalEnhancementType::QUEST_ACTIVE:
		return m_intensityModifier * 1.05f; // Slightly more visible
	case PortalEnhancementType::BOSS_AREA:
		return m_intensityModifier * 1.15f; // More dramatic
	case PortalEnhancementType::SPECIAL_EVENT:
		return m_intensityModifier * 1.25f; // Eye-catching
	case PortalEnhancementType::NORMAL:
	default:
		return m_intensityModifier; // Standard with pulse
	}
}

void EnhancedPortalSystem::PlayEnhancedPortalAudio(PortalEnhancementType enhancementType, Point portalPosition)
{
	if (!m_enabled || m_fallbackMode) {
		return;
	}
	
	// SAFETY: Throttle audio to prevent spam
	const uint32_t currentTime = SDL_GetTicks();
	if (currentTime - m_lastAudioTime < AUDIO_THROTTLE_MS) {
		return;
	}
	m_lastAudioTime = currentTime;
	
	// MINIMAL AUDIO: Only subtle enhancements, no new sounds
	// This would integrate with existing audio system to modify portal sounds slightly
	// For now, just log the enhancement (actual audio integration would be minimal)
	
	switch (enhancementType) {
	case PortalEnhancementType::QUEST_ACTIVE:
		Log("Enhanced Portal System: Quest portal audio enhancement at ({}, {})", 
			portalPosition.x, portalPosition.y);
		break;
	case PortalEnhancementType::BOSS_AREA:
		Log("Enhanced Portal System: Boss portal audio enhancement at ({}, {})", 
			portalPosition.x, portalPosition.y);
		break;
	case PortalEnhancementType::SPECIAL_EVENT:
		Log("Enhanced Portal System: Special portal audio enhancement at ({}, {})", 
			portalPosition.x, portalPosition.y);
		break;
	default:
		// No audio enhancement for normal portals
		break;
	}
}

// Global functions

void InitEnhancedPortal()
{
	// SAFETY: Only initialize if basic systems are ready
	if (MyPlayer == nullptr) {
		Log("Enhanced Portal System: Deferred initialization - game not ready");
		return;
	}
	
	g_enhancedPortal.Initialize();
}

void UpdateEnhancedPortal()
{
	// SAFETY: Update only if system is initialized
	if (g_enhancedPortal.IsEnabled()) {
		g_enhancedPortal.Update();
	}
}

float GetPortalColorModifier(Point portalPosition)
{
	// SAFETY: Always return valid value, fallback to 1.0 if system not ready
	if (!g_enhancedPortal.IsEnabled()) {
		return 1.0f;
	}
	
	const PortalEnhancementType enhancementType = g_enhancedPortal.GetPortalEnhancementType(portalPosition);
	return g_enhancedPortal.GetColorModifier(enhancementType);
}

float GetPortalIntensityModifier(Point portalPosition)
{
	// SAFETY: Always return valid value, fallback to 1.0 if system not ready
	if (!g_enhancedPortal.IsEnabled()) {
		return 1.0f;
	}
	
	const PortalEnhancementType enhancementType = g_enhancedPortal.GetPortalEnhancementType(portalPosition);
	return g_enhancedPortal.GetIntensityModifier(enhancementType);
}

void TriggerPortalAudioFeedback(Point portalPosition)
{
	// SAFETY: Only trigger if system is ready
	if (!g_enhancedPortal.IsEnabled()) {
		return;
	}
	
	const PortalEnhancementType enhancementType = g_enhancedPortal.GetPortalEnhancementType(portalPosition);
	g_enhancedPortal.PlayEnhancedPortalAudio(enhancementType, portalPosition);
}

} // namespace devilution