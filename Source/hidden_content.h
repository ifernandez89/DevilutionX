/**
 * @file hidden_content.h
 *
 * Interface for Hidden Content Recovery System - discovers and integrates
 * unused texts, dialogues, and lore content from DevilutionX safely.
 */
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>

#include "tables/textdat.h"

namespace devilution {

/**
 * @brief Categories of hidden content discovered
 */
enum class HiddenContentType : uint8_t {
	NPC_GOSSIP,      // Unused NPC dialogue lines
	BOOK_TEXT,       // Decorative book content
	LORE_FRAGMENT,   // Story/lore pieces
	QUEST_REMNANT,   // Cut quest dialogue
	AMBIENT_TEXT     // Environmental storytelling
};

/**
 * @brief Structure for discovered hidden content
 */
struct HiddenContentEntry {
	_speech_id textId;
	HiddenContentType type;
	std::string content;
	std::string context;     // Where/how it was found
	bool isIntegrated;       // Whether it's been added to game
	int priority;            // Integration priority (1-10)
};

/**
 * @brief Hidden Content Recovery System
 * 
 * Discovers unused texts, dialogues, and lore content from game files
 * and integrates them safely using existing text display systems.
 */
class HiddenContentSystem {
private:
	std::vector<HiddenContentEntry> discoveredContent;
	std::unordered_map<HiddenContentType, std::vector<_speech_id>> contentByType;
	
	// Discovery methods
	void DiscoverUnusedNPCDialogues();
	void DiscoverBookTexts();
	void DiscoverLoreFragments();
	void DiscoverQuestRemnants();
	void DiscoverAmbientTexts();
	
	// Integration methods
	void IntegrateNPCGossip();
	void IntegrateBookContent();
	void IntegrateLoreContent();
	
	// Safety validation
	bool ValidateTextSafety(const std::string& text) const;
	bool ValidateIntegrationSafety(_speech_id textId) const;
	
public:
	/**
	 * @brief Initialize the Hidden Content Recovery System
	 */
	void Initialize();
	
	/**
	 * @brief Discover all hidden content from game files
	 * @return Number of content pieces discovered
	 */
	int DiscoverHiddenContent();
	
	/**
	 * @brief Integrate discovered content into the game safely
	 * @param maxIntegrations Maximum number of pieces to integrate
	 * @return Number of pieces successfully integrated
	 */
	int IntegrateContent(int maxIntegrations = 50);
	
	/**
	 * @brief Get discovered content by type
	 */
	const std::vector<_speech_id>& GetContentByType(HiddenContentType type) const;
	
	/**
	 * @brief Get total discovered content count
	 */
	size_t GetDiscoveredCount() const { return discoveredContent.size(); }
	
	/**
	 * @brief Get integrated content count
	 */
	size_t GetIntegratedCount() const;
	
	/**
	 * @brief Check if system is initialized
	 */
	bool IsInitialized() const { return !discoveredContent.empty(); }
};

// Global instance
extern HiddenContentSystem g_hiddenContent;

/**
 * @brief Initialize hidden content system (called during game startup)
 */
void InitHiddenContent();

/**
 * @brief Get a random hidden gossip text for NPCs
 * @param fallback Fallback text if no hidden content available
 * @return Speech ID for hidden gossip or fallback
 */
_speech_id GetRandomHiddenGossip(_speech_id fallback);

/**
 * @brief Get a random hidden book text
 * @param fallback Fallback text if no hidden content available
 * @return Speech ID for hidden book text or fallback
 */
_speech_id GetRandomHiddenBookText(_speech_id fallback);

} // namespace devilution