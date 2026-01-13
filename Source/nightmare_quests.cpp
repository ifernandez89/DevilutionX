/**
 * @file nightmare_quests.cpp
 * @brief Nightmare Edition - Post-Diablo Quests Implementation
 * 
 * IMPLEMENTACIÓN DE QUESTS POST-DIABLO
 * ====================================
 * 
 * Quests épicas que expanden la experiencia post-game.
 */

#include "nightmare_quests.h"

#include "nightmare_config.h"
#include "nightmare_post_diablo.h"
#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema
NightmareQuestSystem g_nightmareQuests;

void InitNightmareQuests()
{
	LogVerbose("🎯 Initializing Nightmare Quests System");
	
	g_nightmareQuests.initialized = true;
	g_nightmareQuests.enabled = NIGHTMARE_QUESTS_ENABLED();
	g_nightmareQuests.activeQuestCount = 0;
	g_nightmareQuests.completedQuestCount = 0;
	
	// ============================================================================
	// 🎯 CONFIGURACIÓN DE QUESTS NIGHTMARE ÉPICAS
	// ============================================================================
	
	// Quest 1: Cleanse the Nightmare
	auto &cleanseQuest = g_nightmareQuests.quests[0];
	cleanseQuest.id = NightmareQuestID::CLEANSE_NIGHTMARE;
	cleanseQuest.name = "Cleanse the Nightmare";
	cleanseQuest.description = "The corruption of Diablo lingers in dark corners. Purify 5 corrupted areas to cleanse the nightmare.";
	cleanseQuest.objective = "Purify corrupted areas (0/5)";
	cleanseQuest.requiredProgress = 5;
	cleanseQuest.experienceReward = 50000;
	cleanseQuest.goldReward = 10000;
	cleanseQuest.itemReward = "Purification Amulet";
	cleanseQuest.levelRequirement = 20;
	
	// Quest 2: Collect Soul Fragments
	auto &soulQuest = g_nightmareQuests.quests[1];
	soulQuest.id = NightmareQuestID::SOUL_FRAGMENTS;
	soulQuest.name = "Collect Soul Fragments";
	soulQuest.description = "Diablo's shattered soul has scattered across the realms. Collect 10 Soul Fragments to prevent his return.";
	soulQuest.objective = "Collect Soul Fragments (0/10)";
	soulQuest.requiredProgress = 10;
	soulQuest.experienceReward = 75000;
	soulQuest.goldReward = 15000;
	soulQuest.itemReward = "Soul Vessel";
	soulQuest.levelRequirement = 25;
	
	// Quest 3: The Last Stand
	auto &lastStandQuest = g_nightmareQuests.quests[2];
	lastStandQuest.id = NightmareQuestID::LAST_STAND;
	lastStandQuest.name = "The Last Stand";
	lastStandQuest.description = "Demonic forces assault Tristram in waves. Defend the town and eliminate 50 shadow demons.";
	lastStandQuest.objective = "Eliminate Shadow Demons (0/50)";
	lastStandQuest.requiredProgress = 50;
	lastStandQuest.experienceReward = 100000;
	lastStandQuest.goldReward = 20000;
	lastStandQuest.itemReward = "Defender's Crown";
	lastStandQuest.levelRequirement = 30;
	
	// Quest 4: Nightmare Artifacts
	auto &artifactsQuest = g_nightmareQuests.quests[3];
	artifactsQuest.id = NightmareQuestID::NIGHTMARE_ARTIFACTS;
	artifactsQuest.name = "Nightmare Artifacts";
	artifactsQuest.description = "Ancient artifacts of power lie hidden in the deepest dungeons. Find 7 Nightmare Artifacts.";
	artifactsQuest.objective = "Find Nightmare Artifacts (0/7)";
	artifactsQuest.requiredProgress = 7;
	artifactsQuest.experienceReward = 125000;
	artifactsQuest.goldReward = 25000;
	artifactsQuest.itemReward = "Artifact Collector's Ring";
	artifactsQuest.levelRequirement = 35;
	
	// Quest 5: The True Evil (Boss Final)
	auto &trueEvilQuest = g_nightmareQuests.quests[4];
	trueEvilQuest.id = NightmareQuestID::TRUE_EVIL;
	trueEvilQuest.name = "The True Evil";
	trueEvilQuest.description = "Face Diablo's Shadow in The Void - the ultimate manifestation of evil. Only the bravest dare attempt this.";
	trueEvilQuest.objective = "Defeat Diablo's Shadow (0/1)";
	trueEvilQuest.requiredProgress = 1;
	trueEvilQuest.experienceReward = 200000;
	trueEvilQuest.goldReward = 50000;
	trueEvilQuest.itemReward = "Crown of Ultimate Victory";
	trueEvilQuest.levelRequirement = 40;
	trueEvilQuest.requiresVoidAccess = true;
	
	// Quest 6: Void Exploration
	auto &voidQuest = g_nightmareQuests.quests[5];
	voidQuest.id = NightmareQuestID::VOID_EXPLORATION;
	voidQuest.name = "Void Exploration";
	voidQuest.description = "Map the entirety of The Void dimension. Explore all hidden areas and uncover its secrets.";
	voidQuest.objective = "Explore The Void (0/100%)";
	voidQuest.requiredProgress = 100;
	voidQuest.experienceReward = 80000;
	voidQuest.goldReward = 18000;
	voidQuest.itemReward = "Void Walker Boots";
	voidQuest.levelRequirement = 30;
	voidQuest.requiresVoidAccess = true;
	
	// Quest 7: Shadow Minions
	auto &minionsQuest = g_nightmareQuests.quests[6];
	minionsQuest.id = NightmareQuestID::SHADOW_MINIONS;
	minionsQuest.name = "Shadow Minions";
	minionsQuest.description = "Diablo's Shadow commands legions of nightmare creatures. Eliminate 100 shadow minions in The Void.";
	minionsQuest.objective = "Eliminate Shadow Minions (0/100)";
	minionsQuest.requiredProgress = 100;
	minionsQuest.experienceReward = 90000;
	minionsQuest.goldReward = 20000;
	minionsQuest.itemReward = "Shadowbane Weapon";
	minionsQuest.levelRequirement = 35;
	minionsQuest.requiresVoidAccess = true;
	minionsQuest.isRepeatable = true;
	
	// Quest 8: Nightmare Mastery (Meta-quest)
	auto &masteryQuest = g_nightmareQuests.quests[7];
	masteryQuest.id = NightmareQuestID::NIGHTMARE_MASTERY;
	masteryQuest.name = "Nightmare Mastery";
	masteryQuest.description = "Complete all Nightmare Edition quests to achieve ultimate mastery over the forces of darkness.";
	masteryQuest.objective = "Complete all Nightmare Quests (0/7)";
	masteryQuest.requiredProgress = 7;
	masteryQuest.experienceReward = 500000;
	masteryQuest.goldReward = 100000;
	masteryQuest.itemReward = "Nightmare Master's Regalia";
	masteryQuest.levelRequirement = 45;
	
	LogVerbose("🎯 Nightmare Quests System initialized");
	LogVerbose("📋 Total quests available: {}", g_nightmareQuests.quests.size());
	LogVerbose("🏆 Epic rewards and challenges await!");
}

