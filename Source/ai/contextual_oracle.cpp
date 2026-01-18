#include "ai/contextual_oracle.h"

#include <algorithm>
#include <chrono>
#include <sstream>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

namespace devilution::ai {

namespace {

// Contextual hint templates
const std::unordered_map<std::string, std::vector<std::string>> CONTEXTUAL_HINTS = {
    {"stuck_combat", {
        "Try using different attack patterns against this enemy type.",
        "Consider using ranged attacks or magic if melee isn't working.",
        "Look for environmental advantages or retreat to a better position.",
        "Check if you have the right equipment for this encounter."
    }},
    {"stuck_exploration", {
        "Have you checked all the walls for hidden passages?",
        "Try interacting with suspicious objects or decorations.",
        "Some areas require specific items or keys to access.",
        "Consider talking to NPCs for location hints."
    }},
    {"low_health", {
        "You should find healing potions or visit a healer.",
        "Consider retreating to a safe area to recover.",
        "Look for health shrines or healing wells.",
        "Maybe it's time to upgrade your armor for better protection."
    }},
    {"quest_guidance", {
        "Check your quest log for specific objectives and hints.",
        "Talk to the quest giver again if you're unsure about the goal.",
        "Some quests require you to explore specific areas or defeat certain enemies.",
        "Look for quest-related items or NPCs in the area."
    }}
};

// Behavioral adaptation responses
const std::unordered_map<std::string, std::vector<std::string>> BEHAVIORAL_RESPONSES = {
    {"aggressive_player", {
        "I see you prefer direct combat approaches.",
        "Your aggressive style is noted - here are some tactical suggestions.",
        "For a warrior like yourself, consider these combat strategies.",
        "Your bold approach requires careful resource management."
    }},
    {"cautious_player", {
        "Your careful approach is wise in these dangerous lands.",
        "I appreciate your methodical exploration style.",
        "Your cautious nature will serve you well - here's some guidance.",
        "Patience and preparation are virtues you clearly possess."
    }},
    {"explorer_player", {
        "I see you have a keen eye for exploration and discovery.",
        "Your thorough investigation of areas is commendable.",
        "For someone who appreciates the hidden secrets of this world...",
        "Your curiosity about the world's mysteries is admirable."
    }},
    {"social_player", {
        "I notice you enjoy conversing with the inhabitants of this realm.",
        "Your interest in the stories and lore of others is refreshing.",
        "As someone who values knowledge and conversation...",
        "Your social nature opens many doors to hidden knowledge."
    }}
};

// Emotional support messages
const std::unordered_map<std::string, std::vector<std::string>> EMOTIONAL_SUPPORT = {
    {"encouragement", {
        "Do not lose heart, brave adventurer. Every hero faces challenges.",
        "Your perseverance in the face of adversity shows true courage.",
        "Remember, even the greatest heroes had moments of doubt.",
        "The path of a hero is never easy, but you have the strength to continue."
    }},
    {"celebration", {
        "Excellent work! Your skills are clearly improving.",
        "Well done! That was a masterful display of heroism.",
        "Impressive! You handled that situation with great skill.",
        "Magnificent! Your prowess continues to grow."
    }},
    {"comfort", {
        "Setbacks are part of every hero's journey. Learn and grow stronger.",
        "Do not be discouraged. Even in defeat, there are lessons to be learned.",
        "Every fall teaches us how to rise again, stronger than before.",
        "Failure is but a stepping stone on the path to greatness."
    }}
};

} // anonymous namespace

ContextualOracle::ContextualOracle() = default;

ContextualOracle::~ContextualOracle() = default;

bool ContextualOracle::Initialize() {
    LogVerbose("Initializing AI Contextual Oracle...");
    
    // Load default contextual data
    LoadDefaultContextualHints();
    LoadDefaultBehavioralResponses();
    
    LogVerbose("Contextual Oracle initialized");
    return true;
}

void ContextualOracle::ConnectToBaseOracle(OracleSystem* baseOracle) {
    baseOracle_ = baseOracle;
    LogVerbose("Connected to base Oracle system");
}

std::unique_ptr<EnhancedOracleResponse> ContextualOracle::ProcessQuery(
    const std::string& query,
    const OracleQueryContext& context) {
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    auto response = std::make_unique<EnhancedOracleResponse>();
    
    // Get base response from Oracle system if available
    if (baseOracle_) {
        // In a real implementation, this would call the base Oracle
        response->responseText = "The ancient wisdom speaks: " + query;
        response->responseType = "guidance";
    } else {
        // Generate basic response
        response->responseText = GenerateBasicResponse(query, context);
        response->responseType = DetermineResponseType(query, context);
    }
    
    // Enhance the response with AI capabilities
    std::vector<ResponseEnhancementType> enhancements = {
        ResponseEnhancementType::CONTEXTUAL_HINTS,
        ResponseEnhancementType::BEHAVIORAL_ADAPTATION,
        ResponseEnhancementType::EMOTIONAL_SUPPORT,
        ResponseEnhancementType::PERSONALIZATION
    };
    
    EnhanceResponse(*response, context, enhancements);
    
    // Calculate metrics
    response->confidence = CalculateResponseConfidence(*response, context);
    response->relevance = CalculateResponseRelevance(*response, context);
    
    // Update analytics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    analytics_.totalQueries++;
    analytics_.contextualQueries++;
    analytics_.averageResponseTime = (analytics_.averageResponseTime * (analytics_.totalQueries - 1) + 
                                     duration.count() / 1000.0f) / analytics_.totalQueries;
    analytics_.averageConfidence = (analytics_.averageConfidence * (analytics_.totalQueries - 1) + 
                                   response->confidence) / analytics_.totalQueries;
    analytics_.averageRelevance = (analytics_.averageRelevance * (analytics_.totalQueries - 1) + 
                                  response->relevance) / analytics_.totalQueries;
    
    response->generationTime = static_cast<uint32_t>(duration.count());
    response->aiModel = "ContextualOracle";
    response->sourceSystems = {"AI_Engine", "DialogueGenerator", "BehaviorTracker"};
    
    LogVerbose("Processed contextual query (confidence: {:.2f}, relevance: {:.2f}, time: {:.2f}ms)",
               response->confidence, response->relevance, duration.count() / 1000.0f);
    
    return response;
}

std::unique_ptr<EnhancedOracleResponse> ContextualOracle::ProcessContextualQuery(
    const std::string& query,
    const Player& player,
    const std::vector<std::string>& gameState) {
    
    OracleQueryContext context = BuildQueryContext(player);
    UpdateContextWithGameState(context, gameState);
    
    return ProcessQuery(query, context);
}

std::vector<std::unique_ptr<EnhancedOracleResponse>> ContextualOracle::GenerateProactiveSuggestions(
    const Player& player,
    const OracleQueryContext& context) {
    
    std::vector<std::unique_ptr<EnhancedOracleResponse>> suggestions;
    
    if (!proactiveMode_) {
        return suggestions;
    }
    
    // Analyze context for proactive opportunities
    if (context.frustrationLevel > 0.7f) {
        auto helpResponse = GenerateHelpForStuckPlayer(player, "high_frustration");
        if (helpResponse) {
            suggestions.push_back(std::move(helpResponse));
        }
    }
    
    if (context.sessionDeaths > 3) {
        auto encouragement = GenerateEncouragement(player, "multiple_deaths");
        if (encouragement) {
            suggestions.push_back(std::move(encouragement));
        }
    }
    
    if (context.activeQuests.empty() && context.sessionDuration > 1800) { // 30 minutes
        auto questSuggestion = std::make_unique<EnhancedOracleResponse>();
        questSuggestion->responseText = "You've been exploring for a while without any active quests. Consider talking to NPCs in town for new adventures.";
        questSuggestion->responseType = "suggestion";
        questSuggestion->actionSuggestions = {"Visit town", "Talk to NPCs", "Check quest board"};
        suggestions.push_back(std::move(questSuggestion));
    }
    
    analytics_.proactiveSuggestions += suggestions.size();
    
    return suggestions;
}

std::unique_ptr<EnhancedOracleResponse> ContextualOracle::GenerateHelpForStuckPlayer(
    const Player& player,
    const std::string& stuckReason) {
    
    auto response = std::make_unique<EnhancedOracleResponse>();
    
    if (stuckReason == "combat") {
        response->responseText = "I sense you're having difficulty in combat. ";
        response->actionSuggestions = SelectRandomElements(CONTEXTUAL_HINTS.at("stuck_combat"), 2);
    } else if (stuckReason == "exploration") {
        response->responseText = "It seems you're having trouble finding your way. ";
        response->actionSuggestions = SelectRandomElements(CONTEXTUAL_HINTS.at("stuck_exploration"), 2);
    } else if (stuckReason == "high_frustration") {
        response->responseText = "I can sense your frustration, brave adventurer. ";
        response->actionSuggestions = {"Take a short break", "Try a different approach", "Ask for help from other players"};
    }
    
    response->responseType = "assistance";
    response->motivationalMessage = SelectRandomElement(EMOTIONAL_SUPPORT.at("encouragement"));
    
    return response;
}

std::unique_ptr<EnhancedOracleResponse> ContextualOracle::GenerateEncouragement(
    const Player& player,
    const std::string& situation) {
    
    auto response = std::make_unique<EnhancedOracleResponse>();
    
    if (situation == "multiple_deaths") {
        response->responseText = SelectRandomElement(EMOTIONAL_SUPPORT.at("comfort"));
        response->actionSuggestions = {"Consider upgrading equipment", "Try a different strategy", "Explore easier areas first"};
    } else if (situation == "achievement") {
        response->responseText = SelectRandomElement(EMOTIONAL_SUPPORT.at("celebration"));
    } else {
        response->responseText = SelectRandomElement(EMOTIONAL_SUPPORT.at("encouragement"));
    }
    
    response->responseType = "encouragement";
    response->motivationalMessage = response->responseText;
    
    return response;
}

std::unique_ptr<EnhancedOracleResponse> ContextualOracle::GenerateQuestGuidance(
    const Player& player,
    const std::string& questId,
    const std::string& questStage) {
    
    auto response = std::make_unique<EnhancedOracleResponse>();
    
    response->responseText = "Regarding your current quest '" + questId + "': ";
    
    if (questStage == "start") {
        response->responseText += "You have just begun this journey. ";
        response->actionSuggestions = {"Read quest description carefully", "Talk to quest giver for details", "Prepare appropriate equipment"};
    } else if (questStage == "progress") {
        response->responseText += "You are making progress. ";
        response->actionSuggestions = {"Check quest objectives", "Explore the target area", "Look for quest-specific items"};
    } else if (questStage == "stuck") {
        response->responseText += "It seems you need guidance to proceed. ";
        response->actionSuggestions = SelectRandomElements(CONTEXTUAL_HINTS.at("quest_guidance"), 3);
    }
    
    response->responseType = "quest_guidance";
    response->followUpQuestions = {"Do you need more specific directions?", "Would you like tactical advice?", "Should I explain the quest lore?"};
    
    return response;
}

std::unique_ptr<EnhancedOracleResponse> ContextualOracle::GenerateCombatStrategy(
    const Player& player,
    const std::vector<std::string>& enemyTypes,
    const std::string& location) {
    
    auto response = std::make_unique<EnhancedOracleResponse>();
    
    response->responseText = "For combat in " + location + " against ";
    
    if (enemyTypes.size() == 1) {
        response->responseText += enemyTypes[0] + ": ";
    } else {
        response->responseText += "multiple enemy types: ";
    }
    
    // Generate strategy based on enemy types
    std::vector<std::string> strategies;
    for (const auto& enemy : enemyTypes) {
        if (enemy == "undead") {
            strategies.push_back("Use blessed weapons or holy magic");
        } else if (enemy == "demons") {
            strategies.push_back("Fire resistance and cold attacks are effective");
        } else if (enemy == "beasts") {
            strategies.push_back("Keep your distance and use ranged attacks");
        } else {
            strategies.push_back("Adapt your tactics to enemy weaknesses");
        }
    }
    
    response->strategyTips = strategies;
    response->responseType = "combat_strategy";
    
    return response;
}

void ContextualOracle::EnhanceResponse(
    EnhancedOracleResponse& response,
    const OracleQueryContext& context,
    const std::vector<ResponseEnhancementType>& enhancements) {
    
    for (const auto& enhancement : enhancements) {
        switch (enhancement) {
            case ResponseEnhancementType::CONTEXTUAL_HINTS:
                AddContextualHints(response, context);
                break;
                
            case ResponseEnhancementType::BEHAVIORAL_ADAPTATION:
                AdaptToBehavior(response, context);
                break;
                
            case ResponseEnhancementType::DIFFICULTY_GUIDANCE:
                AddDifficultyGuidance(response, context);
                break;
                
            case ResponseEnhancementType::EMOTIONAL_SUPPORT:
                AddEmotionalSupport(response, context);
                break;
                
            case ResponseEnhancementType::STRATEGIC_ANALYSIS:
                AddStrategicAnalysis(response, context);
                break;
                
            case ResponseEnhancementType::LORE_ENRICHMENT:
                EnrichWithLore(response, context);
                break;
                
            case ResponseEnhancementType::PERSONALIZATION:
                PersonalizeResponse(response, context);
                break;
                
            case ResponseEnhancementType::PREDICTIVE_ASSISTANCE:
                AddPredictiveAssistance(response, context);
                break;
        }
    }
}

OracleQueryContext ContextualOracle::BuildQueryContext(const Player& player) {
    OracleQueryContext context;
    
    // Player context
    context.playerId = "player_1"; // Would be actual player ID
    context.playerLevel = player._pLevel;
    context.playerClass = "warrior"; // Simplified - would determine actual class
    context.playerPosition = Point{player.position.tile.x, player.position.tile.y};
    context.currentLocation = "tristram"; // Would determine actual location
    
    // Game state context (simplified)
    context.activeQuests = {}; // Would get from quest system
    context.completedQuests = {}; // Would get from quest system
    context.availableNPCs = {"griswold", "pepin", "deckard_cain"}; // Would scan area
    
    // Session context
    context.sessionDuration = 1800.0f; // Would get actual session time
    context.sessionDeaths = 0; // Would track deaths
    context.sessionLevelUps = 0; // Would track level ups
    
    // Behavioral context (would integrate with BehaviorTracker)
    context.dominantBehaviorPattern = "casual_player";
    context.frustrationLevel = 0.3f;
    context.engagementLevel = 0.7f;
    
    // Difficulty context (would integrate with DifficultyAnalyzer)
    context.currentDifficulty = 1.0f;
    context.performanceScore = 0.6f;
    
    return context;
}

void ContextualOracle::AddContextualHints(EnhancedOracleResponse& response, const OracleQueryContext& context) {
    // Add hints based on current context
    if (context.frustrationLevel > 0.6f) {
        auto hints = SelectRandomElements(CONTEXTUAL_HINTS.at("stuck_exploration"), 2);
        response.actionSuggestions.insert(response.actionSuggestions.end(), hints.begin(), hints.end());
    }
    
    if (context.sessionDeaths > 2) {
        auto hints = SelectRandomElements(CONTEXTUAL_HINTS.at("low_health"), 1);
        response.actionSuggestions.insert(response.actionSuggestions.end(), hints.begin(), hints.end());
    }
}

void ContextualOracle::AdaptToBehavior(EnhancedOracleResponse& response, const OracleQueryContext& context) {
    std::string behaviorKey;
    
    if (context.dominantBehaviorPattern == "aggressive_combat") {
        behaviorKey = "aggressive_player";
    } else if (context.dominantBehaviorPattern == "exploration_focused") {
        behaviorKey = "explorer_player";
    } else if (context.dominantBehaviorPattern == "social_player") {
        behaviorKey = "social_player";
    } else {
        behaviorKey = "cautious_player";
    }
    
    auto behavioralIt = behavioralResponses_.find(behaviorKey);
    if (behavioralIt != behavioralResponses_.end()) {
        std::string adaptation = SelectRandomElement(behavioralIt->second);
        response.personalizedGreeting = adaptation;
    }
}

void ContextualOracle::AddEmotionalSupport(EnhancedOracleResponse& response, const OracleQueryContext& context) {
    if (context.frustrationLevel > 0.7f) {
        response.motivationalMessage = SelectRandomElement(EMOTIONAL_SUPPORT.at("encouragement"));
    } else if (context.engagementLevel > 0.8f) {
        response.motivationalMessage = SelectRandomElement(EMOTIONAL_SUPPORT.at("celebration"));
    }
}

void ContextualOracle::PersonalizeResponse(EnhancedOracleResponse& response, const OracleQueryContext& context) {
    // Get player profile for personalization
    auto profile = GetPlayerProfile(context.playerId);
    
    // Adjust response based on player preferences
    if (profile.find("prefers_detailed_explanations") != profile.end() && 
        profile.at("prefers_detailed_explanations") > 0.7f) {
        // Add more detailed information
        response.adaptedContent.push_back("Let me provide more detailed information...");
    }
    
    if (profile.find("likes_lore") != profile.end() && 
        profile.at("likes_lore") > 0.6f) {
        // Add lore elements
        response.adaptedContent.push_back("The ancient texts speak of similar situations...");
    }
}

std::string ContextualOracle::GenerateBasicResponse(const std::string& query, const OracleQueryContext& context) {
    // Generate a basic response when no base Oracle is available
    std::string response = "I sense you seek guidance about: " + query + ". ";
    
    if (context.playerLevel < 10) {
        response += "As a beginning adventurer, ";
    } else if (context.playerLevel < 25) {
        response += "As an experienced hero, ";
    } else {
        response += "As a seasoned champion, ";
    }
    
    response += "you should know that wisdom comes through experience and careful observation.";
    
    return response;
}

std::string ContextualOracle::DetermineResponseType(const std::string& query, const OracleQueryContext& context) {
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    if (lowerQuery.find("help") != std::string::npos || lowerQuery.find("stuck") != std::string::npos) {
        return "assistance";
    } else if (lowerQuery.find("quest") != std::string::npos) {
        return "quest_guidance";
    } else if (lowerQuery.find("combat") != std::string::npos || lowerQuery.find("fight") != std::string::npos) {
        return "combat_strategy";
    } else if (lowerQuery.find("lore") != std::string::npos || lowerQuery.find("story") != std::string::npos) {
        return "lore";
    } else {
        return "guidance";
    }
}

float ContextualOracle::CalculateResponseConfidence(const EnhancedOracleResponse& response, const OracleQueryContext& context) {
    float confidence = 0.5f; // Base confidence
    
    // Higher confidence with more context
    if (!context.activeQuests.empty()) confidence += 0.2f;
    if (context.sessionDuration > 600) confidence += 0.1f; // 10 minutes
    if (!response.actionSuggestions.empty()) confidence += 0.2f;
    
    return std::clamp(confidence, 0.0f, 1.0f);
}

float ContextualOracle::CalculateResponseRelevance(const EnhancedOracleResponse& response, const OracleQueryContext& context) {
    float relevance = 0.5f; // Base relevance
    
    // Higher relevance for context-appropriate responses
    if (response.responseType == "assistance" && context.frustrationLevel > 0.6f) {
        relevance += 0.3f;
    }
    
    if (response.responseType == "quest_guidance" && !context.activeQuests.empty()) {
        relevance += 0.3f;
    }
    
    if (!response.actionSuggestions.empty()) {
        relevance += 0.2f;
    }
    
    return std::clamp(relevance, 0.0f, 1.0f);
}

std::string ContextualOracle::SelectRandomElement(const std::vector<std::string>& elements) {
    if (elements.empty()) return "";
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, elements.size() - 1);
    
