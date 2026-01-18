#pragma once

#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>

#include "player.h"

namespace devilution::ai {

/**
 * @brief Player behavior pattern types
 */
enum class BehaviorPattern {
    AGGRESSIVE_COMBAT,    // Rushes into combat, high damage focus
    DEFENSIVE_COMBAT,     // Cautious approach, defensive positioning
    EXPLORATION_FOCUSED,  // Prioritizes exploration over combat
    LOOT_FOCUSED,        // Prioritizes item collection and management
    QUEST_FOCUSED,       // Follows quest objectives efficiently
    SOCIAL_PLAYER,       // Interacts frequently with NPCs
    SPEEDRUNNER,         // Optimizes for fast completion
    COMPLETIONIST,       // Explores everything thoroughly
    CASUAL_PLAYER,       // Relaxed, varied gameplay
    STRATEGIC_PLAYER     // Plans moves carefully, uses tactics
};

/**
 * @brief Player preference profile
 */
struct PlayerPreferences {
    // Combat preferences
    float aggressionLevel = 0.5f;        // 0.0 = very defensive, 1.0 = very aggressive
    float riskTolerance = 0.5f;          // 0.0 = risk-averse, 1.0 = risk-seeking
    float combatComplexity = 0.5f;       // 0.0 = simple, 1.0 = complex tactics
    
    // Exploration preferences
    float explorationThoroughness = 0.5f; // 0.0 = minimal, 1.0 = complete
    float backtrackingTolerance = 0.5f;   // 0.0 = hates backtracking, 1.0 = doesn't mind
    float secretHunting = 0.5f;           // 0.0 = ignores secrets, 1.0 = seeks them out
    
    // Progression preferences
    float questPriority = 0.5f;          // 0.0 = ignores quests, 1.0 = quest-focused
    float levelingSpeed = 0.5f;          // 0.0 = slow and steady, 1.0 = fast leveling
    float gearOptimization = 0.5f;       // 0.0 = uses whatever, 1.0 = optimizes gear
    
    // Social preferences
    float npcInteraction = 0.5f;         // 0.0 = minimal interaction, 1.0 = talks to everyone
    float loreInterest = 0.5f;           // 0.0 = skips lore, 1.0 = reads everything
    
    // Playstyle preferences
    float sessionLength = 0.5f;          // 0.0 = short sessions, 1.0 = long sessions
    float pauseFrequency = 0.5f;         // 0.0 = rarely pauses, 1.0 = pauses often
    float multitasking = 0.5f;           // 0.0 = focused, 1.0 = multitasks
};

/**
 * @brief Behavior tracking data point
 */
struct BehaviorDataPoint {
    std::chrono::steady_clock::time_point timestamp;
    std::string actionType;
    std::string actionData;
    Point playerPosition;
    int playerLevel;
    float sessionTime;
    std::unordered_map<std::string, float> contextData;
};

/**
 * @brief Player session summary
 */
struct SessionSummary {
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point endTime;
    float duration; // in seconds
    
    // Activity counts
    int combatActions = 0;
    int explorationActions = 0;
    int inventoryActions = 0;
    int npcInteractions = 0;
    int questActions = 0;
    int deathCount = 0;
    
    // Movement data
    float totalDistance = 0.0f;
    int backtrackingCount = 0;
    float averageSpeed = 0.0f;
    
    // Performance metrics
    float averageDPS = 0.0f;
    float damageReceived = 0.0f;
    float healingUsed = 0.0f;
    
    // Progression
    int levelsGained = 0;
    int itemsFound = 0;
    int questsCompleted = 0;
    
    // Dominant behavior patterns
    std::vector<BehaviorPattern> dominantPatterns;
    float patternConfidence = 0.0f;
};

/**
 * @brief Player Behavior Tracker
 * 
 * Tracks and analyzes player behavior patterns to:
 * - Understand player preferences and playstyle
 * - Adapt game systems to player behavior
 * - Provide personalized recommendations
 * - Improve AI system responses
 */
class PlayerBehaviorTracker {
public:
    PlayerBehaviorTracker();
    ~PlayerBehaviorTracker();
    
    // Initialization
    bool Initialize();
    void Reset();
    void StartNewSession();
    void EndCurrentSession();
    
    // Behavior tracking
    void TrackAction(const std::string& actionType, const std::string& actionData, const Player& player);
    void TrackCombatAction(const Player& player, const std::string& actionType, float damage = 0.0f);
    void TrackMovement(const Player& player, Point newPosition);
    void TrackInventoryAction(const Player& player, const std::string& actionType);
    void TrackNPCInteraction(const Player& player, const std::string& npcId, const std::string& interactionType);
    void TrackQuestAction(const Player& player, const std::string& questId, const std::string& actionType);
    void TrackDeath(const Player& player, const std::string& cause);
    
