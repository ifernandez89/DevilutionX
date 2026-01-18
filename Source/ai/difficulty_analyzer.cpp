#include "ai/difficulty_analyzer.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

namespace devilution::ai {

DifficultyAnalyzer::DifficultyAnalyzer() {
    sessionStart_ = std::chrono::steady_clock::now();
    lastAnalysis_ = sessionStart_;
}

DifficultyAnalyzer::~DifficultyAnalyzer() = default;

bool DifficultyAnalyzer::Initialize() {
    LogVerbose("Initializing AI Difficulty Analyzer...");
    
    // Reset all metrics
    Reset();
    
    LogVerbose("Difficulty Analyzer initialized");
    return true;
}

void DifficultyAnalyzer::Reset() {
    currentMetrics_ = PlayerPerformanceMetrics{};
    historicalAverage_ = PlayerPerformanceMetrics{};
    metricsHistory_.clear();
    stats_ = AnalysisStats{};
    
    sessionStart_ = std::chrono::steady_clock::now();
    lastAnalysis_ = sessionStart_;
    lastPlayerPosition_ = Point{0, 0};
    totalDistance_ = 0.0f;
}

void DifficultyAnalyzer::UpdatePlayerMetrics(const Player& player) {
    auto now = std::chrono::steady_clock::now();
    auto sessionDuration = std::chrono::duration_cast<std::chrono::seconds>(now - sessionStart_).count();
    
    currentMetrics_.totalPlayTime = static_cast<float>(sessionDuration);
    currentMetrics_.lastUpdate = now;
    
    // Update gear score based on equipment
    float gearScore = 0.0f;
    for (int i = 0; i < NUM_INVLOC; ++i) {
        const Item& item = player.InvBody[i];
        if (!item.isEmpty()) {
            // Simple gear scoring based on item level and type
            gearScore += item._iIvalue * 0.1f;
        }
    }
    currentMetrics_.gearScore = gearScore;
    
    // Calculate mana efficiency
    if (player._pMaxMana > 0) {
        currentMetrics_.manaEfficiency = static_cast<float>(player._pMana) / player._pMaxMana;
    }
}

void DifficultyAnalyzer::RecordCombatEvent(const Player& player, const Monster* monster, float damage, bool playerDealt) {
    if (playerDealt) {
        // Player dealt damage
        currentMetrics_.averageDamagePerSecond = 
            (currentMetrics_.averageDamagePerSecond * 0.9f) + (damage * 0.1f);
    } else {
        // Player received damage
        currentMetrics_.damageReceived += damage;
    }
    
    if (monster && !monster->_mDelFlag) {
        // Monster was killed by player
        if (playerDealt && monster->_mhitpoints <= 0) {
            currentMetrics_.monstersKilled++;
        }
    }
}

void DifficultyAnalyzer::RecordDeath(const Player& player, const Monster* killer) {
    currentMetrics_.deathCount++;
    
    LogVerbose("Player death recorded. Total deaths: {}", currentMetrics_.deathCount);
}

void DifficultyAnalyzer::RecordLevelCompletion(const Player& player, int level, float completionTime) {
    // Update average completion time
    if (currentMetrics_.averageLevelCompletionTime == 0.0f) {
        currentMetrics_.averageLevelCompletionTime = completionTime;
    } else {
        currentMetrics_.averageLevelCompletionTime = 
            (currentMetrics_.averageLevelCompletionTime * 0.8f) + (completionTime * 0.2f);
    }
    
    LogVerbose("Level {} completed in {:.1f} seconds", level, completionTime);
}

void DifficultyAnalyzer::RecordQuestCompletion(const Player& player, bool success) {
    if (success) {
        currentMetrics_.questsCompleted++;
    } else {
        currentMetrics_.questsFailed++;
    }
}

void DifficultyAnalyzer::RecordItemEvent(const Player& player, bool found, bool equipped) {
    if (found) {
        currentMetrics_.itemsFound++;
    }
    if (equipped) {
        currentMetrics_.itemsEquipped++;
    }
}

void DifficultyAnalyzer::RecordSpellCast(const Player& player, int spellId, bool hit) {
    currentMetrics_.spellUsageCount[spellId]++;
    
    // Update spell accuracy
    int totalCasts = 0;
    int totalHits = 0;
    for (const auto& [spell, count] : currentMetrics_.spellUsageCount) {
        totalCasts += count;
        if (hit) totalHits++; // Simplified - in real implementation, track per spell
    }
    
    if (totalCasts > 0) {
        currentMetrics_.averageSpellAccuracy = static_cast<float>(totalHits) / totalCasts;
    }
}

void DifficultyAnalyzer::RecordMovement(const Player& player, Point newPosition) {
    if (lastPlayerPosition_.x != 0 || lastPlayerPosition_.y != 0) {
        float distance = std::sqrt(
            std::pow(newPosition.x - lastPlayerPosition_.x, 2) +
            std::pow(newPosition.y - lastPlayerPosition_.y, 2)
        );
        totalDistance_ += distance;
        
        // Calculate movement speed (simplified)
        auto now = std::chrono::steady_clock::now();
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - currentMetrics_.lastUpdate).count();
        
        if (timeDiff > 0) {
            float speed = distance / (timeDiff / 1000.0f);
            currentMetrics_.averageMovementSpeed = 
                (currentMetrics_.averageMovementSpeed * 0.9f) + (speed * 0.1f);
        }
    }
    
