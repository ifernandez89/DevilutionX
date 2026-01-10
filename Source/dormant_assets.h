/**
 * @file dormant_assets.h
 *
 * Interface for Dormant Assets Recovery System - discovers and reactivates
 * unused art, sprites, animations, effects, music and decorative elements
 * from DevilutionX safely to enrich visual experience.
 */
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "engine/clx_sprite.hpp"
#include "engine/point.hpp"
#include "levels/dun_tile.hpp"

namespace devilution {

/**
 * @brief Categories of dormant visual assets
 */
enum class DormantAssetType : uint8_t {
	MONSTER_VARIANT,     // Alternative monster sprites/colors
	DECORATIVE_OBJECT,   // Unused decorations (altars, statues, furniture)
	VISUAL_EFFECT,       // Dormant particle effects (fire, smoke, magic)
	UI_ELEMENT,          // Prototype UI graphics (bars, icons, indicators)
	AMBIENT_SOUND,       // Unused audio tracks and sound effects
	NARRATIVE_ART        // Conceptual art, symbols, cinematic elements
};

/**
 * @brief Structure for discovered dormant assets
 */
struct DormantAssetEntry {
	std::string assetId;
	DormantAssetType type;
	std::string filePath;        // Path to asset file
	std::string description;     // What the asset represents
	bool isActive;              // Whether it's currently integrated
	int aestheticValue;         // Visual impact rating (1-10)
	int riskLevel;              // Integration risk (1-5, lower is safer)
	std::string integrationContext; // Where/how to use it
};

/**
 * @brief Dormant Assets Recovery System
 * 
 * Discovers unused visual assets and reactivates them safely
 * to enhance the game's aesthetic experience without affecting gameplay.
 */
class DormantAssetsSystem {
private:
	std::vector<DormantAssetEntry> discoveredAssets;
	std::unordered_map<DormantAssetType, std::vector<std::string>> assetsByType;
	
	// Discovery methods
	void DiscoverMonsterVariants();
	void DiscoverDecorativeObjects();
	void DiscoverVisualEffects();
	void DiscoverUIElements();
	void DiscoverAmbientSounds();
	void DiscoverNarrativeArt();
	
	// Integration methods
	void IntegrateMonsterVariants();
	void IntegrateDecorativeObjects();
	void IntegrateVisualEffects();
	void IntegrateUIElements();
	void IntegrateAmbientSounds();
	
	// Safety validation
	bool ValidateAssetSafety(const DormantAssetEntry& asset) const;
	bool ValidateIntegrationSafety(const std::string& assetId) const;
	
public:
	/**
	 * @brief Initialize the Dormant Assets Recovery System
	 */
	void Initialize();
	
	/**
	 * @brief Discover all dormant assets from game files
	 * @return Number of assets discovered
	 */
	int DiscoverDormantAssets();
	
	/**
	 * @brief Integrate discovered assets into the game safely
	 * @param maxIntegrations Maximum number of assets to integrate
	 * @param minAestheticValue Minimum aesthetic value to consider
	 * @param maxRiskLevel Maximum risk level to accept
	 * @return Number of assets successfully integrated
	 */
	int IntegrateAssets(int maxIntegrations = 20, int minAestheticValue = 6, int maxRiskLevel = 2);
	
	/**
	 * @brief Get discovered assets by type
	 */
	const std::vector<std::string>& GetAssetsByType(DormantAssetType type) const;
	
	/**
	 * @brief Get total discovered assets count
	 */
	size_t GetDiscoveredCount() const { return discoveredAssets.size(); }
	
	/**
	 * @brief Get integrated assets count
	 */
	size_t GetIntegratedCount() const;
	
	/**
	 * @brief Check if system is initialized
	 */
	bool IsInitialized() const { return !discoveredAssets.empty(); }
	
	/**
	 * @brief Get random decorative object for level enhancement
	 */
	std::string GetRandomDecorativeObject() const;
	
	/**
	 * @brief Get random visual effect for combat enhancement
	 */
	std::string GetRandomVisualEffect() const;
};

// Global instance
extern DormantAssetsSystem g_dormantAssets;

/**
 * @brief Initialize dormant assets system (called during game startup)
 */
void InitDormantAssets();

/**
 * @brief Enhance level with dormant decorative objects
 * @param levelType Type of level (dungeon, caves, hell, etc.)
 * @param enhancementChance Percentage chance to add decorations (0-100)
 */
void EnhanceLevelWithDormantAssets(int levelType, int enhancementChance = 15);

/**
 * @brief Get enhanced monster variant sprite (if available)
 * @param originalMonsterType Original monster type
 * @return Enhanced sprite path or empty string if none available
 */
std::string GetEnhancedMonsterVariant(int originalMonsterType);

/**
 * @brief Add dormant visual effect to combat
 * @param effectType Type of effect needed
 * @param position Where to place the effect
 * @return True if effect was added successfully
 */
bool AddDormantVisualEffect(const std::string& effectType, Point position);

} // namespace devilution