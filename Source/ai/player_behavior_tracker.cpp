#include "ai/player_behavior_tracker.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <numeric>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

namespace devilution::ai {

PlayerBehaviorTracker::PlayerBehaviorTracker() = default;

PlayerBehaviorTracker::~PlayerBehaviorTracker() = default;

bool PlayerBehaviorTracker::Initialize() {
    LogVerbose("Initializing AI Player Behavior Tracker...");
    
    Reset();
    
    LogVerbose("Player Behavior Tracker initialized");
    return true;
}

void PlayerBehaviorTracker::Reset() {
    behaviorData_.clear();
    sessionHistory_.clear();
    currentSession_ = SessionSummary{};
    playerPreferences_ = PlayerPreferences{};
    stats_ = TrackingStats{};
    
    trackingEnabled_ = true;
    sessionActive_ = false;
    lastPlayerPosition_ = Point{0, 0};
    
    analysisDepth_ = 20;
    patternSensitivity_ = 1.0f;
    dataRetentionDays_ = 30;
}

void PlayerBehaviorTracker::Update(uint32_t deltaTime) {
    if (!trackingEnabled_ || !sessionActive_) {
        return;
    }
    
    // Update session duration
    auto now = std::chrono::steady_clock::now();
    currentSession_.duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - currentSession_.startTime).count();
    
    // Prune old data periodically
    static uint32_t pruneCounter = 0;
    pruneCounter += deltaTime;
    if (pruneCounter > 60000) { // Every minute
        PruneOldData();
        pruneCounter = 0;
    }
}

void PlayerBehaviorTracker::StartNewSession() {
    if (sessionActive_) {
        EndCurrentSession();
    }
    
    currentSession_ = SessionSummary{};
    currentSession_.startTime = std::chrono::steady_clock::now();
    sessionStartTime_ = currentSession_.startTime;
    sessionActive_ = true;
    
    LogVerbose("Started new behavior tracking session");
}

void PlayerBehaviorTracker::EndCurrentSession() {
    if (!sessionActive_) {
        return;
    }
    
    currentSession_.endTime = std::chrono::steady_clock::now();
    currentSession_.duration = std::chrono::duration_cast<std::chrono::seconds>(
        currentSession_.endTime - currentSession_.startTime).count();
    
    CalculateSessionMetrics();
    
    // Store session in history
    sessionHistory_.push_back(currentSession_);
    if (sessionHistory_.size() > MAX_SESSION_HISTORY) {
        sessionHistory_.erase(sessionHistory_.begin());
    }
    
    stats_.totalSessions++;
    stats_.totalPlayTime += static_cast<uint32_t>(currentSession_.duration);
    stats_.averageSessionLength = static_cast<float>(stats_.totalPlayTime) / stats_.totalSessions;
    
    sessionActive_ = false;
    
    LogVerbose("Ended behavior tracking session (duration: {:.1f}s)", currentSession_.duration);
}

void PlayerBehaviorTracker::TrackAction(const std::string& actionType, const std::string& actionData, const Player& player) {
    if (!trackingEnabled_) {
        return;
    }
    
    BehaviorDataPoint dataPoint;
    dataPoint.timestamp = std::chrono::steady_clock::now();
    dataPoint.actionType = actionType;
    dataPoint.actionData = actionData;
    dataPoint.playerPosition = Point{player.position.tile.x, player.position.tile.y};
    dataPoint.playerLevel = player.getCharacterLevel();
    
    if (sessionActive_) {
        dataPoint.sessionTime = std::chrono::duration_cast<std::chrono::seconds>(
            dataPoint.timestamp - sessionStartTime_).count();
    }
    
    behaviorData_.push_back(dataPoint);
    
    // Limit data size
    if (behaviorData_.size() > MAX_BEHAVIOR_DATA_POINTS) {
        behaviorData_.erase(behaviorData_.begin());
    }
    
    stats_.totalActionsTracked++;
    stats_.actionTypeCount[actionType]++;
    
    lastActionTime_ = dataPoint.timestamp;
}

