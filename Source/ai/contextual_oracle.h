#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "player.h"
#include "oracle/oracle_system.h"

namespace devilution::ai {

/**
 * @brief Enhanced Oracle query context
 */
struct OracleQueryContext {
    // Player context
    std::string playerId;
    int playerLevel;
    std::string playerClass;
    Point playerPosition;
    std::string currentLocation;
    
    // Game state context
    std::vector<std::string> activeQuests;
    std::vector<std::string> completedQuests;
    std::vector<std::string> availableNPCs;
    std::vector<std::string> nearbyItems;
    std::vector<std::string> nearbyMonsters;
    
    // Recent events
    std::vector<std::string> recentActions;
    std::vector<std::string> recentDeaths;
    std::vector<std::string> recentLevelUps;
    std::vector<std::string> recentItemFinds;
    
    // Behavioral context
    std::string dominantBehaviorPattern;
    float frustrationLevel;
    float engagementLevel;
    std::vector<std::string> playerPreferences;
    
    // Session context
    float sessionDuration;
    int sessionDeaths;
    int sessionLevelUps;
    std::string sessionGoals;
    
    // Difficulty context
    float currentDifficulty;
    std::string lastDifficultyAdjustment;
    float performanceScore;
};

/**
 * @brief Enhanced Oracle response with AI integration
 */
struct EnhancedOracleResponse {
    std::string responseText;
    std::string responseType; // hint, strategy, lore, encouragement, warning
    float confidence;
    float relevance;
    
    // Contextual additions
    std::vector<std::string> actionSuggestions;
    std::vector<std::string> locationHints;
    std::vector<std::string> itemRecommendations;
    std::vector<std::string> strategyTips;
    
    // Personalization
    std::string personalizedGreeting;
    std::string motivationalMessage;
    std::vector<std::string> adaptedContent;
    
    // Follow-up
    std::vector<std::string> followUpQuestions;
    std::vector<std::string> relatedTopics;
    bool suggestsContinuation;
    
    // Metadata
    uint32_t generationTime;
    std::string aiModel;
    std::vector<std::string> sourceSystems;
};

/**
 * @brief Oracle response enhancement strategies
 */
enum class ResponseEnhancementType {
    CONTEXTUAL_HINTS,      // Add context-specific hints
    BEHAVIORAL_ADAPTATION, // Adapt to player behavior
    DIFFICULTY_GUIDANCE,   // Provide difficulty-appropriate advice
    EMOTIONAL_SUPPORT,     // Offer encouragement or motivation
    STRATEGIC_ANALYSIS,    // Provide tactical/strategic insights
    LORE_ENRICHMENT,      // Add relevant lore and backstory
    PERSONALIZATION,      // Customize for individual player
    PREDICTIVE_ASSISTANCE // Anticipate player needs
};

/**
 * @brief Contextual Oracle System
 * 
 * Enhances the base Oracle system with AI-powered features:
 * - Context-aware responses based on game state
 * - Behavioral adaptation to player patterns
 * - Predictive assistance and proactive suggestions
 * - Multi-modal response generation
 * - Emotional intelligence and support
 */
class ContextualOracle {
public:
    ContextualOracle();
    ~ContextualOracle();
    
    // Initialization
    bool Initialize();
    void ConnectToBaseOracle(OracleSystem* baseOracle);
    void LoadContextualData(const std::string& dataFile);
    void LoadPersonalizationProfiles(const std::string& profileFile);
    
    // Enhanced query processing
    std::unique_ptr<EnhancedOracleResponse> ProcessQuery(
        const std::string& query,
        const OracleQueryContext& context
    );
    
    std::unique_ptr<EnhancedOracleResponse> ProcessContextualQuery(
        const std::string& query,
        const Player& player,
        const std::vector<std::string>& gameState
    );
    
    // Proactive assistance
    std::vector<std::unique_ptr<EnhancedOracleResponse>> GenerateProactiveSuggestions(
        const Player& player,
        const OracleQueryContext& context
    );
    
    std::unique_ptr<EnhancedOracleResponse> GenerateHelpForStuckPlayer(
        const Player& player,
        const std::string& stuckReason
    );
    
    std::unique_ptr<EnhancedOracleResponse> GenerateEncouragement(
        const Player& player,
        const std::string& situation
    );
    
    // Specialized responses
    std::unique_ptr<EnhancedOracleResponse> GenerateQuestGuidance(
        const Player& player,
        const std::string& questId,
        const std::string& questStage
    );
    
    std::unique_ptr<EnhancedOracleResponse> GenerateCombatStrategy(
        const Player& player,
        const std::vector<std::string>& enemyTypes,
        const std::string& location
    );
    
    std::unique_ptr<EnhancedOracleResponse> GenerateItemAdvice(
        const Player& player,
        const std::vector<std::string>& availableItems,
        const std::string& adviceType
    );
    
