#include "ai/ai_engine.h"

#include <chrono>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"
#include "ai/dialogue_generator.h"
#include "ai/item_generator.h"
#include "ai/player_behavior_tracker.h"
#include "ai/contextual_oracle.h"

namespace devilution::ai {

AIEngine& AIEngine::GetInstance() {
    static AIEngine instance;
    return instance;
}

bool AIEngine::Initialize() {
    if (initialized_) {
        LogWarning("AI Engine already initialized");
        return true;
    }
    
    LogInfo("🤖 Initializing DevilutionX AI Engine...");
    
    try {
        // Initialize core AI systems
        questGenerator_ = std::make_unique<QuestGenerator>();
        if (!questGenerator_->Initialize()) {
            LogError("Failed to initialize Quest Generator");
            return false;
        }
        
        difficultyAnalyzer_ = std::make_unique<DifficultyAnalyzer>();
        if (!difficultyAnalyzer_->Initialize()) {
            LogError("Failed to initialize Difficulty Analyzer");
            return false;
        }
        
        // Initialize advanced AI systems
        dialogueGenerator_ = std::make_unique<DialogueGenerator>();
        if (!dialogueGenerator_->Initialize()) {
            LogError("Failed to initialize Dialogue Generator");
            return false;
        }
        
        itemGenerator_ = std::make_unique<ItemGenerator>();
        if (!itemGenerator_->Initialize()) {
            LogError("Failed to initialize Item Generator");
            return false;
        }
        
        behaviorTracker_ = std::make_unique<PlayerBehaviorTracker>();
        if (!behaviorTracker_->Initialize()) {
            LogError("Failed to initialize Player Behavior Tracker");
            return false;
        }
        
        contextualOracle_ = std::make_unique<ContextualOracle>();
        if (!contextualOracle_->Initialize()) {
            LogError("Failed to initialize Contextual Oracle");
            return false;
        }
        
        initialized_ = true;
        lastUpdateTime_ = SDL_GetTicks();
        
        LogInfo("✅ AI Engine initialized successfully");
        LogInfo("   - Quest Generator: Active");
        LogInfo("   - Difficulty Analyzer: Active");
        LogInfo("   - Dialogue Generator: Active");
        LogInfo("   - Item Generator: Active");
        LogInfo("   - Behavior Tracker: Active");
        LogInfo("   - Contextual Oracle: Active");
        
        return true;
        
    } catch (const std::exception& e) {
        LogError("Exception during AI Engine initialization: {}", e.what());
        Shutdown();
        return false;
    }
}

void AIEngine::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    LogInfo("🤖 Shutting down AI Engine...");
    
    // Shutdown systems in reverse order
    contextualOracle_.reset();
    behaviorTracker_.reset();
    itemGenerator_.reset();
    dialogueGenerator_.reset();
    difficultyAnalyzer_.reset();
    questGenerator_.reset();
    
    initialized_ = false;
    aiEnabled_ = true;
    highPerformanceMode_ = false;
    
    // Reset statistics
    stats_ = AIStats{};
    
    LogInfo("✅ AI Engine shutdown complete");
}

void AIEngine::Update(uint32_t deltaTime) {
    if (!initialized_ || !aiEnabled_) {
        return;
    }
    
    uint32_t currentTime = SDL_GetTicks();
    uint32_t timeSinceLastUpdate = currentTime - lastUpdateTime_;
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Update individual systems
    if (questGenerator_) {
        // Quest generator doesn't need regular updates
        // It's called on-demand when quests are needed
    }
    
    if (difficultyAnalyzer_) {
        // Difficulty analyzer updates are triggered by game events
        // No regular update needed
    }
    
    if (dialogueGenerator_) {
        // Dialogue generator is event-driven
        // Updates happen during NPC interactions
    }
    
    if (itemGenerator_) {
        // Item generator is called on-demand
        // No regular updates needed
    }
    
    if (behaviorTracker_) {
        behaviorTracker_->Update(deltaTime);
    }
    
    if (contextualOracle_) {
        // Oracle system is query-driven
        // No regular updates needed
    }
    
    lastUpdateTime_ = currentTime;
    
    // Optimize performance if needed
    if (highPerformanceMode_) {
        OptimizePerformance();
    }
}