    lastPlayerPosition_ = newPosition;
}

DifficultyAdjustment DifficultyAnalyzer::AnalyzeDifficulty(const Player& player) {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastAnalysis = std::chrono::duration_cast<std::chrono::seconds>(
        now - lastAnalysis_).count();
    
    if (timeSinceLastAnalysis < analysisInterval_) {
        return DifficultyAdjustment{}; // No adjustment needed yet
    }
    
    lastAnalysis_ = now;
    stats_.totalAnalyses++;
    
    // Update current metrics
    UpdatePlayerMetrics(player);
    
    // Perform analysis
    DifficultyAdjustment adjustment;
    
    // Analyze different aspects of performance
    float combatScore = AnalyzeCombatPerformance(player);
    float progressionScore = AnalyzeProgressionRate(player);
    float resourceScore = AnalyzeResourceManagement(player);
    float skillScore = AnalyzeSkillUsage(player);
    float movementScore = AnalyzeMovementEfficiency(player);
    
    // Calculate overall performance score
    float overallScore = (combatScore * 0.3f) + (progressionScore * 0.25f) + 
                        (resourceScore * 0.2f) + (skillScore * 0.15f) + (movementScore * 0.1f);
    
    // Determine adjustment type based on performance
    if (overallScore > 0.8f) {
        // Player performing very well - increase difficulty
        adjustment.adjustmentType = DifficultyAdjustment::Type::INCREASE_MAJOR;
        adjustment.suggestedMultiplier = 1.3f;
        adjustment.reasoning = "Excellent performance detected - increasing challenge";
    } else if (overallScore > 0.6f) {
        // Player performing well - minor increase
        adjustment.adjustmentType = DifficultyAdjustment::Type::INCREASE_MINOR;
        adjustment.suggestedMultiplier = 1.1f;
        adjustment.reasoning = "Good performance - slight difficulty increase";
    } else if (overallScore < 0.3f) {
        // Player struggling - decrease difficulty
        adjustment.adjustmentType = DifficultyAdjustment::Type::DECREASE_MAJOR;
        adjustment.suggestedMultiplier = 0.7f;
        adjustment.reasoning = "Performance below expectations - reducing difficulty";
    } else if (overallScore < 0.5f) {
        // Player having some difficulty - minor decrease
        adjustment.adjustmentType = DifficultyAdjustment::Type::DECREASE_MINOR;
        adjustment.suggestedMultiplier = 0.9f;
        adjustment.reasoning = "Some difficulty detected - minor adjustment";
    } else {
        // Performance is balanced
        adjustment.adjustmentType = DifficultyAdjustment::Type::NONE;
        adjustment.suggestedMultiplier = 1.0f;
        adjustment.reasoning = "Performance is well balanced";
    }
    
    // Calculate confidence based on data quality
    adjustment.confidenceLevel = CalculateConfidence(adjustment);
    
    // Apply sensitivity modifier
    if (adjustment.adjustmentType != DifficultyAdjustment::Type::NONE) {
        float sensitivityFactor = (adjustment.suggestedMultiplier - 1.0f) * sensitivity_;
        adjustment.suggestedMultiplier = 1.0f + sensitivityFactor;
        adjustment.suggestedMultiplier = std::clamp(adjustment.suggestedMultiplier, 
                                                   MIN_DIFFICULTY_MULTIPLIER, 
                                                   MAX_DIFFICULTY_MULTIPLIER);
    }
    
    // Generate specific recommendations
    adjustment.specificRecommendations = GenerateSpecificRecommendations(adjustment, player);
    
    // Update statistics
    if (adjustment.adjustmentType != DifficultyAdjustment::Type::NONE) {
        stats_.adjustmentRecommendations++;
        if (adjustment.adjustmentType == DifficultyAdjustment::Type::INCREASE_MAJOR ||
            adjustment.adjustmentType == DifficultyAdjustment::Type::DECREASE_MAJOR) {
            stats_.majorAdjustments++;
        } else {
            stats_.minorAdjustments++;
        }
    }
    
    stats_.averageConfidence = (stats_.averageConfidence * (stats_.totalAnalyses - 1) + 
                               adjustment.confidenceLevel) / stats_.totalAnalyses;
    stats_.averageDifficultyMultiplier = (stats_.averageDifficultyMultiplier * (stats_.totalAnalyses - 1) + 
                                         adjustment.suggestedMultiplier) / stats_.totalAnalyses;
    
    // Store metrics for historical analysis
    metricsHistory_.push_back(currentMetrics_);
    if (metricsHistory_.size() > MAX_METRICS_HISTORY) {
        metricsHistory_.erase(metricsHistory_.begin());
    }
    UpdateHistoricalAverage();
    
    LogVerbose("Difficulty analysis complete: {} (confidence: {:.2f}, multiplier: {:.2f})",
               adjustment.reasoning, adjustment.confidenceLevel, adjustment.suggestedMultiplier);
    
    return adjustment;
}

