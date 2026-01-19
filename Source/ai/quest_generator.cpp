#include "ai/quest_generator.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"
#include "engine/random.hpp"

namespace devilution::ai {

namespace {

// Quest title fragments for procedural generation
const std::vector<std::string> TITLE_PREFIXES = {
    "The", "A", "An", "Ancient", "Lost", "Forgotten", "Hidden", "Sacred", "Cursed", "Mysterious",
    "Dark", "Bright", "Final", "First", "Last", "Secret", "Forbidden", "Divine", "Infernal"
};

const std::vector<std::string> TITLE_NOUNS = {
    "Quest", "Journey", "Trial", "Challenge", "Mission", "Task", "Duty", "Calling", "Destiny",
    "Path", "Way", "Road", "Adventure", "Expedition", "Hunt", "Search", "Discovery", "Revelation"
};

const std::vector<std::string> TITLE_OBJECTS = {
    "Artifact", "Relic", "Treasure", "Secret", "Truth", "Power", "Knowledge", "Wisdom", "Key",
    "Crown", "Sword", "Shield", "Tome", "Scroll", "Crystal", "Gem", "Stone", "Ring", "Amulet"
};

// Lore fragments for quest descriptions
const std::vector<std::string> LORE_FRAGMENTS = {
    "In the depths of the ancient catacombs",
    "Legend speaks of a time when",
    "The old texts mention",
    "Scholars have long debated",
    "It is said that in ages past",
    "The wise ones foretold",
    "Ancient prophecies speak of",
    "In the shadow of the great cathedral",
    "Beyond the veil of mortal understanding",
    "Where darkness meets the light"
};

// Monster type mappings for kill objectives
const std::unordered_map<std::string, std::vector<std::string>> MONSTER_CATEGORIES = {
    {"undead", {"skeleton", "zombie", "wraith", "lich", "bone_demon"}},
    {"demons", {"fallen", "gargoyle", "succubus", "balrog", "diablo"}},
    {"beasts", {"scavenger", "cave_viper", "acid_beast", "poison_spitter"}},
    {"humanoids", {"rogue", "archer", "mage", "warrior", "assassin"}},
    {"constructs", {"golem", "automaton", "guardian", "sentinel"}}
};

} // anonymous namespace

QuestGenerator::QuestGenerator() 
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()) {
}

QuestGenerator::~QuestGenerator() = default;

bool QuestGenerator::Initialize() {
    LogVerbose("Initializing AI Quest Generator...");
    
    // Load default templates
    LoadDefaultTemplates();
    LoadDefaultLore();
    
    LogVerbose("Quest Generator initialized with {} templates", templates_.size());
    return true;
}