void AIEngine::ProcessGameEvent(const std::string& eventType, const std::string& eventData) {
    if (!initialized_ || !aiEnabled_) {
        return;
    }
    
    // Route events to appropriate systems
    if (eventType == "player_combat") {
        // Handle combat events for difficulty analysis
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordCombatEvent()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordCombatAction(eventData);
        }
    } else if (eventType == "player_death") {
        // Handle death events
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordDeath()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordDeath(eventData);
        }
    } else if (eventType == "level_complete") {
        // Handle level completion
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordLevelCompletion()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordLevelCompletion(eventData);
        }
    } else if (eventType == "quest_complete") {
        // Handle quest completion
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordQuestCompletion()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordQuestCompletion(eventData);
        }
    } else if (eventType == "item_found") {
        // Handle item events
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordItemEvent()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordItemInteraction(eventData);
        }
    } else if (eventType == "spell_cast") {
        // Handle spell casting
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordSpellCast()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordSpellCast(eventData);
        }
    } else if (eventType == "player_movement") {
        // Handle movement
        if (difficultyAnalyzer_) {
            // Parse eventData and call difficultyAnalyzer_->RecordMovement()
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordMovement(eventData);
        }
    } else if (eventType == "npc_interaction") {
        // Handle NPC interactions
        if (dialogueGenerator_) {
            // Process dialogue generation
        }
        if (behaviorTracker_) {
            behaviorTracker_->RecordNPCInteraction(eventData);
        }
    } else if (eventType == "item_generated") {
        // Handle item generation events
        if (itemGenerator_) {
            // Update generation statistics
        }
    }
}

void AIEngine::UpdatePerformanceMetrics() {
    // Update memory usage (simplified)
    stats_.memoryUsageMB = 0.0f;
    
    if (questGenerator_) {
        // Add quest generator memory usage
        stats_.memoryUsageMB += 1.0f; // Estimated
    }
    
    if (difficultyAnalyzer_) {
        // Add difficulty analyzer memory usage
        stats_.memoryUsageMB += 0.5f; // Estimated
    }
    
    if (dialogueGenerator_) {
        // Add dialogue generator memory usage
        stats_.memoryUsageMB += 2.0f; // Estimated
    }
    
    if (itemGenerator_) {
        // Add item generator memory usage
        stats_.memoryUsageMB += 1.5f; // Estimated
    }
    
    if (behaviorTracker_) {
        // Add behavior tracker memory usage
        stats_.memoryUsageMB += 3.0f; // Estimated (stores player data)
    }
    
    if (contextualOracle_) {
        // Add contextual oracle memory usage
        stats_.memoryUsageMB += 2.5f; // Estimated
    }
    
    // Update processing time (would need actual timing in real implementation)
    stats_.averageProcessingTime = 2.5f; // Estimated in milliseconds
}

void AIEngine::OptimizePerformance() {
    // Performance optimization strategies
    
    // Reduce update frequency for non-critical systems
    static uint32_t optimizationCounter = 0;
    optimizationCounter++;
    
    if (optimizationCounter % 10 == 0) {
        // Every 10th update, perform cleanup
        
        if (difficultyAnalyzer_) {
            // Prune old metrics to save memory
            // difficultyAnalyzer_->PruneOldMetrics();
        }
        
        if (behaviorTracker_) {
            // Cleanup old behavior data
            // behaviorTracker_->PruneOldData();
        }
        
        if (contextualOracle_) {
            // Cleanup old conversation history
            // contextualOracle_->PruneOldConversations();
        }
    }
    
    // Adjust AI complexity based on performance
    if (stats_.averageProcessingTime > 10.0f) {
        LogWarning("AI processing time high ({:.2f}ms), reducing complexity", 
                   stats_.averageProcessingTime);
        
        // Reduce AI complexity
        // This could involve reducing analysis frequency, 
        // simplifying algorithms, etc.
    }
}

void AIEngine::ResetStats() {
    stats_ = AIStats{};
    
    if (questGenerator_) {
        questGenerator_->ResetStats();
    }
    
    if (difficultyAnalyzer_) {
        difficultyAnalyzer_->ResetStats();
    }
    
    if (dialogueGenerator_) {
        dialogueGenerator_->ResetStats();
    }
    
    if (itemGenerator_) {
        itemGenerator_->ResetStats();
    }
    
    if (behaviorTracker_) {
        behaviorTracker_->ResetStats();
    }
    
    if (contextualOracle_) {
        contextualOracle_->ResetAnalytics();
    }
}

} // namespace devilution::ai