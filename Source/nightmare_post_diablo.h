/**
 * @file nightmare_post_diablo.h
 * @brief Nightmare Edition - Post-Diablo Content System
 * 
 * SISTEMA DE CONTENIDO POST-DIABLO NIGHTMARE
 * =========================================
 * 
 * Sistema que desbloquea contenido épico después de derrotar a Diablo:
 * - Nivel Secreto 17: "The Void" 
 * - NPCs perdidos con quests especiales
 * - Items únicos nightmare
 * - Boss final: "Diablo's Shadow"
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Contenido para verdaderos campeones
 * - Desafío épico post-game
 * - Recompensas únicas e impactantes
 * - Historia expandida del universo Diablo
 */

#pragma once

#include <cstdint>
#include <string>

namespace devilution {

/**
 * @brief Estados del contenido post-Diablo
 */
enum class PostDiabloState {
	LOCKED,                 ///< Bloqueado (Diablo no derrotado)
	PORTAL_AVAILABLE,       ///< Portal disponible en Hell 16
	VOID_ACCESSIBLE,        ///< The Void accesible
	SHADOW_DEFEATED,        ///< Diablo's Shadow derrotado
	NIGHTMARE_COMPLETE      ///< Nightmare Edition completada
};

/**
 * @brief Tipos de NPCs post-Diablo
 */
enum class PostDiabloNPCType {
	LOST_WARRIOR,           ///< Guerrero perdido en The Void
	SHADOW_MERCHANT,        ///< Comerciante de sombras
	TORMENTED_SOUL,         ///< Alma atormentada
	VOID_GUARDIAN,          ///< Guardián del vacío
	NIGHTMARE_ORACLE        ///< Oráculo de pesadillas
};

/**
 * @brief Configuración del sistema post-Diablo
 */
struct PostDiabloConfig {
	bool enabled = true;                        ///< Sistema habilitado
	bool voidLevelEnabled = true;               ///< Nivel The Void habilitado
	bool shadowBossEnabled = true;              ///< Boss Diablo's Shadow habilitado
	bool nightmareItemsEnabled = true;          ///< Items únicos habilitados
	bool postDiabloQuestsEnabled = true;        ///< Quests post-Diablo habilitadas
	int voidLevelNumber = 17;                   ///< Número del nivel The Void
	int portalRequiredLevel = 16;               ///< Nivel donde aparece el portal
};

/**
 * @brief Estado global del sistema post-Diablo
 */
struct PostDiabloSystemState {
	bool initialized = false;
	PostDiabloConfig config;
	PostDiabloState currentState = PostDiabloState::LOCKED;
	
	// Estado del progreso
	bool diabloDefeated = false;
	bool voidPortalCreated = false;
	bool voidLevelVisited = false;
	bool shadowBossDefeated = false;
	
	// Estadísticas
	uint32_t voidVisitCount = 0;
	uint32_t shadowBossAttempts = 0;
	uint32_t nightmareItemsFound = 0;
	
	// NPCs post-Diablo
	bool lostWarriorSpawned = false;
	bool shadowMerchantSpawned = false;
	bool voidGuardianSpawned = false;
};

// Estado global
extern PostDiabloSystemState g_postDiablo;

/**
 * @brief Inicializa el sistema de contenido post-Diablo
 */
void InitPostDiabloSystem();

/**
 * @brief Actualiza el sistema post-Diablo
 */
void UpdatePostDiabloSystem();

/**
 * @brief Verifica si Diablo ha sido derrotado
 * @return true si Diablo fue derrotado
 */
bool IsDiabloDefeated();

/**
 * @brief Notifica que Diablo ha sido derrotado
 */
void OnDiabloDefeated();

/**
 * @brief Crea el portal a The Void en Hell Level 16
 * @return true si el portal fue creado exitosamente
 */
bool CreateVoidPortal();

/**
 * @brief Verifica si el portal a The Void está disponible
 * @return true si el portal está disponible
 */
bool IsVoidPortalAvailable();

/**
 * @brief Genera el nivel The Void (Level 17)
 */
void GenerateVoidLevel();

/**
 * @brief Inicializa NPCs especiales en The Void
 */
void InitVoidNPCs();

/**
 * @brief Genera el boss Diablo's Shadow
 */
void SpawnDiablosShadow();

/**
 * @brief Notifica que Diablo's Shadow ha sido derrotado
 */
void OnDiablosShadowDefeated();

/**
 * @brief Genera items únicos nightmare
 * @param x Posición X
 * @param y Posición Y
 * @param itemType Tipo de item a generar
 */
void GenerateNightmareItem(int x, int y, int itemType);

/**
 * @brief Obtiene el estado actual del sistema post-Diablo
 * @return Estado actual
 */
PostDiabloState GetPostDiabloState();

/**
 * @brief Habilita/deshabilita el sistema post-Diablo
 * @param enabled Estado a establecer
 */
void SetPostDiabloEnabled(bool enabled);

/**
 * @brief Obtiene texto descriptivo del progreso post-Diablo
 * @return Descripción del progreso actual
 */
std::string GetPostDiabloProgressText();

/**
 * @brief Limpia el sistema post-Diablo
 */
void CleanupPostDiabloSystem();

// Macros de conveniencia
#define POST_DIABLO_ENABLED() (g_postDiablo.initialized && g_postDiablo.config.enabled)
#define VOID_LEVEL_AVAILABLE() (g_postDiablo.currentState >= PostDiabloState::VOID_ACCESSIBLE)
#define SHADOW_BOSS_AVAILABLE() (g_postDiablo.config.shadowBossEnabled && VOID_LEVEL_AVAILABLE())

} // namespace devilution