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
    
    sessionActive_ = false;
    lastPlayerPosition_ = Point{0, 0};
}

void PlayerBehaviorTracker::StartNewSession() {
    if (sessionActive_) {
        EndCurrentSession();
    }
    
    sessionStartTime_ = std::chrono::steady_clock::now();
    lastActionTime_ = sessionStartTime_;
    sessionActive_ = true;
    
    currentSession_ = SessionSummary{};
    currentSession_.startTime = sessionStartTime_;
    
    LogVerbose("Started new behavior tracking session");
}

void PlayerBehaviorTracker::EndCurrentSession() {
    if (!sessionActive_) {
        return;
    }
    
    auto endTime = std::chrono::steady_clock::now();
    currentSession_.endTime = endTime;
    currentSession_.duration = std::chrono::duration_cast<std::chrono::seconds>(
        endTime - sessionStartTime_).count();
    
    // Calculate session metrics
    CalculateSessionMetrics();
    
    // Analyze dominant patterns
    currentSession_.dominantPatterns = AnalyzeBehaviorPatterns(1);
    if (!currentSession_.dominantPatterns.empty()) {
        currentSession_.patternConfidence = GetPatternConfidence(currentSession_.dominantPatterns[0]);
    }
    
    // Store session in history
    sessionHistory_.push_back(currentSession_);
    if (sessionHistory_.size() > MAX_SESSION_HISTORY) {
        sessionHistory_.erase(sessionHistory_.begin());
    }
    
    // Update statistics
    stats_.totalSessions++;
    stats_.totalPlayTime += static_cast<uint32_t>(currentSession_.duration);
    stats_.averageSessionLength = static_cast<float>(stats_.totalPlayTime) / stats_.totalSessions;
    
    sessionActive_ = false;
    
    LogVerbose("Ended behavior tracking session (duration: {:.1f}s, patterns: {})", 
               currentSession_.duration, currentSession_.dominantPatterns.size());
}

void PlayerBehaviorTracker::TrackAction(
    const std::string& actionType, 
    const std::string& actionData, 
    const Player& player) {
    
    if (!trackingEnabled_ || !sessionActive_) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    
    BehaviorDataPoint dataPoint;
    dataPoint.timestamp = now;
    dataPoint.actionType = actionType;
    dataPoint.actionData = actionData;
    dataPoint.playerPosition = Point{player.position.tile.x, player.position.tile.y};
    dataPoint.playerLevel = player._pLevel;
    dataPoint.sessionTime = std::chrono::duration_cast<std::chrono::seconds>(
        now - sessionStartTime_).count();
    
    // Add contextual data
    dataPoint.contextData["health_percent"] = static_cast<float>(player._pHitPoints) / player._pMaxHP;
    dataPoint.contextData["mana_percent"] = static_cast<float>(player._pMana) / player._pMaxMana;
    dataPoint.contextData["experience"] = static_cast<float>(player._pExperience);
    
    behaviorData_.push_back(dataPoint);
    
    // Prune old data if necessary
    if (behaviorData_.size() > MAX_BEHAVIOR_DATA_POINTS) {
        PruneOldData();
    }
    
    lastActionTime_ = now;
    stats_.totalActionsTracked++;
    stats_.actionTypeCount[actionType]++;
    
    // Update current session
    UpdateCurrentSession(player);
}

void PlayerBehaviorTracker::TrackCombatAction(
    const Player& player, 
    const std::string& actionType, 
    float damage) {
    
    std::string actionData = actionType + ":" + std::to_string(damage);
    TrackAction("combat", actionData, player);
    
    currentSession_.combatActions++;
    
    if (actionType == "deal_damage") {
        currentSession_.averageDPS = (currentSession_.averageDPS * 0.9f) + (damage * 0.1f);
    } else if (actionType == "receive_damage") {
        currentSession_.damageReceived += damage;
    } else if (actionType == "heal") {
        currentSession_.healingUsed += damage;
    }
}

