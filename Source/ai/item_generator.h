#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <random>

#include "items.h"
#include "player.h"

namespace devilution::ai {

/**
 * @brief Item generation parameters
 */
struct ItemGenerationParams {
    int playerLevel;
    std::string playerClass;
    std::string itemCategory; // weapon, armor, accessory, consumable
    std::string itemType;     // sword, bow, helmet, ring, etc.
    int qualityTier;         // 1-5 (normal to legendary)
    float powerLevel;        // 0.0-2.0 multiplier for stats
    std::vector<std::string> preferredAffixes;
    std::vector<std::string> excludedAffixes;
    bool allowUnique = true;
    bool allowSet = true;
    std::string thematicStyle; // demonic, angelic, elemental, etc.
};

/**
 * @brief Generated item properties
 */
struct GeneratedItem {
    std::string id;
    std::string name;
    std::string description;
    std::string lore;
    
    // Base properties
    std::string category;
    std::string type;
    int itemLevel;
    int qualityTier;
    
    // Stats
    std::unordered_map<std::string, int> baseStats;
    std::unordered_map<std::string, int> bonusStats;
    std::vector<std::string> affixes;
    std::vector<std::string> specialProperties;
    
    // Visual
    std::string iconPath;
    std::string modelPath;
    std::string colorScheme;
    
    // Gameplay
    int durability;
    int requirements; // level requirement
    std::string classRestriction;
    bool isUnique;
    bool isSetItem;
    std::string setName;
    
    // Generation metadata
    float balanceScore;
    float uniquenessScore;
    uint32_t generationTime;
};

/**
 * @brief Item affix (prefix/suffix) definition
 */
struct ItemAffix {
    std::string id;
    std::string name;
    std::string type; // prefix, suffix
    std::vector<std::string> applicableCategories;
    std::unordered_map<std::string, std::pair<int, int>> statRanges; // stat -> (min, max)
    int tierRequirement; // minimum quality tier
    float rarity; // 0.0-1.0, lower = rarer
    std::vector<std::string> incompatibleAffixes;
    std::string thematicGroup; // fire, ice, poison, etc.
};

/**
 * @brief Intelligent Item Generator
 * 
 * Generates procedural items with:
 * - Balanced statistics for player level
 * - Thematic coherence and lore integration
 * - Unique visual and mechanical properties
 * - Smart affix combinations
 * - Quality progression systems
 */
class ItemGenerator {
public:
    ItemGenerator();
    ~ItemGenerator();
    
    // Initialization
    bool Initialize();
    void LoadAffixDatabase(const std::string& affixFile);
    void LoadItemTemplates(const std::string& templateFile);
    void LoadNamingData(const std::string& namingFile);
    
    // Item generation
    std::unique_ptr<GeneratedItem> GenerateItem(const ItemGenerationParams& params);
    std::vector<std::unique_ptr<GeneratedItem>> GenerateItemSet(
        const Player& player,
        int count = 5,
        const std::string& theme = ""
    );
    
    // Specialized generation
    std::unique_ptr<GeneratedItem> GenerateWeapon(const Player& player, const std::string& weaponType = "");
    std::unique_ptr<GeneratedItem> GenerateArmor(const Player& player, const std::string& armorSlot = "");
    std::unique_ptr<GeneratedItem> GenerateAccessory(const Player& player, const std::string& accessoryType = "");
    std::unique_ptr<GeneratedItem> GenerateConsumable(const Player& player, const std::string& consumableType = "");
    
    // Unique and set items
    std::unique_ptr<GeneratedItem> GenerateUniqueItem(const Player& player, const std::string& baseType = "");
    std::unique_ptr<GeneratedItem> GenerateSetItem(const Player& player, const std::string& setName = "");
    
    // Context-aware generation
    std::unique_ptr<GeneratedItem> GenerateQuestReward(const Player& player, const std::string& questId);
    std::unique_ptr<GeneratedItem> GenerateBossLoot(const Player& player, const std::string& bossName);
    std::unique_ptr<GeneratedItem> GenerateShopItem(const Player& player, const std::string& shopType);
    
    // Item enhancement and modification
    bool EnhanceItem(GeneratedItem& item, const std::string& enhancementType);
    bool AddAffix(GeneratedItem& item, const std::string& affixId);
    bool RemoveAffix(GeneratedItem& item, const std::string& affixId);
    