PlayerSkillLevel DifficultyAnalyzer::AssessPlayerSkill(const Player& player) {
    float skillScore = CalculateSkillScore(currentMetrics_);
    PlayerSkillLevel skillLevel = DetermineSkillFromMetrics(currentMetrics_);
    
    // Update statistics
    stats_.skillLevelHistory[skillLevel]++;
    
    return skillLevel;
}

float DifficultyAnalyzer::AnalyzeCombatPerformance(const Player& player) {
    float score = 0.5f; // Base score
    
    // Analyze damage output
    if (currentMetrics_.averageDamagePerSecond > player._pLevel * 10) {
        score += 0.2f; // Good damage output
    } else if (currentMetrics_.averageDamagePerSecond < player._pLevel * 5) {
        score -= 0.2f; // Low damage output
    }
    
    // Analyze survival
    if (currentMetrics_.deathCount == 0) {
        score += 0.3f; // No deaths
    } else if (currentMetrics_.deathCount > 5) {
        score -= 0.3f; // Many deaths
    } else {
        score -= currentMetrics_.deathCount * 0.05f; // Moderate deaths
    }
    
    // Analyze kill efficiency
    if (currentMetrics_.monstersKilled > player._pLevel * 20) {
        score += 0.1f; // High kill count
    }
    
    return std::clamp(score, 0.0f, 1.0f);
}