void PlayerBehaviorTracker::TrackMovement(const Player& player, Point newPosition) {
    if (!trackingEnabled_ || !sessionActive_) {
        return;
    }
    
    if (lastPlayerPosition_.x != 0 || lastPlayerPosition_.y != 0) {
        float distance = std::sqrt(
            std::pow(newPosition.x - lastPlayerPosition_.x, 2) +
            std::pow(newPosition.y - lastPlayerPosition_.y, 2)
        );
        
        currentSession_.totalDistance += distance;
        
        // Calculate movement speed
        auto now = std::chrono::steady_clock::now();
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastActionTime_).count();
        
        if (timeDiff > 0) {
            float speed = distance / (timeDiff / 1000.0f);
            currentSession_.averageSpeed = (currentSession_.averageSpeed * 0.9f) + (speed * 0.1f);
        }
        
        // Detect backtracking (simplified)
        if (distance < 2.0f && timeDiff > 1000) {
            currentSession_.backtrackingCount++;
        }
    }
    
    lastPlayerPosition_ = newPosition;
    TrackAction("movement", "move", player);
}

void PlayerBehaviorTracker::TrackInventoryAction(const Player& player, const std::string& actionType) {
    TrackAction("inventory", actionType, player);
    currentSession_.inventoryActions++;
}

void PlayerBehaviorTracker::TrackNPCInteraction(
    const Player& player, 
    const std::string& npcId, 
    const std::string& interactionType) {
    
    std::string actionData = npcId + ":" + interactionType;
    TrackAction("npc_interaction", actionData, player);
    currentSession_.npcInteractions++;
}

void PlayerBehaviorTracker::TrackQuestAction(
    const Player& player, 
    const std::string& questId, 
    const std::string& actionType) {
    
    std::string actionData = questId + ":" + actionType;
    TrackAction("quest", actionData, player);
    currentSession_.questActions++;
    
    if (actionType == "complete") {
        currentSession_.questsCompleted++;
    }
}

void PlayerBehaviorTracker::TrackDeath(const Player& player, const std::string& cause) {
    TrackAction("death", cause, player);
    currentSession_.deathCount++;
}

std::vector<BehaviorPattern> PlayerBehaviorTracker::AnalyzeBehaviorPatterns(int sessionCount) {
    std::vector<BehaviorPattern> patterns;
    
    if (behaviorData_.empty()) {
        return patterns;
    }
    
    // Get recent data for analysis
    std::vector<BehaviorDataPoint> recentData = GetRecentData(sessionCount * 30); // 30 minutes per session
    
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
    
    // Update statistics
    for (const auto& pattern : patterns) {
        stats_.patternFrequency[pattern]++;
    }
    
    return patterns;
}

PlayerPreferences PlayerBehaviorTracker::AnalyzePlayerPreferences() {
    PlayerPreferences preferences;
    
    if (behaviorData_.empty()) {
        return preferences;
    }
    
    std::vector<BehaviorDataPoint> recentData = GetRecentData(60); // Last hour
    
    // Analyze aggression level
    preferences.aggressionLevel = CalculateAggressionLevel(recentData);
    
    // Analyze risk tolerance
    preferences.riskTolerance = CalculateRiskTolerance(recentData);
    
    // Analyze exploration thoroughness
    preferences.explorationThoroughness = CalculateExplorationThoroughness(recentData);
    
    // Analyze quest priority
    preferences.questPriority = CalculateQuestPriority(recentData);
    
    // Calculate other preferences based on action frequencies
    float combatFreq = CalculateActionFrequency("combat", 30);
    float explorationFreq = CalculateActionFrequency("movement", 30);
    float npcFreq = CalculateActionFrequency("npc_interaction", 30);
    float inventoryFreq = CalculateActionFrequency("inventory", 30);
    
    preferences.combatComplexity = std::clamp(combatFreq / 10.0f, 0.0f, 1.0f);
    preferences.backtrackingTolerance = 1.0f - std::clamp(currentSession_.backtrackingCount / 20.0f, 0.0f, 1.0f);
    preferences.npcInteraction = std::clamp(npcFreq / 5.0f, 0.0f, 1.0f);
    preferences.gearOptimization = std::clamp(inventoryFreq / 15.0f, 0.0f, 1.0f);
    
    // Update stored preferences
    playerPreferences_ = preferences;
    
    return preferences;
}