void QuestGenerator::LoadDefaultTemplates() {
    // Kill Monsters Template
    QuestTemplate killTemplate;
    killTemplate.templateId = "kill_monsters_basic";
    killTemplate.category = "combat";
    killTemplate.baseObjective = QuestObjectiveType::KILL_MONSTERS;
    killTemplate.titleVariations = {
        "Cleanse the {location}",
        "Purge the {monster_type}",
        "Hunt the {monster_name}",
        "Eliminate the Threat",
        "The {monster_type} Menace"
    };
    killTemplate.descriptionTemplates = {
        "The {location} has been overrun by {monster_type}. Clear them out and restore peace.",
        "A dangerous {monster_name} has been terrorizing the area. Hunt it down.",
        "The local population is threatened by {monster_type}. Eliminate {kill_count} of them."
    };
    killTemplate.baseRelevanceScore = 0.8f;
    
    // Set difficulty scaling
    killTemplate.difficultyScaling[QuestDifficulty::EASY] = {1, 10, 0.8f, 0.8f, 0.7f};
    killTemplate.difficultyScaling[QuestDifficulty::NORMAL] = {5, 20, 1.0f, 1.0f, 1.0f};
    killTemplate.difficultyScaling[QuestDifficulty::HARD] = {15, 35, 1.3f, 1.2f, 1.4f};
    killTemplate.difficultyScaling[QuestDifficulty::EXTREME] = {25, 50, 1.8f, 1.5f, 2.0f};
    
    templates_[killTemplate.templateId] = killTemplate;
    
    // Collect Items Template
    QuestTemplate collectTemplate;
    collectTemplate.templateId = "collect_items_basic";
    collectTemplate.category = "exploration";
    collectTemplate.baseObjective = QuestObjectiveType::COLLECT_ITEMS;
    collectTemplate.titleVariations = {
        "Gather the {item_type}",
        "The Lost {item_name}",
        "Retrieve the {item_type}",
        "Collection Quest",
        "The Missing {item_name}"
    };
    collectTemplate.descriptionTemplates = {
        "Collect {item_count} {item_type} from the {location}.",
        "A valuable {item_name} has been lost. Find and retrieve it.",
        "The local alchemist needs {item_count} {item_type} for an important ritual."
    };
    collectTemplate.baseRelevanceScore = 0.6f;
    
    collectTemplate.difficultyScaling[QuestDifficulty::EASY] = {1, 8, 0.7f, 0.9f, 0.8f};
    collectTemplate.difficultyScaling[QuestDifficulty::NORMAL] = {3, 15, 1.0f, 1.0f, 1.0f};
    collectTemplate.difficultyScaling[QuestDifficulty::HARD] = {10, 25, 1.4f, 1.1f, 1.3f};
    
    templates_[collectTemplate.templateId] = collectTemplate;
    
    // Exploration Template
    QuestTemplate exploreTemplate;
    exploreTemplate.templateId = "explore_area_basic";
    exploreTemplate.category = "exploration";
    exploreTemplate.baseObjective = QuestObjectiveType::EXPLORE_AREA;
    exploreTemplate.titleVariations = {
        "Explore the {location}",
        "Map the {area_name}",
        "Scout the {location}",
        "Venture into {area_name}",
        "The Unknown {location}"
    };
    exploreTemplate.descriptionTemplates = {
        "Venture into the unexplored {location} and map the area.",
        "Strange reports come from {area_name}. Investigate and report back.",
        "The {location} holds secrets. Explore every corner and uncover the truth."
    };
    exploreTemplate.baseRelevanceScore = 0.5f;
    
    exploreTemplate.difficultyScaling[QuestDifficulty::EASY] = {1, 5, 0.6f, 1.0f, 0.5f};
    exploreTemplate.difficultyScaling[QuestDifficulty::NORMAL] = {3, 12, 1.0f, 1.0f, 1.0f};
    exploreTemplate.difficultyScaling[QuestDifficulty::HARD] = {8, 20, 1.5f, 1.0f, 1.5f};
    
    templates_[exploreTemplate.templateId] = exploreTemplate;
}

void QuestGenerator::LoadDefaultLore() {
    loreDatabase_ = LORE_FRAGMENTS;
    
    // Add more lore specific to Diablo universe
    loreDatabase_.insert(loreDatabase_.end(), {
        "The Horadrim once sealed great evils here",
        "Diablo's influence still lingers in these halls",
        "The ancient Zakarum built this sanctuary",
        "Mephisto's corruption spreads through the land",
        "Baal's minions have awakened from their slumber",
        "The Soulstones' power calls to the darkness",
        "Tyrael's light once blessed this sacred ground",
        "The Prime Evils' return draws near",
        "Deckard Cain spoke of this prophecy",
        "The Worldstone's destruction changed everything"
    });
}