    // Validation and balancing
    bool ValidateItem(const GeneratedItem& item, const Player& player);
    void BalanceItemStats(GeneratedItem& item, const Player& player);
    float CalculateItemPower(const GeneratedItem& item);
    float CalculateItemValue(const GeneratedItem& item);
    
    // Affix management
    void AddAffix(const ItemAffix& affix);
    void RemoveAffix(const std::string& affixId);
    const ItemAffix* GetAffix(const std::string& affixId) const;
    std::vector<std::string> GetCompatibleAffixes(const GeneratedItem& item) const;
    
    // Configuration
    void SetGenerationSeed(uint32_t seed);
    void SetPowerScaling(float scaling) { powerScaling_ = std::clamp(scaling, 0.1f, 3.0f); }
    void SetRarityBias(float bias) { rarityBias_ = std::clamp(bias, -1.0f, 1.0f); }
    void SetCreativityLevel(float level) { creativityLevel_ = std::clamp(level, 0.0f, 1.0f); }
    void SetBalanceStrictness(float strictness) { balanceStrictness_ = std::clamp(strictness, 0.0f, 1.0f); }
    
    // Statistics
    struct GenerationStats {
        uint32_t totalGenerated = 0;
        uint32_t weaponsGenerated = 0;
        uint32_t armorGenerated = 0;
        uint32_t accessoriesGenerated = 0;
        uint32_t consumablesGenerated = 0;
        uint32_t uniqueItemsGenerated = 0;
        uint32_t setItemsGenerated = 0;
        float averageBalanceScore = 0.0f;
        float averageUniquenessScore = 0.0f;
        float averageGenerationTime = 0.0f;
        std::unordered_map<int, uint32_t> qualityTierCount;
    };
    
    const GenerationStats& GetStats() const { return stats_; }
    void ResetStats();
    
private:
    // Affix system
    std::unordered_map<std::string, ItemAffix> affixes_;
    std::unordered_map<std::string, std::vector<std::string>> affixesByCategory_;
    std::unordered_map<std::string, std::vector<std::string>> affixesByTheme_;
    
    // Naming system
    std::vector<std::string> weaponPrefixes_;
    std::vector<std::string> weaponSuffixes_;
    std::vector<std::string> armorPrefixes_;
    std::vector<std::string> armorSuffixes_;
    std::vector<std::string> uniqueNames_;
    std::vector<std::string> setNames_;
    
    // Generation state
    std::mt19937 rng_;
    float powerScaling_ = 1.0f;
    float rarityBias_ = 0.0f;
    float creativityLevel_ = 0.7f;
    float balanceStrictness_ = 0.8f;
    
    // Statistics
    GenerationStats stats_;
    
    // Generation methods
    std::string GenerateItemName(const GeneratedItem& item);
    std::string GenerateItemDescription(const GeneratedItem& item);
    std::string GenerateItemLore(const GeneratedItem& item);
    
    void GenerateBaseStats(GeneratedItem& item, const ItemGenerationParams& params);
    void GenerateBonusStats(GeneratedItem& item, const ItemGenerationParams& params);
    void ApplyAffixes(GeneratedItem& item, const ItemGenerationParams& params);
    void GenerateVisualProperties(GeneratedItem& item, const ItemGenerationParams& params);
    
    // Affix selection and application
    std::vector<std::string> SelectAffixes(const GeneratedItem& item, const ItemGenerationParams& params);
    void ApplyAffix(GeneratedItem& item, const ItemAffix& affix);
    bool AreAffixesCompatible(const std::vector<std::string>& affixIds) const;
    
    // Balance and validation
    bool IsItemBalanced(const GeneratedItem& item, const Player& player);
    void AdjustStatsForBalance(GeneratedItem& item, const Player& player);
    float CalculateStatBudget(int itemLevel, int qualityTier);
    
    // Utility methods
    int CalculateStatValue(const std::string& statName, int itemLevel, int qualityTier, float powerLevel);
    std::string SelectRandomElement(const std::vector<std::string>& elements);
    int GetRandomInRange(int min, int max);
    std::string GenerateItemId();
    float CalculateUniquenessScore(const GeneratedItem& item);
    int CalculateOptimalQualityTier(int playerLevel);
    
    // Default content loading
    void LoadDefaultAffixes();
    void LoadDefaultNaming();
};

} // namespace devilution::ai