BehaviorPattern PlayerBehaviorTracker::GetDominantPattern() const {
    auto patterns = const_cast<PlayerBehaviorTracker*>(this)->AnalyzeBehaviorPatterns(3);
    
    if (patterns.empty()) {
        return BehaviorPattern::CASUAL_PLAYER;
    }
    
    // Return pattern with highest confidence
    BehaviorPattern dominant = patterns[0];
    float maxConfidence = GetPatternConfidence(dominant);
    
    for (const auto& pattern : patterns) {
        float confidence = GetPatternConfidence(pattern);
        if (confidence > maxConfidence) {
            maxConfidence = confidence;
            dominant = pattern;
        }
    }
    
    return dominant;
}

float PlayerBehaviorTracker::GetPatternConfidence(BehaviorPattern pattern) const {
    // Calculate confidence based on data consistency and amount
    float baseConfidence = 0.5f;
    
    // More data = higher confidence
    if (behaviorData_.size() > 100) {
        baseConfidence += 0.3f;
    } else if (behaviorData_.size() > 50) {
        baseConfidence += 0.2f;
    } else if (behaviorData_.size() > 20) {
        baseConfidence += 0.1f;
    }
    
    // Pattern-specific confidence adjustments
    switch (pattern) {
        case BehaviorPattern::AGGRESSIVE_COMBAT:
            if (currentSession_.combatActions > currentSession_.explorationActions) {
                baseConfidence += 0.2f;
            }
            break;
            
        case BehaviorPattern::EXPLORATION_FOCUSED:
            if (currentSession_.explorationActions > currentSession_.combatActions) {
                baseConfidence += 0.2f;
            }
            break;
            
        case BehaviorPattern::QUEST_FOCUSED:
            if (currentSession_.questActions > 5) {
                baseConfidence += 0.2f;
            }
            break;
            
        case BehaviorPattern::SOCIAL_PLAYER:
            if (currentSession_.npcInteractions > 10) {
                baseConfidence += 0.2f;
            }
            break;
            
        default:
            break;
    }
    
    return std::clamp(baseConfidence, 0.0f, 1.0f);
}

std::vector<std::string> PlayerBehaviorTracker::PredictNextActions(const Player& player, int count) {
    std::vector<std::string> predictions;
    
    BehaviorPattern dominantPattern = GetDominantPattern();
    predictions = PredictBasedOnPattern(dominantPattern, player);
    
    // Limit to requested count
    if (predictions.size() > static_cast<size_t>(count)) {
        predictions.resize(count);
    }
    
    return predictions;
}

std::vector<std::string> PlayerBehaviorTracker::GenerateRecommendations(const Player& player) {
    std::vector<std::string> recommendations;
    
    PlayerPreferences prefs = AnalyzePlayerPreferences();
    
    // Generate recommendations based on preferences and current state
    if (prefs.aggressionLevel > 0.7f && currentSession_.deathCount > 3) {
        recommendations.push_back("Consider a more defensive approach - you've died several times.");
    }
    
    if (prefs.explorationThoroughness < 0.3f) {
        recommendations.push_back("Try exploring more thoroughly - you might find hidden treasures.");
    }
    
    if (prefs.questPriority < 0.4f && currentSession_.questsCompleted == 0) {
        recommendations.push_back("Consider focusing on quests for better rewards and progression.");
    }
    
    if (prefs.npcInteraction < 0.3f) {
        recommendations.push_back("Talk to NPCs more often - they might have valuable information or quests.");
    }
    
    if (currentSession_.averageDPS < player._pLevel * 5) {
        recommendations.push_back("Your damage output seems low - consider upgrading your weapon.");
    }
    
    return recommendations;
}

