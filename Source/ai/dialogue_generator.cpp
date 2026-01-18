#include "ai/dialogue_generator.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <random>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

namespace devilution::ai {

namespace {

// Default NPC personalities for Diablo universe
const std::vector<NPCPersonality> DEFAULT_PERSONALITIES = {
    {
        "griswold", "gruff", 
        {"experienced", "practical", "straightforward", "reliable"},
        {"*grunts*", "Aye, ", "Well now, ", "Listen here, "},
        {"weapons", "armor", "crafting", "battle stories"},
        {"magic theory", "politics", "romance"},
        0.6f, 0.8f, 0.4f, 0.3f
    },
    {
        "pepin", "wise", 
        {"knowledgeable", "caring", "patient", "scholarly"},
        {"Indeed, ", "As I recall, ", "In my studies, ", "Ah yes, "},
        {"healing", "herbs", "ancient lore", "medicine"},
        {"violence", "crude jokes", "impatience"},
        0.8f, 0.9f, 0.7f, 0.8f
    },
    {
        "deckard_cain", "mysterious", 
        {"ancient", "wise", "cryptic", "prophetic"},
        {"Stay awhile and listen... ", "The ancient texts speak of ", "I have seen much in my years, ", "Beware, "},
        {"ancient lore", "prophecies", "horadrim", "demons"},
        {"trivial matters", "interruptions"},
        0.7f, 1.0f, 0.9f, 0.9f
    },
    {
        "wirt", "shifty", 
        {"suspicious", "greedy", "secretive", "opportunistic"},
        {"Psst... ", "For the right price, ", "I might know something about ", "Keep this between us, "},
        {"rare items", "secrets", "deals", "gossip"},
        {"authorities", "honest dealings", "charity"},
        0.4f, 0.6f, 0.8f, 0.2f
    }
};

// Dialogue templates by category
const std::unordered_map<std::string, std::vector<std::string>> DIALOGUE_TEMPLATES = {
    {"greeting", {
        "Well met, {player_class}!",
        "Greetings, traveler.",
        "Ah, {player_name}, good to see you again.",
        "Welcome to {location}.",
        "What brings you here, {player_class}?"
    }},
    {"farewell", {
        "Safe travels, {player_name}.",
        "May the light protect you.",
        "Until we meet again.",
        "Go with caution, the darkness grows.",
        "Farewell, brave {player_class}."
    }},
    {"quest", {
        "I have a task that requires someone of your... talents.",
        "There's trouble brewing, and I need your help.",
        "A {quest_type} has been causing problems. Can you handle it?",
        "The situation is dire. Will you assist us?",
        "I've heard of your deeds, {player_name}. I have need of a hero."
    }},
    {"shop", {
        "Take a look at my wares.",
        "I have some fine {item_type} for sale.",
        "These items have served me well, perhaps they'll serve you too.",
        "Quality goods at fair prices.",
        "Something here might catch your eye."
    }},
    {"lore", {
        "The old stories tell of {lore_topic}.",
        "In ages past, {lore_event} changed everything.",
        "Legend speaks of {lore_artifact} hidden in these lands.",
        "The ancient ones knew secrets we've forgotten.",
        "Dark forces stir, just as the prophecies foretold."
    }}
};

// Emotional responses based on context
const std::unordered_map<std::string, std::vector<std::string>> EMOTIONAL_RESPONSES = {
    {"happy", {
        "Excellent news!",
        "That's wonderful to hear!",
        "My spirits are lifted!",
        "Such good fortune!",
        "The light shines brighter today!"
    }},
    {"sad", {
        "Alas, such tragedy...",
        "My heart grows heavy.",
        "These are dark times indeed.",
        "Such sorrow fills the air.",
        "The shadows deepen with this news."
    }},
    {"angry", {
        "This cannot stand!",
        "Such insolence!",
        "My blood boils at this injustice!",
        "Enough of this madness!",
        "The time for patience has passed!"
    }},
    {"fearful", {
        "I dare not speak of such things...",
        "The very thought chills my bones.",
        "We must be careful, very careful.",
        "Such darkness should not be awakened.",
        "Perhaps it's better left undisturbed."
    }}
};

} // anonymous namespace

DialogueGenerator::DialogueGenerator() = default;

DialogueGenerator::~DialogueGenerator() = default;

bool DialogueGenerator::Initialize() {
    LogVerbose("Initializing AI Dialogue Generator...");
    
    // Load default personalities and templates
    LoadDefaultPersonalities();
    LoadDefaultTemplates();
    
    LogVerbose("Dialogue Generator initialized with {} personalities", personalities_.size());
    return true;
}

void DialogueGenerator::LoadDefaultPersonalities() {
    for (const auto& personality : DEFAULT_PERSONALITIES) {
        personalities_[personality.name] = personality;
    }
}

void DialogueGenerator::LoadDefaultTemplates() {
    // Templates are loaded from the static data above
    greetingTemplates_ = DIALOGUE_TEMPLATES.at("greeting");
    farewellTemplates_ = DIALOGUE_TEMPLATES.at("farewell");
    questTemplates_ = DIALOGUE_TEMPLATES.at("quest");
    shopTemplates_ = DIALOGUE_TEMPLATES.at("shop");
    generalTemplates_ = DIALOGUE_TEMPLATES.at("lore");
}

std::unique_ptr<GeneratedDialogue> DialogueGenerator::GenerateDialogue(
    const DialogueContext& context,
    const std::string& topic) {
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    const NPCPersonality* personality = GetPersonality(context.npcId);
    if (!personality) {
        LogWarning("No personality found for NPC: {}", context.npcId);
        return nullptr;
    }
    
    auto dialogue = std::make_unique<GeneratedDialogue>();
    dialogue->id = GenerateDialogueId();
    
    // Generate dialogue text based on topic or context
    dialogue->text = GenerateDialogueText(context, *personality, topic);
    dialogue->emotion = DetermineEmotion(context, *personality);
    dialogue->tone = DetermineTone(context, *personality);
    
    // Generate player response options
    dialogue->playerOptions = GeneratePlayerOptions(context, *dialogue);
    
    // Determine if this ends the conversation or triggers a quest
    dialogue->endsConversation = (topic == "farewell");
    dialogue->triggersQuest = (topic == "quest" && !context.activeQuests.empty());
    
    // Calculate relevance
    dialogue->relevanceScore = CalculateRelevance(*dialogue, context);
    
    // Update statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    stats_.totalGenerated++;
    if (topic == "greeting") stats_.greetingsGenerated++;
    if (topic == "quest") stats_.questDialoguesGenerated++;
    if (topic == "shop") stats_.shopDialoguesGenerated++;
    
    stats_.averageRelevanceScore = (stats_.averageRelevanceScore * (stats_.totalGenerated - 1) + 
                                   dialogue->relevanceScore) / stats_.totalGenerated;
    stats_.averageGenerationTime = (stats_.averageGenerationTime * (stats_.totalGenerated - 1) + 
                                   duration.count() / 1000.0f) / stats_.totalGenerated;
    
    stats_.npcInteractionCount[context.npcId]++;
    
    LogVerbose("Generated dialogue for {} (relevance: {:.2f}, time: {:.2f}ms)", 
               context.npcId, dialogue->relevanceScore, duration.count() / 1000.0f);
    
    return dialogue;
}

std::unique_ptr<GeneratedDialogue> DialogueGenerator::GenerateGreeting(const DialogueContext& context) {
    return GenerateDialogue(context, "greeting");
}

std::unique_ptr<GeneratedDialogue> DialogueGenerator::GenerateFarewell(const DialogueContext& context) {
    return GenerateDialogue(context, "farewell");
}

std::unique_ptr<GeneratedDialogue> DialogueGenerator::GenerateQuestDialogue(
    const DialogueContext& context, 
    const std::string& questId) {
    
    DialogueContext questContext = context;
    questContext.activeQuests.push_back(questId);
    return GenerateDialogue(questContext, "quest");
}

std::unique_ptr<GeneratedDialogue> DialogueGenerator::GenerateShopDialogue(const DialogueContext& context) {
    return GenerateDialogue(context, "shop");
}

std::string DialogueGenerator::GenerateDialogueText(
    const DialogueContext& context, 
    const NPCPersonality& personality, 
    const std::string& topic) {
    
    std::vector<std::string> templates;
    
    // Select appropriate templates based on topic
    if (topic == "greeting") {
        templates = greetingTemplates_;
    } else if (topic == "farewell") {
        templates = farewellTemplates_;
    } else if (topic == "quest") {
        templates = questTemplates_;
    } else if (topic == "shop") {
        templates = shopTemplates_;
    } else {
        templates = generalTemplates_;
    }
    
    if (templates.empty()) {
        return "I have nothing to say right now.";
    }
    
    // Select random template
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, templates.size() - 1);
    std::string baseText = templates[dis(gen)];
    