std::unique_ptr<GeneratedQuest> QuestGenerator::GenerateQuest(
    const Player& player,
    QuestDifficulty targetDifficulty,
    const std::string& preferredCategory) {
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Select appropriate template
    const QuestTemplate* selectedTemplate = SelectTemplate(player, targetDifficulty, preferredCategory);
    if (!selectedTemplate) {
        LogError("Failed to select quest template");
        stats_.failedValidations++;
        return nullptr;
    }
    
    // Create quest structure
    auto quest = std::make_unique<GeneratedQuest>();
    quest->id = GenerateQuestId();
    quest->primaryObjective = selectedTemplate->baseObjective;
    quest->difficulty = targetDifficulty;
    quest->recommendedLevel = player.getCharacterLevel();
    
    // Generate quest content
    GenerateQuestContent(*quest, *selectedTemplate, player);
    GenerateObjectives(*quest, *selectedTemplate, player);
    GenerateRewards(*quest, player);
    GenerateConditions(*quest, player);
    
    // Validate and balance
    if (!ValidateQuest(*quest, player)) {
        LogWarn("Generated quest failed validation");
        stats_.failedValidations++;
        return nullptr;
    }
    
    BalanceQuestRewards(*quest, player);
    quest->relevanceScore = CalculateQuestRelevance(*quest, player);
    
    // Update statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    stats_.totalGenerated++;
    stats_.successfulGenerations++;
    stats_.averageGenerationTime = (stats_.averageGenerationTime * (stats_.totalGenerated - 1) + 
                                   duration.count() / 1000.0f) / stats_.totalGenerated;
    stats_.averageRelevanceScore = (stats_.averageRelevanceScore * (stats_.totalGenerated - 1) + 
                                   quest->relevanceScore) / stats_.totalGenerated;
    stats_.objectiveTypeCount[quest->primaryObjective]++;
    stats_.difficultyCount[quest->difficulty]++;
    
    quest->generationTime = static_cast<uint32_t>(duration.count());
    
    LogVerbose("Generated quest '{}' (relevance: {:.2f}, time: {:.2f}ms)", 
               quest->title, quest->relevanceScore, duration.count() / 1000.0f);
    
    return quest;
}

const QuestTemplate* QuestGenerator::SelectTemplate(
    const Player& player, 
    QuestDifficulty targetDifficulty, 
    const std::string& preferredCategory) {
    
    std::vector<const QuestTemplate*> candidates;
    
    for (const auto& [id, questTemplate] : templates_) {
        // Check category preference
        if (!preferredCategory.empty() && questTemplate.category != preferredCategory) {
            continue;
        }
        
        // Check difficulty scaling availability
        if (questTemplate.difficultyScaling.find(targetDifficulty) == questTemplate.difficultyScaling.end()) {
            continue;
        }
        
        // Check level requirements
        const auto& scaling = questTemplate.difficultyScaling.at(targetDifficulty);
        if (player.getCharacterLevel() < scaling.minLevel || player.getCharacterLevel() > scaling.maxLevel) {
            continue;
        }
        
        candidates.push_back(&questTemplate);
    }
    
    if (candidates.empty()) {
        return nullptr;
    }
    
    // Select based on relevance and randomness
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(rng_)];
}

void QuestGenerator::GenerateQuestContent(
    GeneratedQuest& quest, 
    const QuestTemplate& questTemplate, 
    const Player& player) {
    
    // Generate title
    quest.title = GenerateQuestTitle(questTemplate, player);
    
    // Generate description
    quest.description = GenerateQuestDescription(questTemplate, quest);
    
    // Generate lore
    quest.lore = GenerateQuestLore(questTemplate, quest);
    
    // Set basic properties
    quest.targetLevel = std::max(1, player.getCharacterLevel() + static_cast<int>(quest.difficulty) - 2);
    quest.isRepeatable = (questTemplate.baseObjective == QuestObjectiveType::KILL_MONSTERS);
    quest.isTimeLimited = false;
}