float PlayerBehaviorTracker::PredictSessionLength(const Player& player) {
    if (sessionHistory_.empty()) {
        return 1800.0f; // Default 30 minutes
    }
    
    // Calculate average from recent sessions
    float totalDuration = 0.0f;
    int sessionCount = std::min(5, static_cast<int>(sessionHistory_.size()));
    
    for (int i = sessionHistory_.size() - sessionCount; i < static_cast<int>(sessionHistory_.size()); ++i) {
        totalDuration += sessionHistory_[i].duration;
    }
    
    return totalDuration / sessionCount;
}

bool PlayerBehaviorTracker::PredictPlayerFrustration(const Player& player) {
    float frustrationScore = CalculateFrustrationScore(player);
    return frustrationScore > 0.7f;
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeCombatPattern(const std::vector<BehaviorDataPoint>& data) {
    int combatActions = 0;
    int aggressiveActions = 0;
    
    for (const auto& point : data) {
        if (point.actionType == "combat") {
            combatActions++;
            if (point.actionData.find("attack") != std::string::npos) {
                aggressiveActions++;
            }
        }
    }
    
    if (combatActions > data.size() * 0.6f) {
        if (aggressiveActions > combatActions * 0.7f) {
            return BehaviorPattern::AGGRESSIVE_COMBAT;
        } else {
            return BehaviorPattern::DEFENSIVE_COMBAT;
        }
    }
    
    return BehaviorPattern::CASUAL_PLAYER;
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeExplorationPattern(const std::vector<BehaviorDataPoint>& data) {
    int movementActions = 0;
    int explorationActions = 0;
    
    for (const auto& point : data) {
        if (point.actionType == "movement") {
            movementActions++;
        } else if (point.actionType == "exploration") {
            explorationActions++;
        }
    }
    
    if (movementActions > data.size() * 0.5f) {
        return BehaviorPattern::EXPLORATION_FOCUSED;
    }
    
    return BehaviorPattern::CASUAL_PLAYER;
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeProgressionPattern(const std::vector<BehaviorDataPoint>& data) {
    int questActions = 0;
    int speedrunActions = 0;
    
    for (const auto& point : data) {
        if (point.actionType == "quest") {
            questActions++;
        }
        // Detect speedrun behavior (fast completion times)
        if (point.sessionTime < 600 && point.actionType == "level_complete") { // 10 minutes
            speedrunActions++;
        }
    }
    
    if (questActions > data.size() * 0.3f) {
        return BehaviorPattern::QUEST_FOCUSED;
    }
    
    if (speedrunActions > 0) {
        return BehaviorPattern::SPEEDRUNNER;
    }
    
    return BehaviorPattern::CASUAL_PLAYER;
}

BehaviorPattern PlayerBehaviorTracker::AnalyzeSocialPattern(const std::vector<BehaviorDataPoint>& data) {
    int npcInteractions = 0;
    
    for (const auto& point : data) {
        if (point.actionType == "npc_interaction") {
            npcInteractions++;
        }
    }
    
    if (npcInteractions > data.size() * 0.2f) {
        return BehaviorPattern::SOCIAL_PLAYER;
    }
    
    return BehaviorPattern::CASUAL_PLAYER;
}

float PlayerBehaviorTracker::CalculateAggressionLevel(const std::vector<BehaviorDataPoint>& data) {
    int totalCombat = 0;
    int aggressiveActions = 0;
    
    for (const auto& point : data) {
        if (point.actionType == "combat") {
            totalCombat++;
            if (point.actionData.find("attack") != std::string::npos ||
                point.actionData.find("charge") != std::string::npos) {
                aggressiveActions++;
            }
        }
    }
    
    if (totalCombat == 0) return 0.5f;
    
    return std::clamp(static_cast<float>(aggressiveActions) / totalCombat, 0.0f, 1.0f);
}

float PlayerBehaviorTracker::CalculateRiskTolerance(const std::vector<BehaviorDataPoint>& data) {
    // Higher death count and continued aggressive play = higher risk tolerance
    float riskScore = 0.5f;
    
    if (currentSession_.deathCount == 0) {
        riskScore += 0.2f; // Cautious play
    } else if (currentSession_.deathCount > 3) {
        riskScore -= 0.3f; // High death count suggests low tolerance or poor play
    }
    
    // Factor in health management
    float avgHealthPercent = 0.0f;
    int healthSamples = 0;
    
    for (const auto& point : data) {
        auto healthIt = point.contextData.find("health_percent");
        if (healthIt != point.contextData.end()) {
            avgHealthPercent += healthIt->second;
            healthSamples++;
        }
    }
    
    if (healthSamples > 0) {
        avgHealthPercent /= healthSamples;
        if (avgHealthPercent < 0.3f) {
            riskScore += 0.2f; // Playing at low health = high risk tolerance
        }
    }
    
    return std::clamp(riskScore, 0.0f, 1.0f);
}

float PlayerBehaviorTracker::CalculateExplorationThoroughness(const std::vector<BehaviorDataPoint>& data) {
    // Based on backtracking and movement patterns
    float thoroughness = 0.5f;
    
    if (currentSession_.backtrackingCount > 10) {
        thoroughness += 0.3f; // Lots of backtracking = thorough exploration
    } else if (currentSession_.backtrackingCount < 3) {
        thoroughness -= 0.2f; // Little backtracking = rushing through
    }
    
    return std::clamp(thoroughness, 0.0f, 1.0f);
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
    
    return std::clamp(static_cast<float>(questActions) / totalActions * 5.0f, 0.0f, 1.0f);
}

std::vector<std::string> PlayerBehaviorTracker::PredictBasedOnPattern(
    BehaviorPattern pattern, 
    const Player& player) {
    
    std::vector<std::string> predictions;
    
    switch (pattern) {
        case BehaviorPattern::AGGRESSIVE_COMBAT:
            predictions = {"Attack nearest enemy", "Use combat spell", "Charge forward"};
            break;
            
        case BehaviorPattern::DEFENSIVE_COMBAT:
            predictions = {"Retreat to safe distance", "Use healing potion", "Cast defensive spell"};
            break;
            
        case BehaviorPattern::EXPLORATION_FOCUSED:
            predictions = {"Explore unexplored area", "Search for secrets", "Check all corners"};
            break;
            
        case BehaviorPattern::QUEST_FOCUSED:
            predictions = {"Continue current quest", "Talk to quest NPC", "Go to quest location"};
            break;
            
        case BehaviorPattern::SOCIAL_PLAYER:
            predictions = {"Talk to nearby NPC", "Visit town", "Check for new dialogue"};
            break;
            
        case BehaviorPattern::LOOT_FOCUSED:
            predictions = {"Check inventory", "Identify items", "Visit shop"};
            break;
            
        default:
            predictions = {"Continue playing", "Explore area", "Check status"};
            break;
    }
    
    return predictions;
}

float PlayerBehaviorTracker::CalculateFrustrationScore(const Player& player) {
    float frustration = 0.0f;
    
    // High death count increases frustration
    if (currentSession_.deathCount > 5) {
        frustration += 0.4f;
    } else if (currentSession_.deathCount > 2) {
        frustration += 0.2f;
    }
    
    // Low progress increases frustration
    if (currentSession_.questsCompleted == 0 && currentSession_.duration > 1800) { // 30 minutes
        frustration += 0.3f;
    }
    
    // Repetitive actions might indicate being stuck
    float recentActionVariety = CalculateActionFrequency("movement", 5) / 
                               std::max(1.0f, CalculateActionFrequency("combat", 5));
    if (recentActionVariety > 5.0f) { // Lots of movement, little combat
        frustration += 0.2f;
    }
    
    return std::clamp(frustration, 0.0f, 1.0f);
}

void PlayerBehaviorTracker::PruneOldData() {
    auto cutoffTime = std::chrono::steady_clock::now() - 
                     std::chrono::hours(24 * dataRetentionDays_);
    
    behaviorData_.erase(
        std::remove_if(behaviorData_.begin(), behaviorData_.end(),
            [cutoffTime](const BehaviorDataPoint& point) {
                return point.timestamp < cutoffTime;
            }),
        behaviorData_.end()
    );
}

void PlayerBehaviorTracker::UpdateCurrentSession(const Player& player) {
    // Update session metrics based on current player state
    currentSession_.levelsGained = std::max(0, player._pLevel - 1); // Simplified
    
    // Update other metrics as needed
}

void PlayerBehaviorTracker::CalculateSessionMetrics() {
    // Calculate final session metrics
    if (currentSession_.duration > 0) {
        currentSession_.averageSpeed = currentSession_.totalDistance / currentSession_.duration;
    }
}

std::vector<BehaviorDataPoint> PlayerBehaviorTracker::GetRecentData(int minutes) const {
    auto cutoffTime = std::chrono::steady_clock::now() - std::chrono::minutes(minutes);
    
    std::vector<BehaviorDataPoint> recentData;
    for (const auto& point : behaviorData_) {
        if (point.timestamp >= cutoffTime) {
            recentData.push_back(point);
        }
    }
    
    return recentData;
}

std::vector<BehaviorDataPoint> PlayerBehaviorTracker::GetDataByType(const std::string& actionType) const {
    std::vector<BehaviorDataPoint> filteredData;
    
    for (const auto& point : behaviorData_) {
        if (point.actionType == actionType) {
            filteredData.push_back(point);
        }
    }
    
    return filteredData;
}

float PlayerBehaviorTracker::CalculateActionFrequency(const std::string& actionType, int timeWindowMinutes) const {
    auto recentData = GetRecentData(timeWindowMinutes);
    
    int count = 0;
    for (const auto& point : recentData) {
        if (point.actionType == actionType) {
            count++;
        }
    }
    
    return static_cast<float>(count);
}

SessionSummary PlayerBehaviorTracker::GetAverageSession() const {
    if (sessionHistory_.empty()) {
        return SessionSummary{};
    }
    
    SessionSummary average;
    
    for (const auto& session : sessionHistory_) {
        average.duration += session.duration;
        average.combatActions += session.combatActions;
        average.explorationActions += session.explorationActions;
        average.inventoryActions += session.inventoryActions;
        average.npcInteractions += session.npcInteractions;
        average.questActions += session.questActions;
        average.deathCount += session.deathCount;
        average.totalDistance += session.totalDistance;
        average.backtrackingCount += session.backtrackingCount;
        average.averageSpeed += session.averageSpeed;
        average.averageDPS += session.averageDPS;
        average.damageReceived += session.damageReceived;
        average.healingUsed += session.healingUsed;
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
    average.backtrackingCount /= count;
    average.averageSpeed /= count;
    average.averageDPS /= count;
    average.damageReceived /= count;
    average.healingUsed /= count;
    average.levelsGained /= count;
    average.itemsFound /= count;
    average.questsCompleted /= count;
    
    return average;
}

std::vector<SessionSummary> PlayerBehaviorTracker::GetRecentSessions(int count) const {
    std::vector<SessionSummary> recent;
    
    int startIndex = std::max(0, static_cast<int>(sessionHistory_.size()) - count);
    for (int i = startIndex; i < static_cast<int>(sessionHistory_.size()); ++i) {
        recent.push_back(sessionHistory_[i]);
    }
    
    return recent;
}

void PlayerBehaviorTracker::ResetStats() {
    stats_ = TrackingStats{};
}

} // namespace devilution::ai