void PlayerBehaviorTracker::RecordCombatAction(const std::string& actionData) {
    if (!sessionActive_) return;
    
    currentSession_.combatActions++;
    
    // Parse damage from actionData if available
    // This is a simplified implementation
    if (actionData.find("damage:") != std::string::npos) {
        // Extract damage value and update DPS calculation
        // For now, just increment combat actions
    }
}

void PlayerBehaviorTracker::RecordDeath(const std::string& cause) {
    if (!sessionActive_) return;
    
    currentSession_.deathCount++;
    
    LogVerbose("Player death recorded: {}", cause);
}

void PlayerBehaviorTracker::RecordLevelCompletion(const std::string& levelData) {
    if (!sessionActive_) return;
    
    currentSession_.levelsGained++;
    
    LogVerbose("Level completion recorded: {}", levelData);
}

void PlayerBehaviorTracker::RecordQuestCompletion(const std::string& questData) {
    if (!sessionActive_) return;
    
    currentSession_.questsCompleted++;
    
    LogVerbose("Quest completion recorded: {}", questData);
}

void PlayerBehaviorTracker::RecordItemInteraction(const std::string& itemData) {
    if (!sessionActive_) return;
    
    currentSession_.itemsFound++;
    
    LogVerbose("Item interaction recorded: {}", itemData);
}

void PlayerBehaviorTracker::RecordSpellCast(const std::string& spellData) {
    if (!sessionActive_) return;
    
    // Track spell usage patterns
    LogVerbose("Spell cast recorded: {}", spellData);
}

void PlayerBehaviorTracker::RecordMovement(const std::string& movementData) {
    if (!sessionActive_) return;
    
    // Track movement patterns
    LogVerbose("Movement recorded: {}", movementData);
}

void PlayerBehaviorTracker::RecordNPCInteraction(const std::string& npcData) {
    if (!sessionActive_) return;
    
    currentSession_.npcInteractions++;
    
    LogVerbose("NPC interaction recorded: {}", npcData);
}

void PlayerBehaviorTracker::TrackCombatAction(const Player& player, const std::string& actionType, float damage) {
    TrackAction("combat", actionType + ":" + std::to_string(damage), player);
    RecordCombatAction(actionType + ":" + std::to_string(damage));
}

void PlayerBehaviorTracker::TrackMovement(const Player& player, Point newPosition) {
    if (lastPlayerPosition_.x != 0 || lastPlayerPosition_.y != 0) {
        float distance = std::sqrt(
            std::pow(newPosition.x - lastPlayerPosition_.x, 2) +
            std::pow(newPosition.y - lastPlayerPosition_.y, 2)
        );
        currentSession_.totalDistance += distance;
    }
    
    lastPlayerPosition_ = newPosition;
    TrackAction("movement", "position:" + std::to_string(newPosition.x) + "," + std::to_string(newPosition.y), player);
}

void PlayerBehaviorTracker::TrackInventoryAction(const Player& player, const std::string& actionType) {
    TrackAction("inventory", actionType, player);
    currentSession_.inventoryActions++;
}

void PlayerBehaviorTracker::TrackNPCInteraction(const Player& player, const std::string& npcId, const std::string& interactionType) {
    TrackAction("npc_interaction", npcId + ":" + interactionType, player);
    RecordNPCInteraction(npcId + ":" + interactionType);
}

void PlayerBehaviorTracker::TrackQuestAction(const Player& player, const std::string& questId, const std::string& actionType) {
    TrackAction("quest", questId + ":" + actionType, player);
    if (actionType == "completed") {
        RecordQuestCompletion(questId);
    }
}

void PlayerBehaviorTracker::TrackDeath(const Player& player, const std::string& cause) {
    TrackAction("death", cause, player);
    RecordDeath(cause);
}