    // Process template with context variables
    baseText = ProcessDialogueTemplate(baseText, context);
    
    // Apply personality to the text
    baseText = ApplyPersonalityToText(baseText, personality);
    
    return baseText;
}

std::vector<std::string> DialogueGenerator::GeneratePlayerOptions(
    const DialogueContext& context, 
    const GeneratedDialogue& dialogue) {
    
    std::vector<std::string> options;
    
    // Standard options based on dialogue type
    if (dialogue.tone == "friendly") {
        options.push_back("Tell me more.");
        options.push_back("That's interesting.");
    } else if (dialogue.tone == "formal") {
        options.push_back("I understand.");
        options.push_back("Please continue.");
    } else if (dialogue.tone == "casual") {
        options.push_back("Got it.");
        options.push_back("What else?");
    }
    
    // Context-specific options
    if (!context.activeQuests.empty()) {
        options.push_back("About my current quest...");
    }
    
    if (context.npcId == "griswold" || context.npcId == "wirt") {
        options.push_back("Show me your wares.");
    }
    
    // Always include farewell option
    options.push_back("Farewell.");
    
    return options;
}

std::string DialogueGenerator::DetermineEmotion(
    const DialogueContext& context, 
    const NPCPersonality& personality) {
    
    // Check NPC mood first
    auto moodIt = npcMoods_.find(context.npcId);
    if (moodIt != npcMoods_.end()) {
        return moodIt->second;
    }
    
    // Determine emotion based on context and personality
    if (!context.recentDeaths.empty()) {
        return "sad";
    } else if (!context.completedQuests.empty()) {
        return "happy";
    } else if (context.playerLevel < 5) {
        return "concerned";
    } else {
        return "neutral";
    }
}