    // Analysis and pattern recognition
    std::vector<BehaviorPattern> AnalyzeBehaviorPatterns(int sessionCount = 5);
    PlayerPreferences AnalyzePlayerPreferences();
    BehaviorPattern GetDominantPattern() const;
    float GetPatternConfidence(BehaviorPattern pattern) const;
    
    // Prediction and recommendations
    std::vector<std::string> PredictNextActions(const Player& player, int count = 3);
    std::vector<std::string> GenerateRecommendations(const Player& player);
    float PredictSessionLength(const Player& player);
    bool PredictPlayerFrustration(const Player& player);
    
    // Session management
    const SessionSummary& GetCurrentSession() const { return currentSession_; }
    std::vector<SessionSummary> GetRecentSessions(int count = 10) const;
    SessionSummary GetAverageSession() const;
    
    // Preference access
    const PlayerPreferences& GetPlayerPreferences() const { return playerPreferences_; }
    void UpdatePlayerPreferences(const PlayerPreferences& preferences);
    
    // Configuration
    void SetTrackingEnabled(bool enabled) { trackingEnabled_ = enabled; }
    void SetAnalysisDepth(int depth) { analysisDepth_ = std::clamp(depth, 1, 100); }
    void SetPatternSensitivity(float sensitivity) { patternSensitivity_ = std::clamp(sensitivity, 0.1f, 2.0f); }
    void SetDataRetentionDays(int days) { dataRetentionDays_ = std::max(1, days); }
    
    // Statistics
    struct TrackingStats {
        uint32_t totalActionsTracked = 0;
        uint32_t totalSessions = 0;
        uint32_t totalPlayTime = 0; // in seconds
        float averageSessionLength = 0.0f;
        std::unordered_map<std::string, uint32_t> actionTypeCount;
        std::unordered_map<BehaviorPattern, uint32_t> patternFrequency;
        float averagePatternConfidence = 0.0f;
    };
    
    const TrackingStats& GetStats() const { return stats_; }
    void ResetStats();
    
private:
    // Data storage
    std::vector<BehaviorDataPoint> behaviorData_;
    std::vector<SessionSummary> sessionHistory_;
    SessionSummary currentSession_;
    PlayerPreferences playerPreferences_;
    
    // Tracking state
    bool trackingEnabled_ = true;
    bool sessionActive_ = false;
    Point lastPlayerPosition_;
    std::chrono::steady_clock::time_point sessionStartTime_;
    std::chrono::steady_clock::time_point lastActionTime_;
    
    // Analysis configuration
    int analysisDepth_ = 20;
    float patternSensitivity_ = 1.0f;
    int dataRetentionDays_ = 30;
    
    // Statistics
    TrackingStats stats_;
    
    // Pattern analysis methods
    BehaviorPattern AnalyzeCombatPattern(const std::vector<BehaviorDataPoint>& data);
    BehaviorPattern AnalyzeExplorationPattern(const std::vector<BehaviorDataPoint>& data);
    BehaviorPattern AnalyzeProgressionPattern(const std::vector<BehaviorDataPoint>& data);
    BehaviorPattern AnalyzeSocialPattern(const std::vector<BehaviorDataPoint>& data);
    
    // Preference calculation
    float CalculateAggressionLevel(const std::vector<BehaviorDataPoint>& data);
    float CalculateRiskTolerance(const std::vector<BehaviorDataPoint>& data);
    float CalculateExplorationThoroughness(const std::vector<BehaviorDataPoint>& data);
    float CalculateQuestPriority(const std::vector<BehaviorDataPoint>& data);
    
    // Prediction algorithms
    std::vector<std::string> PredictBasedOnPattern(BehaviorPattern pattern, const Player& player);
    float CalculateFrustrationScore(const Player& player);
    
    // Data management
    void PruneOldData();
    void UpdateCurrentSession(const Player& player);
    void CalculateSessionMetrics();
    
    // Utility methods
    std::vector<BehaviorDataPoint> GetRecentData(int minutes = 30) const;
    std::vector<BehaviorDataPoint> GetDataByType(const std::string& actionType) const;
    float CalculateActionFrequency(const std::string& actionType, int timeWindowMinutes = 10) const;
    
    // Constants
    static constexpr size_t MAX_BEHAVIOR_DATA_POINTS = 10000;
    static constexpr size_t MAX_SESSION_HISTORY = 100;
    static constexpr float PATTERN_CONFIDENCE_THRESHOLD = 0.6f;
};

} // namespace devilution::ai