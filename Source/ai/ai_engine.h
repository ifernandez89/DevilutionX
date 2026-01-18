#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "ai/quest_generator.h"
#include "ai/dialogue_generator.h"
#include "ai/item_generator.h"
#include "ai/difficulty_analyzer.h"
#include "ai/player_behavior_tracker.h"
#include "ai/contextual_oracle.h"

namespace devilution::ai {

/**
 * @brief Core AI Engine for DevilutionX Nightmare Edition
 * 
 * Provides advanced AI capabilities including:
 * - Procedural quest generation
 * - Dynamic dialogue creation
 * - Intelligent item generation
 * - Adaptive difficulty scaling
 * - Enhanced Oracle system
 */
class AIEngine {
public:
    static AIEngine& GetInstance();
    
    // Initialization and cleanup
    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    
    // Core AI systems
    QuestGenerator& GetQuestGenerator() { return *questGenerator_; }
    DialogueGenerator& GetDialogueGenerator() { return *dialogueGenerator_; }
    ItemGenerator& GetItemGenerator() { return *itemGenerator_; }
    DifficultyAnalyzer& GetDifficultyAnalyzer() { return *difficultyAnalyzer_; }
    PlayerBehaviorTracker& GetBehaviorTracker() { return *behaviorTracker_; }
    ContextualOracle& GetContextualOracle() { return *contextualOracle_; }
    
    // Update and processing
    void Update(uint32_t deltaTime);
    void ProcessGameEvent(const std::string& eventType, const std::string& eventData);
    
    // Configuration
    void SetAIEnabled(bool enabled) { aiEnabled_ = enabled; }
    bool IsAIEnabled() const { return aiEnabled_; }
    
    void SetPerformanceMode(bool highPerformance) { highPerformanceMode_ = highPerformance; }
    bool IsHighPerformanceMode() const { return highPerformanceMode_; }
    
    // Statistics and monitoring
    struct AIStats {
        uint32_t questsGenerated = 0;
        uint32_t dialoguesCreated = 0;
        uint32_t itemsGenerated = 0;
        uint32_t difficultyAdjustments = 0;
        uint32_t oracleQueries = 0;
        float averageProcessingTime = 0.0f;
        float memoryUsageMB = 0.0f;
    };
    
    const AIStats& GetStats() const { return stats_; }
    void ResetStats();
    
private:
    AIEngine() = default;
    ~AIEngine() = default;
    AIEngine(const AIEngine&) = delete;
    AIEngine& operator=(const AIEngine&) = delete;
    
    // Core systems
    std::unique_ptr<QuestGenerator> questGenerator_;
    std::unique_ptr<DialogueGenerator> dialogueGenerator_;
    std::unique_ptr<ItemGenerator> itemGenerator_;
    std::unique_ptr<DifficultyAnalyzer> difficultyAnalyzer_;
    std::unique_ptr<PlayerBehaviorTracker> behaviorTracker_;
    std::unique_ptr<ContextualOracle> contextualOracle_;
    
    // State management
    bool initialized_ = false;
    bool aiEnabled_ = true;
    bool highPerformanceMode_ = false;
    
    // Statistics
    AIStats stats_;
    uint32_t lastUpdateTime_ = 0;
    
    // Performance monitoring
    void UpdatePerformanceMetrics();
    void OptimizePerformance();
};

// Global AI engine access
inline AIEngine& GetAI() {
    return AIEngine::GetInstance();
}

// AI system macros for easy access
#define AI_QUEST_GEN GetAI().GetQuestGenerator()
#define AI_DIALOGUE_GEN GetAI().GetDialogueGenerator()
#define AI_ITEM_GEN GetAI().GetItemGenerator()
#define AI_DIFFICULTY GetAI().GetDifficultyAnalyzer()
#define AI_BEHAVIOR GetAI().GetBehaviorTracker()
#define AI_ORACLE GetAI().GetContextualOracle()

} // namespace devilution::ai