    std::unique_ptr<EnhancedOracleResponse> GenerateExplorationGuidance(
        const Player& player,
        const std::string& currentArea,
        const std::vector<std::string>& unexploredAreas
    );
    
    // Response enhancement
    void EnhanceResponse(
        EnhancedOracleResponse& response,
        const OracleQueryContext& context,
        const std::vector<ResponseEnhancementType>& enhancements
    );
    
    // Context building
    OracleQueryContext BuildQueryContext(const Player& player);
    void UpdateContextWithGameState(OracleQueryContext& context, const std::vector<std::string>& gameState);
    void UpdateContextWithBehavior(OracleQueryContext& context, const std::string& behaviorData);
    
    // Personalization
    void LearnFromInteraction(const std::string& playerId, const std::string& query, const EnhancedOracleResponse& response, float satisfaction);
    void UpdatePlayerProfile(const std::string& playerId, const std::unordered_map<std::string, float>& preferences);
    std::unordered_map<std::string, float> GetPlayerProfile(const std::string& playerId) const;
    
    // Configuration
    void SetEnhancementLevel(float level) { enhancementLevel_ = std::clamp(level, 0.0f, 1.0f); }
    void SetContextSensitivity(float sensitivity) { contextSensitivity_ = std::clamp(sensitivity, 0.0f, 1.0f); }
    void SetProactiveMode(bool enabled) { proactiveMode_ = enabled; }
    void SetPersonalizationEnabled(bool enabled) { personalizationEnabled_ = enabled; }
    void SetEmotionalIntelligence(bool enabled) { emotionalIntelligence_ = enabled; }
    
    // Analytics and learning
    struct OracleAnalytics {
        uint32_t totalQueries = 0;
        uint32_t contextualQueries = 0;
        uint32_t proactiveSuggestions = 0;
        float averageResponseTime = 0.0f;
        float averageConfidence = 0.0f;
        float averageRelevance = 0.0f;
        float averageSatisfaction = 0.0f;
        std::unordered_map<std::string, uint32_t> queryTypeCount;
        std::unordered_map<std::string, float> enhancementEffectiveness;
    };
    
    const OracleAnalytics& GetAnalytics() const { return analytics_; }
    void ResetAnalytics();
    
private:
    // Core systems
    OracleSystem* baseOracle_ = nullptr;
    
    // Context and personalization data
    std::unordered_map<std::string, std::unordered_map<std::string, float>> playerProfiles_;
    std::unordered_map<std::string, std::vector<std::string>> contextualHints_;
    std::unordered_map<std::string, std::vector<std::string>> behavioralResponses_;
    
    // Configuration
    float enhancementLevel_ = 0.8f;
    float contextSensitivity_ = 0.7f;
    bool proactiveMode_ = true;
    bool personalizationEnabled_ = true;
    bool emotionalIntelligence_ = true;
    
    // Analytics
    OracleAnalytics analytics_;
    
    // Enhancement methods
    void AddContextualHints(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void AdaptToBehavior(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void AddDifficultyGuidance(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void AddEmotionalSupport(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void AddStrategicAnalysis(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void EnrichWithLore(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void PersonalizeResponse(EnhancedOracleResponse& response, const OracleQueryContext& context);
    void AddPredictiveAssistance(EnhancedOracleResponse& response, const OracleQueryContext& context);
    
    // Context analysis
    float AnalyzeFrustrationLevel(const OracleQueryContext& context);
    float AnalyzeEngagementLevel(const OracleQueryContext& context);
    std::vector<std::string> IdentifyPlayerNeeds(const OracleQueryContext& context);
    std::string DetermineOptimalResponseTone(const OracleQueryContext& context);
    
    // Response generation helpers
    std::vector<std::string> GenerateActionSuggestions(const OracleQueryContext& context, const std::string& responseType);
    std::vector<std::string> GenerateLocationHints(const OracleQueryContext& context);
    std::vector<std::string> GenerateItemRecommendations(const OracleQueryContext& context);
    std::vector<std::string> GenerateStrategyTips(const OracleQueryContext& context);
    
    // Learning and adaptation
    void UpdateResponseEffectiveness(const std::string& responseType, float satisfaction);
    void AdaptToPlayerFeedback(const std::string& playerId, const std::string& feedback);
    
    // Utility methods
    float CalculateResponseRelevance(const EnhancedOracleResponse& response, const OracleQueryContext& context);
    std::string SelectOptimalResponseStyle(const OracleQueryContext& context);
    bool ShouldProvideProactiveSuggestion(const OracleQueryContext& context);
    
    // Default content loading
    void LoadDefaultContextualHints();
    void LoadDefaultBehavioralResponses();
};

} // namespace devilution::ai