void UpdateNightmareQuests()
{
	if (!g_nightmareQuests.initialized) {
		return;
	}
	
	// Verificar si la feature está habilitada
	bool shouldBeEnabled = NIGHTMARE_QUESTS_ENABLED();
	if (g_nightmareQuests.enabled != shouldBeEnabled) {
		g_nightmareQuests.enabled = shouldBeEnabled;
		LogVerbose("🎯 Nightmare Quests: {}", shouldBeEnabled ? "ENABLED" : "DISABLED");
	}
	
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	// Desbloquear quests automáticamente si Diablo fue derrotado
	if (IsDiabloDefeated()) {
		UnlockPostDiabloQuests();
	}
	
	// Verificar progreso de quest de maestría
	UpdateMasteryQuestProgress();
}

void UnlockPostDiabloQuests()
{
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	LogVerbose("🎯 Unlocking Post-Diablo Nightmare Quests...");
	
	int unlockedCount = 0;
	
	for (auto &quest : g_nightmareQuests.quests) {
		// Desbloquear quests que no requieren acceso a The Void
		if (quest.state == NightmareQuestState::LOCKED && !quest.requiresVoidAccess) {
			quest.state = NightmareQuestState::AVAILABLE;
			unlockedCount++;
			LogVerbose("🔓 Quest unlocked: {}", quest.name);
		}
		
		// Desbloquear quests de The Void si está accesible
		if (quest.requiresVoidAccess && VOID_LEVEL_AVAILABLE() && 
		    quest.state == NightmareQuestState::LOCKED) {
			quest.state = NightmareQuestState::AVAILABLE;
			unlockedCount++;
			LogVerbose("🌌 Void Quest unlocked: {}", quest.name);
		}
	}
	
	if (unlockedCount > 0) {
		LogVerbose("🎯 {} Nightmare Quests now available!", unlockedCount);
		LogVerbose("📋 Check your quest log for epic challenges and rewards");
	}
}

