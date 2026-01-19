#include "ai/item_generator.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

namespace devilution::ai {

namespace {

// Base item categories and types
const std::unordered_map<std::string, std::vector<std::string>> ITEM_CATEGORIES = {
    {"weapon", {"sword", "axe", "mace", "bow", "staff", "dagger"}},
    {"armor", {"helmet", "armor", "shield", "boots", "gloves"}},
    {"accessory", {"ring", "amulet", "belt"}},
    {"consumable", {"potion", "scroll", "elixir", "tome"}}
};

// Stat names and their importance weights
const std::unordered_map<std::string, float> STAT_WEIGHTS = {
    {"damage", 1.0f},
    {"armor", 1.0f},
    {"strength", 0.8f},
    {"dexterity", 0.8f},
    {"magic", 0.8f},
    {"vitality", 0.9f},
    {"life", 0.9f},
    {"mana", 0.7f},
    {"resistance", 0.6f},
    {"speed", 0.5f},
    {"luck", 0.4f}
};

// Quality tier names and multipliers
const std::unordered_map<int, std::pair<std::string, float>> QUALITY_TIERS = {
    {1, {"Normal", 1.0f}},
    {2, {"Magic", 1.3f}},
    {3, {"Rare", 1.7f}},
    {4, {"Epic", 2.2f}},
    {5, {"Legendary", 3.0f}}
};

// Thematic groups for coherent item generation
const std::unordered_map<std::string, std::vector<std::string>> THEMATIC_GROUPS = {
    {"fire", {"flame", "burning", "infernal", "blazing", "scorching"}},
    {"ice", {"frost", "frozen", "glacial", "arctic", "chilling"}},
    {"poison", {"toxic", "venomous", "noxious", "corrupted", "plagued"}},
    {"lightning", {"shocking", "electric", "thunderous", "sparking", "charged"}},
    {"shadow", {"dark", "shadowy", "umbral", "void", "cursed"}},
    {"holy", {"blessed", "divine", "sacred", "righteous", "pure"}}
};

} // anonymous namespace

ItemGenerator::ItemGenerator() 
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {
}

ItemGenerator::~ItemGenerator() = default;

bool ItemGenerator::Initialize() {
    LogVerbose("Initializing AI Item Generator...");
    
    // Load default content
    LoadDefaultAffixes();
    LoadDefaultNaming();
    
    LogVerbose("Item Generator initialized with {} affixes", affixes_.size());
    return true;
}

void ItemGenerator::LoadDefaultAffixes() {
    // Weapon affixes
    ItemAffix sharpAffix;
    sharpAffix.id = "sharp";
    sharpAffix.name = "Sharp";
    sharpAffix.type = "prefix";
    sharpAffix.applicableCategories = {"weapon"};
    sharpAffix.statRanges["damage"] = {5, 15};
    sharpAffix.tierRequirement = 1;
    sharpAffix.rarity = 0.7f;
    sharpAffix.thematicGroup = "physical";
    affixes_[sharpAffix.id] = sharpAffix;
    
    ItemAffix fireAffix;
    fireAffix.id = "flaming";
    fireAffix.name = "Flaming";
    fireAffix.type = "prefix";
    fireAffix.applicableCategories = {"weapon"};
    fireAffix.statRanges["damage"] = {3, 8};
    fireAffix.statRanges["fire_damage"] = {2, 6};
    fireAffix.tierRequirement = 2;
    fireAffix.rarity = 0.4f;
    fireAffix.thematicGroup = "fire";
    affixes_[fireAffix.id] = fireAffix;
    
    // Armor affixes
    ItemAffix sturdyAffix;
    sturdyAffix.id = "sturdy";
    sturdyAffix.name = "Sturdy";
    sturdyAffix.type = "prefix";
    sturdyAffix.applicableCategories = {"armor"};
    sturdyAffix.statRanges["armor"] = {5, 12};
    sturdyAffix.tierRequirement = 1;
    sturdyAffix.rarity = 0.8f;
    sturdyAffix.thematicGroup = "defense";
    affixes_[sturdyAffix.id] = sturdyAffix;
    
    ItemAffix protectionAffix;
    protectionAffix.id = "protection";
    protectionAffix.name = "of Protection";
    protectionAffix.type = "suffix";
    protectionAffix.applicableCategories = {"armor", "accessory"};
    protectionAffix.statRanges["resistance"] = {10, 25};
    protectionAffix.tierRequirement = 2;
    protectionAffix.rarity = 0.5f;
    protectionAffix.thematicGroup = "defense";
    affixes_[protectionAffix.id] = protectionAffix;
    
    // Accessory affixes
    ItemAffix strengthAffix;
    strengthAffix.id = "strength";
    strengthAffix.name = "of Strength";
    strengthAffix.type = "suffix";
    strengthAffix.applicableCategories = {"accessory", "weapon"};
    strengthAffix.statRanges["strength"] = {3, 10};
    strengthAffix.tierRequirement = 1;
    strengthAffix.rarity = 0.6f;
    strengthAffix.thematicGroup = "attribute";
    affixes_[strengthAffix.id] = strengthAffix;
    
    // Organize affixes by category and theme
    for (const auto& [id, affix] : affixes_) {
        for (const auto& category : affix.applicableCategories) {
            affixesByCategory_[category].push_back(id);
        }
        affixesByTheme_[affix.thematicGroup].push_back(id);
    }
}

void ItemGenerator::LoadDefaultNaming() {
    weaponPrefixes_ = {"Sharp", "Keen", "Deadly", "Brutal", "Swift", "Heavy", "Fine", "Masterwork"};
    weaponSuffixes_ = {"Slaying", "Wounding", "Striking", "Cutting", "Piercing", "Smashing"};
    
    armorPrefixes_ = {"Sturdy", "Reinforced", "Thick", "Layered", "Plated", "Hardened"};
    armorSuffixes_ = {"Protection", "Defense", "Warding", "Shielding", "Guarding"};
    
    uniqueNames_ = {
        "Shadowfang", "Lightbringer", "Soulrender", "Stormcaller", "Frostmourne",
        "Flamberge", "Voidwalker", "Doomhammer", "Starfall", "Nightwhisper"
    };
    
    setNames_ = {
        "Immortal King's", "Tal Rasha's", "Griswold's", "Sigon's", "Berserker's",
        "Death's", "Orphan's", "Cow King's", "Aldur's", "Natalya's"
    };
}

std::unique_ptr<GeneratedItem> ItemGenerator::GenerateItem(const ItemGenerationParams& params) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    auto item = std::make_unique<GeneratedItem>();
    item->id = GenerateItemId();
    item->category = params.itemCategory;
    item->type = params.itemType;
    item->itemLevel = params.playerLevel;
    item->qualityTier = params.qualityTier;
    item->isUnique = false;
    item->isSetItem = false;
    