std::vector<BehaviorPattern> PlayerBehaviorTracker::AnalyzeBehaviorPatterns(int sessionCount) {
    std::vector<BehaviorPattern> patterns;
    
    // Analyze recent behavior data
    auto recentData = GetRecentData(30); // Last 30 minutes
    
    if (recentData.empty()) {
        return patterns;
    }
    
    // Analyze different pattern types
    BehaviorPattern combatPattern = AnalyzeCombatPattern(recentData);
    BehaviorPattern explorationPattern = AnalyzeExplorationPattern(recentData);
    BehaviorPattern progressionPattern = AnalyzeProgressionPattern(recentData);
    BehaviorPattern socialPattern = AnalyzeSocialPattern(recentData);
    
    // Add patterns with confidence above threshold
    if (GetPatternConfidence(combatPattern) >= PATTERN_CONFIDENCE_THRESHOLD) {
        patterns.push_back(combatPattern);
    }
    if (GetPatternConfidence(explorationPattern) >= PATTERN_CONFIDENCE_THRESHOLD) {
        patterns.push_back(explorationPattern);
    }
    if (GetPatternConfidence(progressionPattern) >= PATTERN_CONFIDENCE_THRESHOLD) {
        patterns.push_back(progressionPattern);
    }
    if (GetPatternConfidence(socialPattern) >= PATTERN_CONFIDENCE_THRESHOLD) {
        patterns.push_back(socialPattern);
    }
    
    return patterns;
}

PlayerPreferences PlayerBehaviorTracker::AnalyzePlayerPreferences() {
    PlayerPreferences preferences;
    
    auto recentData = GetRecentData(60); // Last hour
    if (recentData.empty()) {
        return preferences;
    }
    
    // Calculate preferences based on behavior data
    preferences.aggressionLevel = CalculateAggressionLevel(recentData);
    preferences.riskTolerance = CalculateRiskTolerance(recentData);
    preferences.explorationThoroughness = CalculateExplorationThoroughness(recentData);
    preferences.questPriority = CalculateQuestPriority(recentData);
    
    // Update stored preferences
    playerPreferences_ = preferences;
    
    return preferences;
}

BehaviorPattern PlayerBehaviorTracker::GetDominantPattern() const {
    if (sessionHistory_.empty()) {
        return BehaviorPattern::CASUAL_PLAYER;
    }
    
    // Analyze most recent session
    const auto& lastSession = sessionHistory_.back();
    
    if (lastSession.combatActions > lastSession.explorationActions * 2) {
        return BehaviorPattern::AGGRESSIVE_COMBAT;
    } else if (lastSession.explorationActions > lastSession.combatActions * 2) {
        return BehaviorPattern::EXPLORATION_FOCUSED;
    } else if (lastSession.questsCompleted > 0) {
        return BehaviorPattern::QUEST_FOCUSED;
    } else {
        return BehaviorPattern::CASUAL_PLAYER;
    }
}

float PlayerBehaviorTracker::GetPatternConfidence(BehaviorPattern pattern) const {
    // Simplified confidence calculation
    if (stats_.totalActionsTracked < 10) {
        return 0.1f; // Low confidence with little data
    } else if (stats_.totalActionsTracked < 50) {
        return 0.5f; // Medium confidence
    } else {
        return 0.8f; // High confidence with lots of data
    }
}

std::vector<std::string> PlayerBehaviorTracker::PredictNextActions(const Player& player, int count) {
    std::vector<std::string> predictions;
    
    BehaviorPattern dominantPattern = GetDominantPattern();
    
    switch (dominantPattern) {
        case BehaviorPattern::AGGRESSIVE_COMBAT:
            predictions.push_back("Seek out combat encounters");
            predictions.push_back("Use offensive spells");
            predictions.push_back("Engage multiple enemies");
            break;
            
        case BehaviorPattern::EXPLORATION_FOCUSED:
            predictions.push_back("Explore new areas");
            predictions.push_back("Search for secrets");
            predictions.push_back("Map unknown territories");
            break;
            
        case BehaviorPattern::QUEST_FOCUSED:
            predictions.push_back("Continue current quest");
            predictions.push_back("Talk to quest NPCs");
            predictions.push_back("Gather quest items");
            break;
            
        default:
            predictions.push_back("Continue current activity");
            predictions.push_back("Explore nearby areas");
            predictions.push_back("Manage inventory");
            break;
    }
    
    // Limit to requested count
    if (predictions.size() > static_cast<size_t>(count)) {
        predictions.resize(count);
    }
    
    return predictions;
}