bool StartNightmareQuest(NightmareQuestID questId)
{
	auto* quest = GetNightmareQuest(questId);
	if (!quest || quest->state != NightmareQuestState::AVAILABLE) {
		return false;
	}
	
	quest->state = NightmareQuestState::ACTIVE;
	quest->currentProgress = 0;
	g_nightmareQuests.activeQuestCount++;
	
	LogVerbose("🎯 Started Nightmare Quest: {}", quest->name);
	LogVerbose("📋 Objective: {}", quest->objective);
	LogVerbose("🏆 Reward: {} XP, {} Gold, {}", 
		quest->experienceReward, quest->goldReward, quest->itemReward);
	
	return true;
}

void UpdateQuestProgress(NightmareQuestID questId, int progress)
{
	auto* quest = GetNightmareQuest(questId);
	if (!quest || quest->state != NightmareQuestState::ACTIVE) {
		return;
	}
	
	quest->currentProgress += progress;
	
	LogVerbose("🎯 Quest Progress: {} ({}/{})", 
		quest->name, quest->currentProgress, quest->requiredProgress);
	
	// Verificar si se completó
	if (quest->currentProgress >= quest->requiredProgress) {
		CompleteNightmareQuest(questId);
	}
}

void CompleteNightmareQuest(NightmareQuestID questId)
{
	auto* quest = GetNightmareQuest(questId);
	if (!quest) {
		return;
	}
	
	quest->state = NightmareQuestState::COMPLETED;
	quest->currentProgress = quest->requiredProgress;
	g_nightmareQuests.completedQuestCount++;
	
	LogVerbose("🏆 QUEST COMPLETED: {}", quest->name);
	LogVerbose("✨ Rewards granted:");
	LogVerbose("   📈 Experience: {}", quest->experienceReward);
	LogVerbose("   💰 Gold: {}", quest->goldReward);
	LogVerbose("   🎁 Item: {}", quest->itemReward);
	
	// En implementación completa:
	// - Otorgar experiencia al jugador
	// - Agregar oro al inventario
	// - Crear item de recompensa
	// - Mostrar mensaje de completación
	// - Reproducir sonido de victoria
	
	// Verificar si se desbloquearon nuevas quests
	UnlockPostDiabloQuests();
}