    // Generate base properties
    GenerateBaseStats(*item, params);
    GenerateBonusStats(*item, params);
    ApplyAffixes(*item, params);
    GenerateVisualProperties(*item, params);
    
    // Generate names and descriptions
    item->name = GenerateItemName(*item);
    item->description = GenerateItemDescription(*item);
    item->lore = GenerateItemLore(*item);
    
    // Set gameplay properties
    item->durability = 100; // Base durability
    item->requirements = std::max(1, params.playerLevel - 2);
    item->classRestriction = params.playerClass;
    
    // Validate and balance
    if (!ValidateItem(*item, Player{})) { // Simplified validation
        LogWarn("Generated item failed validation");
        return nullptr;
    }
    
    BalanceItemStats(*item, Player{});
    item->balanceScore = CalculateItemPower(*item);
    item->uniquenessScore = CalculateUniquenessScore(*item);
    
    // Update statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    stats_.totalGenerated++;
    if (params.itemCategory == "weapon") stats_.weaponsGenerated++;
    else if (params.itemCategory == "armor") stats_.armorGenerated++;
    else if (params.itemCategory == "accessory") stats_.accessoriesGenerated++;
    else if (params.itemCategory == "consumable") stats_.consumablesGenerated++;
    
    if (item->isUnique) stats_.uniqueItemsGenerated++;
    if (item->isSetItem) stats_.setItemsGenerated++;
    