    return elements[dis(gen)];
}

std::vector<std::string> ContextualOracle::SelectRandomElements(const std::vector<std::string>& elements, int count) {
    std::vector<std::string> selected;
    
    if (elements.empty() || count <= 0) return selected;
    
    std::vector<std::string> shuffled = elements;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(shuffled.begin(), shuffled.end(), gen);
    
    int actualCount = std::min(count, static_cast<int>(shuffled.size()));
    for (int i = 0; i < actualCount; ++i) {
        selected.push_back(shuffled[i]);
    }
    
    return selected;
}

void ContextualOracle::LoadDefaultContextualHints() {
    contextualHints_ = CONTEXTUAL_HINTS;
}

void ContextualOracle::LoadDefaultBehavioralResponses() {
    behavioralResponses_ = BEHAVIORAL_RESPONSES;
}

std::unordered_map<std::string, float> ContextualOracle::GetPlayerProfile(const std::string& playerId) const {
    auto it = playerProfiles_.find(playerId);
    if (it != playerProfiles_.end()) {
        return it->second;
    }
    
    // Return default profile
    return {
        {"prefers_detailed_explanations", 0.5f},
        {"likes_lore", 0.5f},
        {"prefers_direct_answers", 0.5f},
        {"enjoys_hints", 0.7f}
    };
}

void ContextualOracle::ResetAnalytics() {
    analytics_ = OracleAnalytics{};
}

} // namespace devilution::ai