float DifficultyAnalyzer::AnalyzeProgressionRate(const Player& player) {
    float score = 0.5f; // Base score
    
    // Analyze quest completion rate
    int totalQuests = currentMetrics_.questsCompleted + currentMetrics_.questsFailed;
    if (totalQuests > 0) {
        float successRate = static_cast<float>(currentMetrics_.questsCompleted) / totalQuests;
        score += (successRate - 0.5f); // Adjust based on success rate
    }
    
    // Analyze level completion time
    if (currentMetrics_.averageLevelCompletionTime > 0) {
        float expectedTime = player._pLevel * 300.0f; // 5 minutes per level as baseline
        if (currentMetrics_.averageLevelCompletionTime < expectedTime * 0.7f) {
            score += 0.2f; // Fast completion
        } else if (currentMetrics_.averageLevelCompletionTime > expectedTime * 1.5f) {
            score -= 0.2f; // Slow completion
        }
    }
    
    return std::clamp(score, 0.0f, 1.0f);
}

float DifficultyAnalyzer::AnalyzeResourceManagement(const Player& player) {
    float score = 0.5f; // Base score
    
    // Analyze mana efficiency
    if (currentMetrics_.manaEfficiency > 0.7f) {
        score += 0.2f; // Good mana management
    } else if (currentMetrics_.manaEfficiency < 0.3f) {
        score -= 0.1f; // Poor mana management
    }
    
    // Analyze potion usage (simplified - would need actual tracking)
    // For now, assume moderate usage is optimal
    
    return std::clamp(score, 0.0f, 1.0f);
}

float DifficultyAnalyzer::AnalyzeSkillUsage(const Player& player) {
    float score = 0.5f; // Base score
    
    // Analyze spell accuracy
    if (currentMetrics_.averageSpellAccuracy > 0.8f) {
        score += 0.3f; // High accuracy
    } else if (currentMetrics_.averageSpellAccuracy < 0.4f) {
        score -= 0.2f; // Low accuracy
    }
    
    // Analyze spell variety (more spells used = better)
    if (currentMetrics_.spellUsageCount.size() > 5) {
        score += 0.1f; // Good variety
    } else if (currentMetrics_.spellUsageCount.size() < 2) {
        score -= 0.1f; // Limited variety
    }
    
    return std::clamp(score, 0.0f, 1.0f);
}

float DifficultyAnalyzer::AnalyzeMovementEfficiency(const Player& player) {
    float score = 0.5f; // Base score
    
    // Analyze movement speed (higher is generally better)
    if (currentMetrics_.averageMovementSpeed > 100.0f) {
        score += 0.1f; // Fast movement
    } else if (currentMetrics_.averageMovementSpeed < 50.0f) {
        score -= 0.1f; // Slow movement
    }
    
    // Analyze backtracking (less is better)
    if (currentMetrics_.backtrackingCount < 5) {
        score += 0.1f; // Efficient pathfinding
    } else if (currentMetrics_.backtrackingCount > 20) {
        score -= 0.1f; // Inefficient pathfinding
    }
    
    return std::clamp(score, 0.0f, 1.0f);
}

PlayerSkillLevel DifficultyAnalyzer::DetermineSkillFromMetrics(const PlayerPerformanceMetrics& metrics) {
    float skillScore = CalculateSkillScore(metrics);
    
    if (skillScore >= 0.9f) return PlayerSkillLevel::MASTER;
    if (skillScore >= 0.8f) return PlayerSkillLevel::EXPERT;
    if (skillScore >= 0.65f) return PlayerSkillLevel::ADVANCED;
    if (skillScore >= 0.5f) return PlayerSkillLevel::INTERMEDIATE;
    if (skillScore >= 0.3f) return PlayerSkillLevel::NOVICE;
    return PlayerSkillLevel::BEGINNER;
}

float DifficultyAnalyzer::CalculateSkillScore(const PlayerPerformanceMetrics& metrics) {
    float score = 0.0f;
    
    // Combat skill (40% weight)
    if (metrics.deathCount == 0 && metrics.monstersKilled > 10) {
        score += 0.4f;
    } else if (metrics.deathCount < 3) {
        score += 0.2f;
    }
    
    // Spell accuracy (30% weight)
    score += metrics.averageSpellAccuracy * 0.3f;
    
    // Resource management (20% weight)
    score += metrics.manaEfficiency * 0.2f;
    
    // Quest success rate (10% weight)
    int totalQuests = metrics.questsCompleted + metrics.questsFailed;
    if (totalQuests > 0) {
        float successRate = static_cast<float>(metrics.questsCompleted) / totalQuests;
        score += successRate * 0.1f;
    }
    
    return std::clamp(score, 0.0f, 1.0f);
}