std::string QuestGenerator::GenerateQuestTitle(const QuestTemplate& questTemplate, const Player& player) {
    if (questTemplate.titleVariations.empty()) {
        return "Generated Quest";
    }
    
    std::uniform_int_distribution<size_t> dist(0, questTemplate.titleVariations.size() - 1);
    std::string titleTemplate = questTemplate.titleVariations[dist(rng_)];
    
    // Replace placeholders
    std::unordered_map<std::string, std::string> variables;
    variables["location"] = SelectRandomElement({"Catacombs", "Caves", "Dungeon", "Labyrinth", "Crypts"});
    variables["monster_type"] = SelectRandomElement({"Skeletons", "Demons", "Undead", "Beasts", "Fallen"});
    variables["monster_name"] = SelectRandomElement({"Bone Demon", "Shadow Beast", "Cursed One", "Dark Lord"});
    variables["area_name"] = SelectRandomElement({"Dark Passage", "Forgotten Halls", "Ancient Chamber"});
    variables["item_type"] = SelectRandomElement({"Crystals", "Scrolls", "Artifacts", "Relics", "Gems"});
    variables["item_name"] = SelectRandomElement({"Sacred Relic", "Ancient Tome", "Mystic Crystal"});
    
    return ProcessTemplate(titleTemplate, variables);
}

std::string QuestGenerator::GenerateQuestDescription(const QuestTemplate& questTemplate, const GeneratedQuest& quest) {
    if (questTemplate.descriptionTemplates.empty()) {
        return "A quest has been generated for you.";
    }
    
    std::uniform_int_distribution<size_t> dist(0, questTemplate.descriptionTemplates.size() - 1);
    std::string descTemplate = questTemplate.descriptionTemplates[dist(rng_)];
    
    // Replace placeholders with quest-specific data
    std::unordered_map<std::string, std::string> variables;
    variables["location"] = "the depths below";
    variables["monster_type"] = "hostile creatures";
    variables["kill_count"] = std::to_string(5 + static_cast<int>(quest.difficulty) * 3);
    variables["item_count"] = std::to_string(2 + static_cast<int>(quest.difficulty));
    
    return ProcessTemplate(descTemplate, variables);
}

std::string QuestGenerator::GenerateQuestLore(const QuestTemplate& questTemplate, const GeneratedQuest& quest) {
    if (loreDatabase_.empty()) {
        return "";
    }
    
    std::string lore = SelectRandomElement(loreDatabase_);
    lore += ", ";
    
    // Add quest-specific lore continuation
    switch (quest.primaryObjective) {
        case QuestObjectiveType::KILL_MONSTERS:
            lore += "evil creatures have awakened and threaten the innocent.";
            break;
        case QuestObjectiveType::COLLECT_ITEMS:
            lore += "sacred artifacts were scattered and must be recovered.";
            break;
        case QuestObjectiveType::EXPLORE_AREA:
            lore += "hidden passages await discovery by brave souls.";
            break;
        default:
            lore += "a great challenge awaits those who dare to accept it.";
            break;
    }
    
    return lore;
}

void QuestGenerator::GenerateObjectives(GeneratedQuest& quest, const QuestTemplate& questTemplate, const Player& player) {
    // Primary objective is already set
    
    // Generate secondary objectives based on difficulty
    int secondaryCount = static_cast<int>(quest.difficulty) / 2;
    
    std::vector<QuestObjectiveType> possibleSecondary = {
        QuestObjectiveType::COLLECT_ITEMS,
        QuestObjectiveType::EXPLORE_AREA,
        QuestObjectiveType::SURVIVE_TIME
    };
    
    std::shuffle(possibleSecondary.begin(), possibleSecondary.end(), rng_);
    
    for (int i = 0; i < secondaryCount && i < static_cast<int>(possibleSecondary.size()); ++i) {
        if (possibleSecondary[i] != quest.primaryObjective) {
            quest.secondaryObjectives.push_back(possibleSecondary[i]);
        }
    }
}

void QuestGenerator::GenerateRewards(GeneratedQuest& quest, const Player& player) {
    const auto& scaling = templates_[quest.primaryObjective == QuestObjectiveType::KILL_MONSTERS ? 
                                   "kill_monsters_basic" : "collect_items_basic"]
                         .difficultyScaling.at(quest.difficulty);
    
    // Base rewards scaled by level and difficulty
    int baseExp = player.getCharacterLevel() * 100;
    int baseGold = player.getCharacterLevel() * 50;
    
    quest.experienceReward = static_cast<int>(baseExp * scaling.experienceMultiplier);
    quest.goldReward = static_cast<int>(baseGold * scaling.goldMultiplier);
    
    // Estimated duration
    quest.estimatedDuration = static_cast<int>(15 * scaling.durationMultiplier);
    
    // Item rewards (simplified for now)
    if (quest.difficulty >= QuestDifficulty::HARD) {
        quest.itemRewards.push_back("Magic Item");
    }
    if (quest.difficulty >= QuestDifficulty::EXTREME) {
        quest.itemRewards.push_back("Rare Item");
    }
}

