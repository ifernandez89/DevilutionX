#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <random>

#include "quests.h"
#include "player.h"

namespace devilution::ai {

/**
 * @brief Quest objective types for procedural generation
 */
enum class QuestObjectiveType {
    KILL_MONSTERS,      // Kill X monsters of type Y
    COLLECT_ITEMS,      // Collect X items of type Y
    EXPLORE_AREA,       // Explore specific area/level
    DELIVER_ITEM,       // Deliver item to NPC
    SURVIVE_TIME,       // Survive for X minutes
    REACH_LEVEL,        // Reach specific character level
    FIND_LOCATION,      // Find hidden location
    PROTECT_NPC,        // Protect NPC from monsters
    SOLVE_PUZZLE,       // Solve environmental puzzle
    COMBO_OBJECTIVE     // Multiple objectives combined
};

/**
 * @brief Quest difficulty levels
 */
enum class QuestDifficulty {
    TRIVIAL,    // Very easy, tutorial-like
    EASY,       // Below player level
    NORMAL,     // At player level
    HARD,       // Above player level
    EXTREME,    // Significantly challenging
    NIGHTMARE   // Maximum difficulty
};

/**
 * @brief Generated quest structure
 */
struct GeneratedQuest {
    std::string id;
    std::string title;
    std::string description;
    std::string lore;
    
    QuestObjectiveType primaryObjective;
    std::vector<QuestObjectiveType> secondaryObjectives;
    
    QuestDifficulty difficulty;
    int recommendedLevel;
    int estimatedDuration; // in minutes
    
    // Rewards
    int experienceReward;
    int goldReward;
    std::vector<std::string> itemRewards;
    
    // Context
    int targetLevel;        // Dungeon level where quest takes place
    std::string targetNPC;  // NPC associated with quest
    std::string questGiver; // Who gives the quest
    
    // Conditions
    std::vector<std::string> prerequisites;
    std::vector<std::string> unlocks;
    
    // Metadata
    uint32_t generationTime;
    float relevanceScore;   // How relevant to current game state
    bool isRepeatable;
    bool isTimeLimited;
    int timeLimit;          // in game hours
};

/**
 * @brief Quest template for procedural generation
 */
struct QuestTemplate {
    std::string templateId;
    std::string category;
    QuestObjectiveType baseObjective;
    
    std::vector<std::string> titleVariations;
    std::vector<std::string> descriptionTemplates;
    std::vector<std::string> loreFragments;
    
    struct DifficultyScaling {
        int minLevel;
        int maxLevel;
        float experienceMultiplier;
        float goldMultiplier;
        float durationMultiplier;
    };
    
    std::unordered_map<QuestDifficulty, DifficultyScaling> difficultyScaling;
    
    // Generation parameters
    float baseRelevanceScore;
    std::vector<std::string> requiredGameStates;
    std::vector<std::string> incompatibleQuests;
};

/**
 * @brief Procedural Quest Generator
 * 
 * Generates dynamic quests based on:
 * - Player progress and level
 * - Current game state
 * - Available NPCs and locations
 * - Player behavior patterns
 * - Difficulty preferences
 */
class QuestGenerator {
public:
    QuestGenerator();
    ~QuestGenerator();
    
    // Initialization
    bool Initialize();
    void LoadTemplates(const std::string& templateFile);
    void LoadLoreDatabase(const std::string& loreFile);
    
    // Quest generation
    std::unique_ptr<GeneratedQuest> GenerateQuest(
        const Player& player,
        QuestDifficulty targetDifficulty = QuestDifficulty::NORMAL,
        const std::string& preferredCategory = ""
    );
    
    std::vector<std::unique_ptr<GeneratedQuest>> GenerateQuestBatch(
        const Player& player,
        int count = 3,
        bool diverseObjectives = true
    );
    
    // Context-aware generation
    std::unique_ptr<GeneratedQuest> GenerateContextualQuest(
        const Player& player,
        const std::string& currentLocation,
        const std::vector<std::string>& availableNPCs,
        const std::vector<std::string>& recentEvents
    );
    
    // Quest validation and balancing
    bool ValidateQuest(const GeneratedQuest& quest, const Player& player);
    void BalanceQuestRewards(GeneratedQuest& quest, const Player& player);
    float CalculateQuestRelevance(const GeneratedQuest& quest, const Player& player);
    
    // Template management
    void AddTemplate(const QuestTemplate& questTemplate);
    void RemoveTemplate(const std::string& templateId);
    const QuestTemplate* GetTemplate(const std::string& templateId) const;
    std::vector<std::string> GetAvailableCategories() const;
    
    // Configuration
    void SetGenerationSeed(uint32_t seed);
    void SetDifficultyBias(float bias); // -1.0 (easier) to 1.0 (harder)
    void SetCreativityLevel(float level); // 0.0 (template-based) to 1.0 (highly creative)
    void SetRelevanceThreshold(float threshold);
    
    // Statistics
    struct GenerationStats {
        uint32_t totalGenerated = 0;
        uint32_t successfulGenerations = 0;
        uint32_t failedValidations = 0;
        float averageRelevanceScore = 0.0f;
        float averageGenerationTime = 0.0f;
        std::unordered_map<QuestObjectiveType, uint32_t> objectiveTypeCount;
        std::unordered_map<QuestDifficulty, uint32_t> difficultyCount;
    };
    
    const GenerationStats& GetStats() const { return stats_; }
    void ResetStats();
    
private:
    // Template system
    std::unordered_map<std::string, QuestTemplate> templates_;
    std::vector<std::string> loreDatabase_;
    std::vector<std::string> nameFragments_;
    
    // Generation state
    std::mt19937 rng_;
    float difficultyBias_ = 0.0f;
    float creativityLevel_ = 0.5f;
    float relevanceThreshold_ = 0.3f;
    
    // Statistics
    GenerationStats stats_;
    
    // Internal generation methods
    const QuestTemplate* SelectTemplate(const Player& player, QuestDifficulty targetDifficulty, const std::string& preferredCategory);
    void LoadDefaultTemplates();
    void LoadDefaultLore();
    void GenerateQuestContent(GeneratedQuest& quest, const QuestTemplate& questTemplate, const Player& player);
    std::string GenerateQuestId();
    std::string GenerateQuestTitle(const QuestTemplate& questTemplate, const Player& player);
    std::string GenerateQuestDescription(const QuestTemplate& questTemplate, const GeneratedQuest& quest);
    std::string GenerateQuestLore(const QuestTemplate& questTemplate, const GeneratedQuest& quest);
    
    void GenerateObjectives(GeneratedQuest& quest, const QuestTemplate& questTemplate, const Player& player);
    void GenerateRewards(GeneratedQuest& quest, const Player& player);
    void GenerateConditions(GeneratedQuest& quest, const Player& player);
    
    // Utility methods
    QuestDifficulty CalculateOptimalDifficulty(const Player& player);
    std::string SelectRandomElement(const std::vector<std::string>& elements);
    std::string ProcessTemplate(const std::string& templateStr, const std::unordered_map<std::string, std::string>& variables);
    
    // Validation helpers
    bool IsQuestFeasible(const GeneratedQuest& quest, const Player& player);
    bool AreRewardsBalanced(const GeneratedQuest& quest, const Player& player);
    bool IsObjectiveCombinationValid(const std::vector<QuestObjectiveType>& objectives);
};

} // namespace devilution::ai