    stats_.qualityTierCount[item->qualityTier]++;
    stats_.averageBalanceScore = (stats_.averageBalanceScore * (stats_.totalGenerated - 1) + 
                                 item->balanceScore) / stats_.totalGenerated;
    stats_.averageUniquenessScore = (stats_.averageUniquenessScore * (stats_.totalGenerated - 1) + 
                                    item->uniquenessScore) / stats_.totalGenerated;
    stats_.averageGenerationTime = (stats_.averageGenerationTime * (stats_.totalGenerated - 1) + 
                                   duration.count() / 1000.0f) / stats_.totalGenerated;
    
    item->generationTime = static_cast<uint32_t>(duration.count());
    
    LogVerbose("Generated {} '{}' (balance: {:.2f}, uniqueness: {:.2f}, time: {:.2f}ms)", 
               item->category, item->name, item->balanceScore, item->uniquenessScore, 
               duration.count() / 1000.0f);
    
    return item;
}

std::unique_ptr<GeneratedItem> ItemGenerator::GenerateWeapon(const Player& player, const std::string& weaponType) {
    ItemGenerationParams params;
    params.playerLevel = player.getCharacterLevel();
    params.playerClass = "warrior"; // Simplified
    params.itemCategory = "weapon";
    params.itemType = weaponType.empty() ? SelectRandomElement(ITEM_CATEGORIES.at("weapon")) : weaponType;
    params.qualityTier = CalculateOptimalQualityTier(player.getCharacterLevel());
    params.powerLevel = 1.0f + (player.getCharacterLevel() * 0.05f);
    
    return GenerateItem(params);
}

std::unique_ptr<GeneratedItem> ItemGenerator::GenerateArmor(const Player& player, const std::string& armorSlot) {
    ItemGenerationParams params;
    params.playerLevel = player.getCharacterLevel();
    params.playerClass = "warrior"; // Simplified
    params.itemCategory = "armor";
    params.itemType = armorSlot.empty() ? SelectRandomElement(ITEM_CATEGORIES.at("armor")) : armorSlot;
    params.qualityTier = CalculateOptimalQualityTier(player.getCharacterLevel());
    params.powerLevel = 1.0f + (player.getCharacterLevel() * 0.05f);
    
    return GenerateItem(params);
}

void ItemGenerator::GenerateBaseStats(GeneratedItem& item, const ItemGenerationParams& params) {
    // Generate base stats based on item type and level
    if (item.category == "weapon") {
        int baseDamage = params.playerLevel * 2 + GetRandomInRange(1, 5);
        item.baseStats["damage"] = baseDamage;
        
        if (item.type == "bow") {
            item.baseStats["dexterity"] = GetRandomInRange(1, 3);
        } else if (item.type == "staff") {
            item.baseStats["magic"] = GetRandomInRange(1, 3);
        } else {
            item.baseStats["strength"] = GetRandomInRange(1, 3);
        }
    } else if (item.category == "armor") {
        int baseArmor = params.playerLevel + GetRandomInRange(2, 8);
        item.baseStats["armor"] = baseArmor;
        item.baseStats["vitality"] = GetRandomInRange(1, 2);
    } else if (item.category == "accessory") {
        // Accessories focus on secondary stats
        item.baseStats["life"] = GetRandomInRange(5, 15);
        item.baseStats["mana"] = GetRandomInRange(3, 10);
    }
}

void ItemGenerator::GenerateBonusStats(GeneratedItem& item, const ItemGenerationParams& params) {
    // Quality tier determines number and magnitude of bonus stats
    auto tierInfo = QUALITY_TIERS.at(params.qualityTier);
    float multiplier = tierInfo.second;
    
    int bonusStatCount = params.qualityTier - 1; // Normal = 0, Magic = 1, etc.
    
    std::vector<std::string> availableStats = {"strength", "dexterity", "magic", "vitality", "life", "mana"};
    std::shuffle(availableStats.begin(), availableStats.end(), rng_);
    
    for (int i = 0; i < bonusStatCount && i < static_cast<int>(availableStats.size()); ++i) {
        std::string stat = availableStats[i];
        int value = static_cast<int>(GetRandomInRange(1, 5) * multiplier * params.powerLevel);
        item.bonusStats[stat] = value;
    }
}