float DifficultyAnalyzer::CalculateConfidence(const DifficultyAdjustment& adjustment) {
    float confidence = 0.5f; // Base confidence
    
    // More data = higher confidence
    if (currentMetrics_.totalPlayTime > 1800.0f) { // 30 minutes
        confidence += 0.3f;
    } else if (currentMetrics_.totalPlayTime > 600.0f) { // 10 minutes
        confidence += 0.2f;
    }
    
    // More combat data = higher confidence for combat adjustments
    if (currentMetrics_.monstersKilled > 20) {
        confidence += 0.2f;
    }
    
    return std::clamp(confidence, 0.0f, 1.0f);
}

std::vector<std::string> DifficultyAnalyzer::GenerateSpecificRecommendations(
    const DifficultyAdjustment& adjustment, const Player& player) {
    
    std::vector<std::string> recommendations;
    
    switch (adjustment.adjustmentType) {
        case DifficultyAdjustment::Type::INCREASE_MAJOR:
            recommendations.push_back("Increase monster health by 30%");
            recommendations.push_back("Increase monster damage by 20%");
            recommendations.push_back("Add more elite monsters");
            break;
            
        case DifficultyAdjustment::Type::INCREASE_MINOR:
            recommendations.push_back("Increase monster health by 10%");
            recommendations.push_back("Slightly increase monster aggression");
            break;
            
        case DifficultyAdjustment::Type::DECREASE_MAJOR:
            recommendations.push_back("Decrease monster health by 30%");
            recommendations.push_back("Decrease monster damage by 25%");
            recommendations.push_back("Increase healing item drops");
            break;
            
        case DifficultyAdjustment::Type::DECREASE_MINOR:
            recommendations.push_back("Decrease monster health by 10%");
            recommendations.push_back("Provide more healing opportunities");
            break;
            
        default:
            recommendations.push_back("Maintain current difficulty balance");
            break;
    }
    
    return recommendations;
}

void DifficultyAnalyzer::UpdateHistoricalAverage() {
    if (metricsHistory_.empty()) {
        return;
    }
    
    // Calculate averages from historical data
    historicalAverage_ = PlayerPerformanceMetrics{};
    
    for (const auto& metrics : metricsHistory_) {
        historicalAverage_.averageDamagePerSecond += metrics.averageDamagePerSecond;
        historicalAverage_.damageReceived += metrics.damageReceived;
        historicalAverage_.deathCount += metrics.deathCount;
        historicalAverage_.monstersKilled += metrics.monstersKilled;
        historicalAverage_.averageLevelCompletionTime += metrics.averageLevelCompletionTime;
        historicalAverage_.manaEfficiency += metrics.manaEfficiency;
        historicalAverage_.averageSpellAccuracy += metrics.averageSpellAccuracy;
        historicalAverage_.questsCompleted += metrics.questsCompleted;
        historicalAverage_.questsFailed += metrics.questsFailed;
    }
    
    size_t count = metricsHistory_.size();
    historicalAverage_.averageDamagePerSecond /= count;
    historicalAverage_.damageReceived /= count;
    historicalAverage_.deathCount /= count;
    historicalAverage_.monstersKilled /= count;
    historicalAverage_.averageLevelCompletionTime /= count;
    historicalAverage_.manaEfficiency /= count;
    historicalAverage_.averageSpellAccuracy /= count;
    historicalAverage_.questsCompleted /= count;
    historicalAverage_.questsFailed /= count;
}

bool DifficultyAnalyzer::ShouldAdjustDifficulty(const Player& player) {
    DifficultyAdjustment adjustment = AnalyzeDifficulty(player);
    return adjustment.adjustmentType != DifficultyAdjustment::Type::NONE &&
           adjustment.confidenceLevel >= minConfidenceThreshold_;
}

void DifficultyAnalyzer::ResetStats() {
    stats_ = AnalysisStats{};
}

} // namespace devilution::ai