/**
 * @file enhanced_portal.h
 *
 * Interface for Enhanced Portal System - improves visual feedback and atmosphere
 * of portals without modifying teleport logic, coordinates, or gameplay.
 */
#pragma once

#include <cstdint>

#include "engine/point.hpp"

namespace devilution {

/**
 * @brief Portal visual enhancement types
 */
enum class PortalEnhancementType : uint8_t {
	NORMAL,      // Standard portal appearance
	QUEST_ACTIVE, // Quest-related portal (warmer colors)
	BOSS_AREA,   // Portal to boss area (more intense)
	SPECIAL_EVENT // Special event portal (unique colors)
};

/**
 * @brief Portal visual intensity levels
 */
enum class PortalIntensity : uint8_t {
	LOW = 1,     // Subtle enhancement
	MEDIUM = 2,  // Moderate enhancement  
	HIGH = 3     // Strong enhancement
};

/**
 * @brief Enhanced Portal System
 * 
 * Provides visual and audio enhancements to portals without affecting
 * their core functionality, teleport logic, or gameplay mechanics.
 */
class EnhancedPortalSystem {
private:
	bool m_enabled;
	uint32_t m_lastUpdateTime;
	PortalIntensity m_currentIntensity;
	
	// Visual enhancement state
	float m_colorModifier;
	float m_intensityModifier;
	
	// Audio throttling
	uint32_t m_lastAudioTime;
	static constexpr uint32_t AUDIO_THROTTLE_MS = 2000; // 2 seconds minimum between sounds
	
	// Safety fallback
	bool m_fallbackMode;
	
public:
	/**
	 * @brief Initialize the Enhanced Portal System
	 */
	void Initialize();
	
	/**
	 * @brief Update portal enhancements (called on timer, not per frame)
	 */
	void Update();
	
	/**
	 * @brief Get portal enhancement type based on context
	 * @param portalPosition Position of the portal
	 * @return Enhancement type to apply
	 */
	PortalEnhancementType GetPortalEnhancementType(Point portalPosition) const;
	
	/**
	 * @brief Get color modifier for portal rendering
	 * @param enhancementType Type of enhancement
	 * @return Color modifier value (0.0-2.0, 1.0 = normal)
	 */
	float GetColorModifier(PortalEnhancementType enhancementType) const;
	
	/**
	 * @brief Get intensity modifier for portal effects
	 * @param enhancementType Type of enhancement
	 * @return Intensity modifier value (0.0-2.0, 1.0 = normal)
	 */
	float GetIntensityModifier(PortalEnhancementType enhancementType) const;
	
	/**
	 * @brief Play enhanced portal audio (throttled)
	 * @param enhancementType Type of enhancement
	 * @param portalPosition Position of the portal
	 */
	void PlayEnhancedPortalAudio(PortalEnhancementType enhancementType, Point portalPosition);
	
	/**
	 * @brief Enable/disable the enhanced portal system
	 */
	void SetEnabled(bool enabled) { m_enabled = enabled; }
	
	/**
	 * @brief Check if system is enabled
	 */
	bool IsEnabled() const { return m_enabled && !m_fallbackMode; }
	
	/**
	 * @brief Force fallback to classic portal rendering
	 */
	void EnableFallbackMode() { m_fallbackMode = true; }
	
	/**
	 * @brief Check if in fallback mode
	 */
	bool IsInFallbackMode() const { return m_fallbackMode; }
};

// Global instance
extern EnhancedPortalSystem g_enhancedPortal;

/**
 * @brief Initialize enhanced portal system (called during game startup)
 */
void InitEnhancedPortal();

/**
 * @brief Update enhanced portal system (called on timer)
 */
void UpdateEnhancedPortal();

/**
 * @brief Get enhanced portal color modifier for rendering
 * @param portalPosition Position of the portal
 * @return Color modifier (1.0 = normal, fallback if system disabled)
 */
float GetPortalColorModifier(Point portalPosition);

/**
 * @brief Get enhanced portal intensity modifier for rendering
 * @param portalPosition Position of the portal  
 * @return Intensity modifier (1.0 = normal, fallback if system disabled)
 */
float GetPortalIntensityModifier(Point portalPosition);

/**
 * @brief Trigger enhanced portal audio feedback
 * @param portalPosition Position of the portal
 */
void TriggerPortalAudioFeedback(Point portalPosition);

} // namespace devilution