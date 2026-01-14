/**
 * @file nightmare_quests.h
 * @brief Nightmare Edition - Post-Diablo Quests System
 * 
 * SISTEMA DE QUESTS POST-DIABLO NIGHTMARE
 * ======================================
 * 
 * Quests épicas que se desbloquean después de derrotar a Diablo:
 * - "Cleanse the Nightmare" - Purificar áreas corrompidas
 * - "Collect Soul Fragments" - Reunir fragmentos del alma de Diablo
 * - "The Last Stand" - Defender Tristram de oleadas demoníacas
 * - "Nightmare Artifacts" - Buscar artefactos legendarios
 * - "The True Evil" - Enfrentar a Diablo's Shadow
 */

#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace devilution {

/**
 * @brief IDs de quests nightmare
 */
enum class NightmareQuestID {
	CLEANSE_NIGHTMARE = 100,        ///< Purificar áreas corrompidas
	SOUL_FRAGMENTS = 101,           ///< Recolectar fragmentos de alma
	LAST_STAND = 102,               ///< Defender Tristram
	NIGHTMARE_ARTIFACTS = 103,      ///< Buscar artefactos
	TRUE_EVIL = 104,                ///< Enfrentar Diablo's Shadow
	VOID_EXPLORATION = 105,         ///< Explorar The Void completamente
	SHADOW_MINIONS = 106,           ///< Eliminar secuaces de sombra
	NIGHTMARE_MASTERY = 107         ///< Dominar todas las quests nightmare
};

/**
 * @brief Estados de quest nightmare
 */
enum class NightmareQuestState {
	LOCKED,                         ///< Bloqueada
	AVAILABLE,                      ///< Disponible para tomar
	ACTIVE,                         ///< Activa (en progreso)
	COMPLETED,                      ///< Completada
	FAILED                          ///< Fallida
};

/**
 * @brief Datos de una quest nightmare
 */
struct NightmareQuest {
	NightmareQuestID id;
	std::string name;
	std::string description;
	std::string objective;
	NightmareQuestState state = NightmareQuestState::LOCKED;
	
	// Progreso
	int currentProgress = 0;
	int requiredProgress = 1;
	
	// Recompensas
	int experienceReward = 0;
	int goldReward = 0;
	std::string itemReward;
	
	// Configuración
	bool isRepeatable = false;
	int levelRequirement = 1;
	bool requiresVoidAccess = false;
};

/**
 * @brief Estado del sistema de quests nightmare
 */
struct NightmareQuestSystem {
	bool initialized = false;
	bool enabled = true;
	
	std::array<NightmareQuest, 8> quests;
	int activeQuestCount = 0;
	int completedQuestCount = 0;
	
	// Estadísticas globales
	int totalSoulFragments = 0;
	int areasCleanseD = 0;
	int artifactsFound = 0;
	int shadowMinionsKilled = 0;
	int voidExplorationProgress = 0;
};

// Estado global
extern NightmareQuestSystem g_nightmareQuests;

/**
 * @brief Inicializa el sistema de quests nightmare
 */
void InitNightmareQuests();

/**
 * @brief Actualiza el sistema de quests nightmare
 */
void UpdateNightmareQuests();

/**
 * @brief Desbloquea quests post-Diablo
 */
void UnlockPostDiabloQuests();

/**
 * @brief Inicia una quest nightmare
 * @param questId ID de la quest
 * @return true si se inició exitosamente
 */
bool StartNightmareQuest(NightmareQuestID questId);

/**
 * @brief Actualiza el progreso de una quest
 * @param questId ID de la quest
 * @param progress Progreso a agregar
 */
void UpdateQuestProgress(NightmareQuestID questId, int progress = 1);

/**
 * @brief Completa una quest nightmare
 * @param questId ID de la quest
 */
void CompleteNightmareQuest(NightmareQuestID questId);

/**
 * @brief Obtiene el estado de una quest
 * @param questId ID de la quest
 * @return Estado actual de la quest
 */
NightmareQuestState GetQuestState(NightmareQuestID questId);

/**
 * @brief Obtiene información de una quest
 * @param questId ID de la quest
 * @return Puntero a la quest o nullptr si no existe
 */
NightmareQuest* GetNightmareQuest(NightmareQuestID questId);

/**
 * @brief Verifica si todas las quests nightmare están completadas
 * @return true si todas están completadas
 */
bool AreAllNightmareQuestsCompleted();

/**
 * @brief Obtiene texto de progreso de todas las quests
 * @return Descripción del progreso
 */
std::string GetNightmareQuestProgressSummary();

/**
 * @brief Limpia el sistema de quests nightmare
 */
void UpdateMasteryQuestProgress();

void CleanupNightmareQuests();

// Funciones de eventos específicos
void OnMonsterKilled(int monsterType);
void OnItemFound(int itemType);
void OnAreaExplored(int levelType);
void OnSoulFragmentCollected();
void OnNightmareArtifactFound();

} // namespace devilution