void QuestGenerator::GenerateConditions(GeneratedQuest& quest, const Player& player) {
    // Prerequisites based on level
    if (quest.recommendedLevel > 5) {
        quest.prerequisites.push_back("Minimum level " + std::to_string(quest.recommendedLevel - 2));
    }
    
    // Unlocks based on quest type
    switch (quest.primaryObjective) {
        case QuestObjectiveType::KILL_MONSTERS:
            quest.unlocks.push_back("Access to deeper levels");
            break;
        case QuestObjectiveType::EXPLORE_AREA:
            quest.unlocks.push_back("New area discovered");
            break;
        default:
            break;
    }
}

std::string QuestGenerator::GenerateQuestId() {
    static uint32_t questCounter = 1;
    std::stringstream ss;
    ss << "ai_quest_" << std::setfill('0') << std::setw(6) << questCounter++;
    return ss.str();
}

bool QuestGenerator::ValidateQuest(const GeneratedQuest& quest, const Player& player) {
    // Basic validation checks
    if (quest.title.empty() || quest.description.empty()) {
        return false;
    }
    
    if (quest.experienceReward <= 0 || quest.goldReward < 0) {
        return false;
    }
    
    if (quest.estimatedDuration <= 0 || quest.estimatedDuration > 180) { // Max 3 hours
        return false;
    }
    
    return true;
}

void QuestGenerator::BalanceQuestRewards(GeneratedQuest& quest, const Player& player) {
    // Ensure rewards are reasonable for player level
    int maxExp = player.getCharacterLevel() * 500;
    int maxGold = player.getCharacterLevel() * 200;
    
    quest.experienceReward = std::min(quest.experienceReward, maxExp);
    quest.goldReward = std::min(quest.goldReward, maxGold);
    
    // Ensure minimum rewards
    quest.experienceReward = std::max(quest.experienceReward, player.getCharacterLevel() * 25);
    quest.goldReward = std::max(quest.goldReward, player.getCharacterLevel() * 10);
}

float QuestGenerator::CalculateQuestRelevance(const GeneratedQuest& quest, const Player& player) {
    float relevance = 0.5f; // Base relevance
    
    // Level appropriateness
    int levelDiff = std::abs(quest.recommendedLevel - player.getCharacterLevel());
    if (levelDiff <= 2) {
        relevance += 0.3f;
    } else if (levelDiff <= 5) {
        relevance += 0.1f;
    } else {
        relevance -= 0.2f;
    }
    
    // Difficulty preference (assume normal is preferred)
    if (quest.difficulty == QuestDifficulty::NORMAL) {
        relevance += 0.2f;
    } else if (quest.difficulty == QuestDifficulty::EASY || quest.difficulty == QuestDifficulty::HARD) {
        relevance += 0.1f;
    }
    
    return std::clamp(relevance, 0.0f, 1.0f);
}

std::string QuestGenerator::SelectRandomElement(const std::vector<std::string>& elements) {
    if (elements.empty()) {
        return "";
    }
    
    std::uniform_int_distribution<size_t> dist(0, elements.size() - 1);
    return elements[dist(rng_)];
}

std::string QuestGenerator::ProcessTemplate(const std::string& templateStr, const std::unordered_map<std::string, std::string>& variables) {
    std::string result = templateStr;
    
    for (const auto& [key, value] : variables) {
        std::string placeholder = "{" + key + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    
    return result;
}

void QuestGenerator::ResetStats() {
    stats_ = GenerationStats{};
}

} // namespace devilution::ai