void ItemGenerator::ApplyAffixes(GeneratedItem& item, const ItemGenerationParams& params) {
    std::vector<std::string> selectedAffixes = SelectAffixes(item, params);
    
    for (const std::string& affixId : selectedAffixes) {
        const ItemAffix* affix = GetAffix(affixId);
        if (affix) {
            ApplyAffix(item, *affix);
            item.affixes.push_back(affixId);
        }
    }
}

std::vector<std::string> ItemGenerator::SelectAffixes(const GeneratedItem& item, const ItemGenerationParams& params) {
    std::vector<std::string> candidates;
    
    // Get affixes applicable to this item category
    auto categoryIt = affixesByCategory_.find(item.category);
    if (categoryIt != affixesByCategory_.end()) {
        for (const std::string& affixId : categoryIt->second) {
            const ItemAffix* affix = GetAffix(affixId);
            if (affix && affix->tierRequirement <= params.qualityTier) {
                candidates.push_back(affixId);
            }
        }
    }
    
    // Select affixes based on quality tier
    int maxAffixes = std::min(params.qualityTier, 3);
    std::vector<std::string> selected;
    
    std::shuffle(candidates.begin(), candidates.end(), rng_);
    
    for (int i = 0; i < maxAffixes && i < static_cast<int>(candidates.size()); ++i) {
        const ItemAffix* affix = GetAffix(candidates[i]);
        if (affix) {
            // Check rarity
            std::uniform_real_distribution<float> rarityRoll(0.0f, 1.0f);
            if (rarityRoll(rng_) <= affix->rarity) {
                selected.push_back(candidates[i]);
            }
        }
    }
    
    return selected;
}

void ItemGenerator::ApplyAffix(GeneratedItem& item, const ItemAffix& affix) {
    for (const auto& [stat, range] : affix.statRanges) {
        int value = GetRandomInRange(range.first, range.second);
        item.bonusStats[stat] += value; // Add to existing bonus stats
    }
}

void ItemGenerator::GenerateVisualProperties(GeneratedItem& item, const ItemGenerationParams& params) {
    // Generate visual properties based on affixes and theme
    item.iconPath = "items/" + item.category + "_" + item.type + ".png";
    item.modelPath = "models/" + item.category + "/" + item.type + ".3d";
    
    // Color scheme based on quality and theme
    if (item.qualityTier == 1) {
        item.colorScheme = "white";
    } else if (item.qualityTier == 2) {
        item.colorScheme = "blue";
    } else if (item.qualityTier == 3) {
        item.colorScheme = "yellow";
    } else if (item.qualityTier == 4) {
        item.colorScheme = "orange";
    } else {
        item.colorScheme = "red";
    }
}

std::string ItemGenerator::GenerateItemName(const GeneratedItem& item) {
    std::string name;
    
    if (item.isUnique) {
        name = SelectRandomElement(uniqueNames_);
    } else if (item.isSetItem) {
        name = SelectRandomElement(setNames_) + " " + item.type;
    } else {
        // Generate name based on affixes
        std::string prefix, suffix;
        
        for (const std::string& affixId : item.affixes) {
            const ItemAffix* affix = GetAffix(affixId);
            if (affix) {
                if (affix->type == "prefix" && prefix.empty()) {
                    prefix = affix->name + " ";
                } else if (affix->type == "suffix" && suffix.empty()) {
                    suffix = " " + affix->name;
                }
            }
        }
        
        name = prefix + item.type + suffix;
        
        // Capitalize first letter
        if (!name.empty()) {
            name[0] = std::toupper(name[0]);
        }
    }
    
    return name;
}

std::string ItemGenerator::GenerateItemDescription(const GeneratedItem& item) {
    std::stringstream desc;
    
    desc << "A " << QUALITY_TIERS.at(item.qualityTier).first << " quality " << item.type;
    
    if (!item.affixes.empty()) {
        desc << " with special properties";
    }
    
    desc << ".";
    
    return desc.str();
}

std::string ItemGenerator::GenerateItemLore(const GeneratedItem& item) {
    if (item.isUnique) {
        return "This legendary " + item.type + " has a storied history, passed down through generations of heroes.";
    } else if (item.qualityTier >= 4) {
        return "Crafted by master artisans, this " + item.type + " bears the mark of exceptional quality.";
    } else {
        return "A well-made " + item.type + " suitable for adventuring.";
    }
}