void UpdateMasteryQuestProgress()
{
	auto* masteryQuest = GetNightmareQuest(NightmareQuestID::NIGHTMARE_MASTERY);
	if (!masteryQuest || masteryQuest->state != NightmareQuestState::ACTIVE) {
		return;
	}
	
	// Contar quests completadas (excluyendo la de maestría)
	int completedCount = 0;
	for (int i = 0; i < 7; i++) { // Primeras 7 quests
		if (g_nightmareQuests.quests[i].state == NightmareQuestState::COMPLETED) {
			completedCount++;
		}
	}
	
	if (completedCount != masteryQuest->currentProgress) {
		masteryQuest->currentProgress = completedCount;
		LogVerbose("🏆 Nightmare Mastery Progress: {}/7 quests completed", completedCount);
		
		if (completedCount >= 7) {
			CompleteNightmareQuest(NightmareQuestID::NIGHTMARE_MASTERY);
			LogVerbose("👑 NIGHTMARE MASTERY ACHIEVED! You are the ultimate champion!");
		}
	}
}

NightmareQuestState GetQuestState(NightmareQuestID questId)
{
	auto* quest = GetNightmareQuest(questId);
	return quest ? quest->state : NightmareQuestState::LOCKED;
}

NightmareQuest* GetNightmareQuest(NightmareQuestID questId)
{
	for (auto &quest : g_nightmareQuests.quests) {
		if (quest.id == questId) {
			return &quest;
		}
	}
	return nullptr;
}

bool AreAllNightmareQuestsCompleted()
{
	return g_nightmareQuests.completedQuestCount >= 8;
}

std::string GetNightmareQuestProgressSummary()
{
	if (!g_nightmareQuests.enabled) {
		return "Nightmare Quests disabled";
	}
	
	return "Nightmare Quests: " + std::to_string(g_nightmareQuests.completedQuestCount) + 
	       "/8 completed, " + std::to_string(g_nightmareQuests.activeQuestCount) + " active";
}

void OnMonsterKilled(int monsterType)
{
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	// Actualizar quest de Shadow Demons (The Last Stand)
	if (monsterType == 666) { // Shadow Demon placeholder ID
		UpdateQuestProgress(NightmareQuestID::LAST_STAND);
	}
	
	// Actualizar quest de Shadow Minions
	if (monsterType >= 700 && monsterType <= 750) { // Shadow Minions range
		UpdateQuestProgress(NightmareQuestID::SHADOW_MINIONS);
		g_nightmareQuests.shadowMinionsKilled++;
	}
}

void OnItemFound(int itemType)
{
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	// Verificar si es un Nightmare Artifact
	if (itemType >= 1000 && itemType <= 1010) { // Nightmare Artifacts range
		OnNightmareArtifactFound();
	}
}

void OnAreaExplored(int levelType)
{
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	// Actualizar quest de purificación
	if (levelType >= 1 && levelType <= 4) { // Dungeon levels
		UpdateQuestProgress(NightmareQuestID::CLEANSE_NIGHTMARE);
		g_nightmareQuests.areasCleanseD++;
	}
	
	// Actualizar exploración de The Void
	if (levelType == 17) { // The Void
		g_nightmareQuests.voidExplorationProgress += 10;
		UpdateQuestProgress(NightmareQuestID::VOID_EXPLORATION, 10);
	}
}

void OnSoulFragmentCollected()
{
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	g_nightmareQuests.totalSoulFragments++;
	UpdateQuestProgress(NightmareQuestID::SOUL_FRAGMENTS);
	
	LogVerbose("💎 Soul Fragment collected! Total: {}", g_nightmareQuests.totalSoulFragments);
}

void OnNightmareArtifactFound()
{
	if (!g_nightmareQuests.enabled) {
		return;
	}
	
	g_nightmareQuests.artifactsFound++;
	UpdateQuestProgress(NightmareQuestID::NIGHTMARE_ARTIFACTS);
	
	LogVerbose("🏺 Nightmare Artifact found! Total: {}", g_nightmareQuests.artifactsFound);
}

void CleanupNightmareQuests()
{
	LogVerbose("🎯 Cleaning up Nightmare Quests System");
	
	g_nightmareQuests.initialized = false;
	g_nightmareQuests.activeQuestCount = 0;
	g_nightmareQuests.completedQuestCount = 0;
	
	for (auto &quest : g_nightmareQuests.quests) {
		quest.state = NightmareQuestState::LOCKED;
		quest.currentProgress = 0;
	}
}

} // namespace devilution