std::string DialogueGenerator::DetermineTone(
    const DialogueContext& context, 
    const NPCPersonality& personality) {
    
    // Base tone on personality formality
    if (personality.formality > 0.7f) {
        return "formal";
    } else if (personality.formality < 0.3f) {
        return "casual";
    } else if (personality.friendliness > 0.7f) {
        return "friendly";
    } else if (personality.friendliness < 0.3f) {
        return "hostile";
    } else {
        return "neutral";
    }
}

std::string DialogueGenerator::ApplyPersonalityToText(
    const std::string& baseText, 
    const NPCPersonality& personality) {
    
    std::string result = baseText;
    
    // Add speech patterns
    if (!personality.speechPatterns.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, personality.speechPatterns.size() - 1);
        
        // 30% chance to add a speech pattern prefix
        std::uniform_real_distribution<> chance(0.0, 1.0);
        if (chance(gen) < 0.3) {
            std::string pattern = personality.speechPatterns[dis(gen)];
            result = pattern + result;
        }
    }
    
    // Adjust formality
    if (personality.formality < 0.3f) {
        // Make more casual
        std::string casual = result;
        // Replace formal words with casual equivalents (simplified)
        size_t pos = casual.find("indeed");
        if (pos != std::string::npos) {
            casual.replace(pos, 6, "yeah");
        }
        result = casual;
    }
    
    return result;
}

float DialogueGenerator::CalculateRelevance(
    const GeneratedDialogue& dialogue, 
    const DialogueContext& context) {
    
    float relevance = 0.5f; // Base relevance
    
    // Higher relevance for context-appropriate dialogue
    if (!context.activeQuests.empty() && dialogue.text.find("quest") != std::string::npos) {
        relevance += 0.3f;
    }
    
    // Higher relevance for player level appropriate content
    if (context.playerLevel < 10 && dialogue.text.find("beginner") != std::string::npos) {
        relevance += 0.2f;
    }
    
    // Higher relevance for location-specific content
    if (dialogue.text.find(context.currentLocation) != std::string::npos) {
        relevance += 0.2f;
    }
    
    return std::clamp(relevance, 0.0f, 1.0f);
}

std::string DialogueGenerator::ProcessDialogueTemplate(
    const std::string& templateStr, 
    const DialogueContext& context) {
    
    std::string result = templateStr;
    
    // Replace common placeholders
    std::unordered_map<std::string, std::string> replacements = {
        {"{player_name}", "hero"}, // Would get actual player name in real implementation
        {"{player_class}", context.playerClass},
        {"{location}", context.currentLocation},
        {"{player_level}", std::to_string(context.playerLevel)},
        {"{quest_type}", "dangerous creature"},
        {"{item_type}", "weapons"},
        {"{lore_topic}", "ancient evils"},
        {"{lore_event}", "the great conflict"},
        {"{lore_artifact}", "a powerful relic"}
    };
    
    for (const auto& [placeholder, replacement] : replacements) {
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), replacement);
            pos += replacement.length();
        }
    }
    
    return result;
}

std::string DialogueGenerator::GenerateDialogueId() {
    static uint32_t dialogueCounter = 1;
    std::stringstream ss;
    ss << "ai_dialogue_" << std::setfill('0') << std::setw(6) << dialogueCounter++;
    return ss.str();
}

void DialogueGenerator::AddPersonality(const NPCPersonality& personality) {
    personalities_[personality.name] = personality;
}

const NPCPersonality* DialogueGenerator::GetPersonality(const std::string& npcId) const {
    auto it = personalities_.find(npcId);
    return (it != personalities_.end()) ? &it->second : nullptr;
}

void DialogueGenerator::UpdateNPCMood(const std::string& npcId, const std::string& mood) {
    npcMoods_[npcId] = mood;
}

void DialogueGenerator::RecordConversation(
    const std::string& npcId, 
    const std::string& playerId, 
    const GeneratedDialogue& dialogue) {
    
    std::string key = npcId + "_" + playerId;
    conversationHistory_[key].push_back(dialogue.text);
    
    // Keep only last 10 conversations
    if (conversationHistory_[key].size() > 10) {
        conversationHistory_[key].erase(conversationHistory_[key].begin());
    }
}

std::vector<std::string> DialogueGenerator::GetConversationHistory(
    const std::string& npcId, 
    const std::string& playerId) const {
    
    std::string key = npcId + "_" + playerId;
    auto it = conversationHistory_.find(key);
    return (it != conversationHistory_.end()) ? it->second : std::vector<std::string>{};
}

void DialogueGenerator::ResetStats() {
    stats_ = DialogueStats{};
}

} // namespace devilution::ai