bool ItemGenerator::ValidateItem(const GeneratedItem& item, const Player& player) {
    // Basic validation
    if (item.name.empty() || item.category.empty() || item.type.empty()) {
        return false;
    }
    
    if (item.qualityTier < 1 || item.qualityTier > 5) {
        return false;
    }
    
    if (item.itemLevel < 1 || item.itemLevel > 100) {
        return false;
    }
    
    return true;
}

void ItemGenerator::BalanceItemStats(GeneratedItem& item, const Player& player) {
    // Calculate total stat budget
    float budget = CalculateStatBudget(item.itemLevel, item.qualityTier);
    
    // Calculate current stat value
    float currentValue = 0.0f;
    for (const auto& [stat, value] : item.baseStats) {
        currentValue += value * STAT_WEIGHTS.at(stat);
    }
    for (const auto& [stat, value] : item.bonusStats) {
        auto weightIt = STAT_WEIGHTS.find(stat);
        float weight = (weightIt != STAT_WEIGHTS.end()) ? weightIt->second : 0.5f;
        currentValue += value * weight;
    }
    
    // Adjust if over budget
    if (currentValue > budget * 1.2f) {
        float scaleFactor = budget / currentValue;
        
        // Scale down bonus stats
        for (auto& [stat, value] : item.bonusStats) {
            value = static_cast<int>(value * scaleFactor);
        }
    }
}

float ItemGenerator::CalculateItemPower(const GeneratedItem& item) {
    float power = 0.0f;
    
    // Sum weighted stats
    for (const auto& [stat, value] : item.baseStats) {
        auto weightIt = STAT_WEIGHTS.find(stat);
        float weight = (weightIt != STAT_WEIGHTS.end()) ? weightIt->second : 0.5f;
        power += value * weight;
    }
    
    for (const auto& [stat, value] : item.bonusStats) {
        auto weightIt = STAT_WEIGHTS.find(stat);
        float weight = (weightIt != STAT_WEIGHTS.end()) ? weightIt->second : 0.5f;
        power += value * weight;
    }
    
    // Apply quality multiplier
    power *= QUALITY_TIERS.at(item.qualityTier).second;
    
    return power;
}

float ItemGenerator::CalculateUniquenessScore(const GeneratedItem& item) {
    float uniqueness = 0.0f;
    
    // Base uniqueness from quality
    uniqueness += item.qualityTier * 0.2f;
    
    // Uniqueness from affixes
    uniqueness += item.affixes.size() * 0.15f;
    
    // Uniqueness from special properties
    if (item.isUnique) uniqueness += 1.0f;
    if (item.isSetItem) uniqueness += 0.8f;
    
    return std::clamp(uniqueness, 0.0f, 1.0f);
}

float ItemGenerator::CalculateStatBudget(int itemLevel, int qualityTier) {
    float baseBudget = itemLevel * 2.0f;
    float qualityMultiplier = QUALITY_TIERS.at(qualityTier).second;
    return baseBudget * qualityMultiplier;
}

int ItemGenerator::CalculateOptimalQualityTier(int playerLevel) {
    if (playerLevel < 5) return 1;
    if (playerLevel < 15) return 2;
    if (playerLevel < 25) return 3;
    if (playerLevel < 35) return 4;
    return 5;
}

const ItemAffix* ItemGenerator::GetAffix(const std::string& affixId) const {
    auto it = affixes_.find(affixId);
    return (it != affixes_.end()) ? &it->second : nullptr;
}

std::string ItemGenerator::SelectRandomElement(const std::vector<std::string>& elements) {
    if (elements.empty()) return "";
    
    std::uniform_int_distribution<size_t> dist(0, elements.size() - 1);
    return elements[dist(rng_)];
}

int ItemGenerator::GetRandomInRange(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng_);
}

std::string ItemGenerator::GenerateItemId() {
    static uint32_t itemCounter = 1;
    std::stringstream ss;
    ss << "ai_item_" << std::setfill('0') << std::setw(6) << itemCounter++;
    return ss.str();
}

void ItemGenerator::ResetStats() {
    stats_ = GenerationStats{};
}

} // namespace devilution::ai