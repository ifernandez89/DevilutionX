#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "player.h"
#include "towners.h"

namespace devilution::ai {

/**
 * @brief Dialogue context for AI generation
 */
struct DialogueContext {
    std::string npcId;
    std::string npcName;
    std::string npcPersonality;
    std::string location;
    std::string timeOfDay;
    std::string playerClass;
    int playerLevel;
    std::vector<std::string> recentEvents;
    std::vector<std::string> completedQuests;
    std::vector<std::string> activeQuests;
    std::string playerReputation;
    std::string mood; // happy, sad, angry, neutral, etc.
};

/**
 * @brief Generated dialogue response
 */
struct GeneratedDialogue {
    std::string id;
    std::string text;
    std::string emotion;
    std::string tone; // formal, casual, friendly, hostile, etc.
    std::vector<std::string> playerOptions;
    std::vector<std::string> followUpTopics;
    bool endsConversation = false;
    bool triggersQuest = false;
    std::string questId;
    float relevanceScore = 0.0f;
};

/**
 * @brief NPC personality traits
 */
struct NPCPersonality {
    std::string name;
    std::string basePersonality; // friendly, gruff, mysterious, etc.
    std::vector<std::string> traits;
    std::vector<std::string> speechPatterns;
    std::vector<std::string> favoriteTopics;
    std::vector<std::string> avoidedTopics;
    float friendliness = 0.5f;
    float helpfulness = 0.5f;
    float chattiness = 0.5f;
    float formality = 0.5f;
};

/**
 * @brief Dynamic Dialogue Generator
 * 
 * Generates contextual NPC dialogue based on:
 * - Player progress and actions
 * - NPC personality and mood
 * - Current game state and events
 * - Previous conversations
 */
class DialogueGenerator {
public:
    DialogueGenerator();
    ~DialogueGenerator();
    
    // Initialization
    bool Initialize();
    void LoadPersonalities(const std::string& personalityFile);
    void LoadDialogueTemplates(const std::string& templateFile);
    
    // Dialogue generation
    std::unique_ptr<GeneratedDialogue> GenerateDialogue(
        const DialogueContext& context,
        const std::string& topic = ""
    );
    
    std::vector<std::unique_ptr<GeneratedDialogue>> GenerateConversationFlow(
        const DialogueContext& context,
        int maxExchanges = 5
    );
    
    // Context-aware responses
    std::unique_ptr<GeneratedDialogue> GenerateGreeting(const DialogueContext& context);
    std::unique_ptr<GeneratedDialogue> GenerateFarewell(const DialogueContext& context);
    std::unique_ptr<GeneratedDialogue> GenerateQuestDialogue(const DialogueContext& context, const std::string& questId);
    std::unique_ptr<GeneratedDialogue> GenerateShopDialogue(const DialogueContext& context);
    
    // Personality management
    void AddPersonality(const NPCPersonality& personality);
    const NPCPersonality* GetPersonality(const std::string& npcId) const;
    void UpdateNPCMood(const std::string& npcId, const std::string& mood);
    
    // Conversation memory
    void RecordConversation(const std::string& npcId, const std::string& playerId, const GeneratedDialogue& dialogue);
    std::vector<std::string> GetConversationHistory(const std::string& npcId, const std::string& playerId) const;
    
    // Configuration
    void SetCreativityLevel(float level) { creativityLevel_ = std::clamp(level, 0.0f, 1.0f); }
    void SetContextSensitivity(float sensitivity) { contextSensitivity_ = std::clamp(sensitivity, 0.0f, 1.0f); }
    void SetPersonalityInfluence(float influence) { personalityInfluence_ = std::clamp(influence, 0.0f, 1.0f); }
    
    // Statistics
    struct DialogueStats {
        uint32_t totalGenerated = 0;
        uint32_t greetingsGenerated = 0;
        uint32_t questDialoguesGenerated = 0;
        uint32_t shopDialoguesGenerated = 0;
        float averageRelevanceScore = 0.0f;
        float averageGenerationTime = 0.0f;
        std::unordered_map<std::string, uint32_t> npcInteractionCount;
    };
    
    const DialogueStats& GetStats() const { return stats_; }
    void ResetStats();
    
private:
    // Personality system
    std::unordered_map<std::string, NPCPersonality> personalities_;
    std::unordered_map<std::string, std::string> npcMoods_;
    
    // Template system
    std::vector<std::string> greetingTemplates_;
    std::vector<std::string> farewellTemplates_;
    std::vector<std::string> questTemplates_;
    std::vector<std::string> shopTemplates_;
    std::vector<std::string> generalTemplates_;
    
    // Conversation memory
    std::unordered_map<std::string, std::vector<std::string>> conversationHistory_;
    
    // Configuration
    float creativityLevel_ = 0.7f;
    float contextSensitivity_ = 0.8f;
    float personalityInfluence_ = 0.6f;
    
    // Statistics
    DialogueStats stats_;
    
    // Generation methods
    std::string GenerateDialogueText(const DialogueContext& context, const NPCPersonality& personality, const std::string& topic);
    std::vector<std::string> GeneratePlayerOptions(const DialogueContext& context, const GeneratedDialogue& dialogue);
    std::string DetermineEmotion(const DialogueContext& context, const NPCPersonality& personality);
    std::string DetermineTone(const DialogueContext& context, const NPCPersonality& personality);
    
    // Utility methods
    std::string ApplyPersonalityToText(const std::string& baseText, const NPCPersonality& personality);
    float CalculateRelevance(const GeneratedDialogue& dialogue, const DialogueContext& context);
    std::string ProcessDialogueTemplate(const std::string& templateStr, const DialogueContext& context);
    
    // Default content loading
    void LoadDefaultPersonalities();
    void LoadDefaultTemplates();
};

} // namespace devilution::ai