std::vector<std::string> PlayerBehaviorTracker::GenerateRecommendations(const Player& player) {
    std::vector<std::string> recommendations;
    
    PlayerPreferences prefs = AnalyzePlayerPreferences();
    
    if (prefs.aggressionLevel < 0.3f) {
        recommendations.push_back("Consider using more offensive tactics");
    }
    if (prefs.explorationThoroughness < 0.4f) {
        recommendations.push_back("Try exploring areas more thoroughly");
    }
    if (prefs.questPriority < 0.5f) {
        recommendations.push_back("Focus on completing active quests");
    }
    
    return recommendations;
}

float PlayerBehaviorTracker::PredictSessionLength(const Player& player) {
    if (sessionHistory_.empty()) {
        return 1800.0f; // Default 30 minutes
    }
    
    // Calculate average from recent sessions
    float totalDuration = 0.0f;
    int count = std::min(5, static_cast<int>(sessionHistory_.size()));
    
    for (int i = sessionHistory_.size() - count; i < static_cast<int>(sessionHistory_.size()); ++i) {
        totalDuration += sessionHistory_[i].duration;
    }
    
    return totalDuration / count;
}

bool PlayerBehaviorTracker::PredictPlayerFrustration(const Player& player) {
    if (!sessionActive_) {
        return false;
    }
    
    // Simple frustration indicators
    if (currentSession_.deathCount > 5) {
        return true; // Many deaths indicate frustration
    }
    
    if (currentSession_.duration > 3600 && currentSession_.levelsGained == 0) {
        return true; // Long session with no progress
    }
    
    return false;
}

std::vector<SessionSummary> PlayerBehaviorTracker::GetRecentSessions(int count) const {
    std::vector<SessionSummary> recent;
    
    int startIndex = std::max(0, static_cast<int>(sessionHistory_.size()) - count);
    for (int i = startIndex; i < static_cast<int>(sessionHistory_.size()); ++i) {
        recent.push_back(sessionHistory_[i]);
    }
    
    return recent;
}

SessionSummary PlayerBehaviorTracker::GetAverageSession() const {
    if (sessionHistory_.empty()) {
        return SessionSummary{};
    }
    
    SessionSummary average{};
    
    for (const auto& session : sessionHistory_) {
        average.duration += session.duration;
        average.combatActions += session.combatActions;
        average.explorationActions += session.explorationActions;
        average.inventoryActions += session.inventoryActions;
        average.npcInteractions += session.npcInteractions;
        average.questActions += session.questActions;
        average.deathCount += session.deathCount;
        average.totalDistance += session.totalDistance;
        average.levelsGained += session.levelsGained;
        average.itemsFound += session.itemsFound;
        average.questsCompleted += session.questsCompleted;
    }
    
    size_t count = sessionHistory_.size();
    average.duration /= count;
    average.combatActions /= count;
    average.explorationActions /= count;
    average.inventoryActions /= count;
    average.npcInteractions /= count;
    average.questActions /= count;
    average.deathCount /= count;
    average.totalDistance /= count;
    average.levelsGained /= count;
    average.itemsFound /= count;
    average.questsCompleted /= count;
    
    return average;
}

void PlayerBehaviorTracker::UpdatePlayerPreferences(const PlayerPreferences& preferences) {
    playerPreferences_ = preferences;
}

void PlayerBehaviorTracker::ResetStats() {
    stats_ = TrackingStats{};
}

// Private method implementations

