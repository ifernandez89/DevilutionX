#pragma once

#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>

#include "player.h"
#include "monster.h"

namespace devilution::ai {

/**
 * @brief Player performance metrics for difficulty analysis
 */
struct PlayerPerformanceMetrics {
    // Combat metrics
    float averageDamagePerSecond = 0.0f;
    float damageReceived = 0.0f;
    float healingUsed = 0.0f;
    int deathCount = 0;
    int monstersKilled = 0;
    
    // Time metrics
    float averageLevelCompletionTime = 0.0f;
    float totalPlayTime = 0.0f;
    float idleTime = 0.0f;
    
    // Resource management
    float manaEfficiency = 0.0f;
    float healthPotionUsage = 0.0f;
    float manaPotionUsage = 0.0f;
    
    // Skill usage
    std::unordered_map<int, int> spellUsageCount;
    float averageSpellAccuracy = 0.0f;
    
    // Movement and positioning
    float averageMovementSpeed = 0.0f;
    int backtrackingCount = 0;
    float positioningScore = 0.0f;
    
    // Quest and exploration
    int questsCompleted = 0;
    int questsFailed = 0;
    float explorationPercentage = 0.0f;
    
    // Equipment and progression
    int itemsFound = 0;
    int itemsEquipped = 0;
    float gearScore = 0.0f;
    
    // Timestamp
    std::chrono::steady_clock::time_point lastUpdate;
};

/**
 * @brief Difficulty adjustment recommendation
 */
struct DifficultyAdjustment {
    enum class Type {
        NONE,           // No adjustment needed
        INCREASE_MINOR, // Slightly increase difficulty
        INCREASE_MAJOR, // Significantly increase difficulty
        DECREASE_MINOR, // Slightly decrease difficulty
        DECREASE_MAJOR, // Significantly decrease difficulty
        ADAPTIVE_SCALE  // Use adaptive scaling
    };
    
    Type adjustmentType = Type::NONE;
    float confidenceLevel = 0.0f;  // 0.0 to 1.0
    float suggestedMultiplier = 1.0f; // Difficulty multiplier
    
    // Specific adjustments
    float monsterHealthMultiplier = 1.0f;
    float monsterDamageMultiplier = 1.0f;
    float experienceMultiplier = 1.0f;
    float lootQualityMultiplier = 1.0f;
    
    std::string reasoning;
    std::vector<std::string> specificRecommendations;
};

/**
 * @brief Player skill level assessment
 */
enum class PlayerSkillLevel {
    BEGINNER,       // New to the game
    NOVICE,         // Basic understanding
    INTERMEDIATE,   // Competent player
    ADVANCED,       // Skilled player
    EXPERT,         // Highly skilled
    MASTER          // Exceptional skill
};

/**
 * @brief Difficulty Analyzer for adaptive gameplay
 * 
 * Analyzes player performance in real-time and provides
 * recommendations for dynamic difficulty adjustment to
 * maintain optimal challenge and engagement.
 */
class DifficultyAnalyzer {
public:
    DifficultyAnalyzer();
    ~DifficultyAnalyzer();
    
    // Initialization
    bool Initialize();
    void Reset();
    
    // Performance tracking
    void UpdatePlayerMetrics(const Player& player);
    void RecordCombatEvent(const Player& player, const Monster* monster, float damage, bool playerDealt);
    void RecordDeath(const Player& player, const Monster* killer);
    void RecordLevelCompletion(const Player& player, int level, float completionTime);
    void RecordQuestCompletion(const Player& player, bool success);
    void RecordItemEvent(const Player& player, bool found, bool equipped);
    void RecordSpellCast(const Player& player, int spellId, bool hit);
    void RecordMovement(const Player& player, Point newPosition);
    
    // Analysis and recommendations
    DifficultyAdjustment AnalyzeDifficulty(const Player& player);
    PlayerSkillLevel AssessPlayerSkill(const Player& player);
    float CalculateOptimalDifficulty(const Player& player);
    
    // Difficulty prediction
    float PredictPlayerPerformance(const Player& player, float difficultyMultiplier);
    bool ShouldAdjustDifficulty(const Player& player);
    
    // Configuration
    void SetAnalysisInterval(float seconds) { analysisInterval_ = seconds; }
    void SetAdaptiveMode(bool enabled) { adaptiveMode_ = enabled; }
    void SetSensitivity(float sensitivity) { sensitivity_ = std::clamp(sensitivity, 0.1f, 2.0f); }
    void SetMinConfidenceThreshold(float threshold) { minConfidenceThreshold_ = threshold; }
    
    // Metrics access
    const PlayerPerformanceMetrics& GetCurrentMetrics() const { return currentMetrics_; }
    const PlayerPerformanceMetrics& GetHistoricalAverage() const { return historicalAverage_; }
    
    // Statistics
    struct AnalysisStats {
        uint32_t totalAnalyses = 0;
        uint32_t adjustmentRecommendations = 0;
        uint32_t majorAdjustments = 0;
        uint32_t minorAdjustments = 0;
        float averageConfidence = 0.0f;
        float averageDifficultyMultiplier = 1.0f;
        std::unordered_map<PlayerSkillLevel, uint32_t> skillLevelHistory;
    };
    
    const AnalysisStats& GetStats() const { return stats_; }
    void ResetStats();
    
private:
    // Metrics storage
    PlayerPerformanceMetrics currentMetrics_;
    PlayerPerformanceMetrics historicalAverage_;
    std::vector<PlayerPerformanceMetrics> metricsHistory_;
    
    // Analysis configuration
    float analysisInterval_ = 30.0f; // seconds
    bool adaptiveMode_ = true;
    float sensitivity_ = 1.0f;
    float minConfidenceThreshold_ = 0.6f;
    
    // Tracking state
    std::chrono::steady_clock::time_point lastAnalysis_;
    std::chrono::steady_clock::time_point sessionStart_;
    Point lastPlayerPosition_;
    float totalDistance_ = 0.0f;
    
    // Statistics
    AnalysisStats stats_;
    
    // Analysis methods
    float AnalyzeCombatPerformance(const Player& player);
    float AnalyzeProgressionRate(const Player& player);
    float AnalyzeResourceManagement(const Player& player);
    float AnalyzeSkillUsage(const Player& player);
    float AnalyzeMovementEfficiency(const Player& player);
    
    // Skill assessment helpers
    PlayerSkillLevel DetermineSkillFromMetrics(const PlayerPerformanceMetrics& metrics);
    float CalculateSkillScore(const PlayerPerformanceMetrics& metrics);
    
    // Difficulty calculation
    float CalculateBaseDifficulty(const Player& player);
    float ApplySkillModifier(float baseDifficulty, PlayerSkillLevel skillLevel);
    float ApplyPerformanceModifier(float difficulty, const PlayerPerformanceMetrics& metrics);
    
    // Utility methods
    void UpdateHistoricalAverage();
    void PruneOldMetrics();
    float CalculateConfidence(const DifficultyAdjustment& adjustment);
    std::string GenerateReasoningText(const DifficultyAdjustment& adjustment, const Player& player);
    
    // Constants
    static constexpr size_t MAX_METRICS_HISTORY = 100;
    static constexpr float SKILL_ASSESSMENT_WEIGHT = 0.4f;
    static constexpr float PERFORMANCE_WEIGHT = 0.6f;
    static constexpr float MIN_DIFFICULTY_MULTIPLIER = 0.5f;
    static constexpr float MAX_DIFFICULTY_MULTIPLIER = 3.0f;
};

} // namespace devilution::ai