BehaviorPattern PlayerBehaviorTracker::AnalyzeCombatPattern(const std::vector<BehaviorDataPoint>& data) {
    int combatActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "combat") {
            combatActions++;
        }
    }
    
    if (totalActions == 0) return BehaviorPattern::CASUAL_PLAYER;
    
    float combatRatio = static_cast<float>(combatActions) / totalActions;
    
    if (combatRatio > 0.6f) {
        return BehaviorPattern::AGGRESSIVE_COMBAT;
    } else if (combatRatio > 0.3f) {
        return BehaviorPattern::DEFENSIVE_COMBAT;
    } else {
        return BehaviorPattern::CASUAL_PLAYER;
    }
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeExplorationPattern(const std::vector<BehaviorDataPoint>& data) {
    int movementActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "movement") {
            movementActions++;
        }
    }
    
    if (totalActions == 0) return BehaviorPattern::CASUAL_PLAYER;
    
    float movementRatio = static_cast<float>(movementActions) / totalActions;
    
    if (movementRatio > 0.5f) {
        return BehaviorPattern::EXPLORATION_FOCUSED;
    } else {
        return BehaviorPattern::CASUAL_PLAYER;
    }
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeProgressionPattern(const std::vector<BehaviorDataPoint>& data) {
    int questActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "quest") {
            questActions++;
        }
    }
    
    if (totalActions == 0) return BehaviorPattern::CASUAL_PLAYER;
    
    float questRatio = static_cast<float>(questActions) / totalActions;
    
    if (questRatio > 0.4f) {
        return BehaviorPattern::QUEST_FOCUSED;
    } else {
        return BehaviorPattern::CASUAL_PLAYER;
    }
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeSocialPattern(const std::vector<BehaviorDataPoint>& data) {
    int npcActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "npc_interaction") {
            npcActions++;
        }
    }
    
    if (totalActions == 0) return BehaviorPattern::CASUAL_PLAYER;
    
    float npcRatio = static_cast<float>(npcActions) / totalActions;
    
    if (npcRatio > 0.3f) {
        return BehaviorPattern::SOCIAL_PLAYER;
    } else {
        return BehaviorPattern::CASUAL_PLAYER;
    }
}

float PlayerBehaviorTracker::CalculateAggressionLevel(const std::vector<BehaviorDataPoint>& data) {
    int combatActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "combat") {
            combatActions++;
        }
    }
    
    if (totalActions == 0) return 0.5f;
    
    return std::clamp(static_cast<float>(combatActions) / totalActions * 2.0f, 0.0f, 1.0f);
}

float PlayerBehaviorTracker::CalculateRiskTolerance(const std::vector<BehaviorDataPoint>& data) {
    int deathActions = 0;
    int combatActions = 0;
    
    for (const auto& point : data) {
        if (point.actionType == "death") {
            deathActions++;
        } else if (point.actionType == "combat") {
            combatActions++;
        }
    }
    
    if (combatActions == 0) return 0.5f;
    
    float deathRate = static_cast<float>(deathActions) / combatActions;
    return std::clamp(1.0f - deathRate * 5.0f, 0.0f, 1.0f); // Lower death rate = higher risk tolerance
}

float PlayerBehaviorTracker::CalculateExplorationThoroughness(const std::vector<BehaviorDataPoint>& data) {
    int movementActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "movement") {
            movementActions++;
        }
    }
    
    if (totalActions == 0) return 0.5f;
    
    return std::clamp(static_cast<float>(movementActions) / totalActions * 1.5f, 0.0f, 1.0f);
}

float PlayerBehaviorTracker::CalculateQuestPriority(const std::vector<BehaviorDataPoint>& data) {
    int questActions = 0;
    int totalActions = data.size();
    
    for (const auto& point : data) {
        if (point.actionType == "quest") {
            questActions++;
        }
    }
    
    if (totalActions == 0) return 0.5f;
    
    return std::clamp(static_cast<float>(questActions) / totalActions * 2.5f, 0.0f, 1.0f);
}

std::vector<std::string> PlayerBehaviorTracker::PredictBasedOnPattern(BehaviorPattern pattern, const Player& player) {
    std::vector<std::string> predictions;
    
    switch (pattern) {
        case BehaviorPattern::AGGRESSIVE_COMBAT:
            predictions = {"attack", "cast_offensive_spell", "charge_enemy"};
            break;
        case BehaviorPattern::EXPLORATION_FOCUSED:
            predictions = {"move_to_unexplored", "search_area", "check_secrets"};
            break;
        case BehaviorPattern::QUEST_FOCUSED:
            predictions = {"continue_quest", "talk_to_npc", "collect_quest_item"};
            break;
        default:
            predictions = {"continue_current", "explore", "manage_inventory"};
            break;
    }
    
    return predictions;
}

float PlayerBehaviorTracker::CalculateFrustrationScore(const Player& player) {
    if (!sessionActive_) {
        return 0.0f;
    }
    
    float frustration = 0.0f;
    
    // Death count contributes to frustration
    frustration += currentSession_.deathCount * 0.2f;
    
    // Long session with no progress
    if (currentSession_.duration > 1800 && currentSession_.levelsGained == 0) {
        frustration += 0.3f;
    }
    
    // No quest progress
    if (currentSession_.duration > 900 && currentSession_.questsCompleted == 0) {
        frustration += 0.2f;
    }
    
    return std::clamp(frustration, 0.0f, 1.0f);
}

void PlayerBehaviorTracker::PruneOldData() {
    auto now = std::chrono::steady_clock::now();
    auto cutoffTime = now - std::chrono::hours(24 * dataRetentionDays_);
    
    behaviorData_.erase(
        std::remove_if(behaviorData_.begin(), behaviorData_.end(),
            [cutoffTime](const BehaviorDataPoint& point) {
                return point.timestamp < cutoffTime;
            }),
        behaviorData_.end()
    );
}

void PlayerBehaviorTracker::UpdateCurrentSession(const Player& player) {
    if (!sessionActive_) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    currentSession_.duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - currentSession_.startTime).count();
}

void PlayerBehaviorTracker::CalculateSessionMetrics() {
    // Calculate derived metrics from raw data
    if (currentSession_.duration > 0) {
        currentSession_.averageSpeed = currentSession_.totalDistance / currentSession_.duration;
    }
    
    // Analyze dominant patterns for this session
    auto recentData = GetRecentData(static_cast<int>(currentSession_.duration / 60)); // Session data
    currentSession_.dominantPatterns = AnalyzeBehaviorPatterns(1);
    
    if (!currentSession_.dominantPatterns.empty()) {
        currentSession_.patternConfidence = GetPatternConfidence(currentSession_.dominantPatterns[0]);
    }
}

std::vector<BehaviorDataPoint> PlayerBehaviorTracker::GetRecentData(int minutes) const {
    std::vector<BehaviorDataPoint> recent;
    
    auto now = std::chrono::steady_clock::now();
    auto cutoffTime = now - std::chrono::minutes(minutes);
    
    for (const auto& point : behaviorData_) {
        if (point.timestamp >= cutoffTime) {
            recent.push_back(point);
        }
    }
    
    return recent;
}

std::vector<BehaviorDataPoint> PlayerBehaviorTracker::GetDataByType(const std::string& actionType) const {
    std::vector<BehaviorDataPoint> filtered;
    
    for (const auto& point : behaviorData_) {
        if (point.actionType == actionType) {
            filtered.push_back(point);
        }
    }
    
    return filtered;
}

float PlayerBehaviorTracker::CalculateActionFrequency(const std::string& actionType, int timeWindowMinutes) const {
    auto recentData = GetRecentData(timeWindowMinutes);
    
    int actionCount = 0;
    for (const auto& point : recentData) {
        if (point.actionType == actionType) {
            actionCount++;
        }
    }
    
    return static_cast<float>(actionCount) / timeWindowMinutes; // Actions per minute
}

} // namespace devilution::ai