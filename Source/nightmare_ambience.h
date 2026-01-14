/**
 * @file nightmare_ambience.h
 * @brief Nightmare Edition - Ambience System
 * 
 * SISTEMA DE AMBIENTACIÓN NIGHTMARE
 * ================================
 * 
 * Sistema que transforma nombres, diálogos y lugares para crear
 * una atmósfera más tenebrosa y nightmare.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Nombres más oscuros y atmosféricos
 * - Coherencia temática en todo el juego
 * - Inmersión narrativa profunda
 * - Respeto por la esencia original
 */

#pragma once

#include <string>
#include <unordered_map>

namespace devilution {

/**
 * @brief Tipos de elementos que pueden ser renombrados
 */
enum class AmbienceElementType {
	NPC_NAME,           ///< Nombres de NPCs
	PLACE_NAME,         ///< Nombres de lugares
	ITEM_NAME,          ///< Nombres de items
	QUEST_TEXT,         ///< Textos de quests
	DIALOG_TEXT         ///< Diálogos de NPCs
};

/**
 * @brief Mapeo de nombres originales a nombres nightmare
 */
struct AmbienceMapping {
	std::string originalName;
	std::string nightmareName;
	std::string description;
	bool enabled = true;
};

/**
 * @brief Estado del sistema de ambientación
 */
struct NightmareAmbienceState {
	bool initialized = false;
	bool enabled = false;
	
	// Mapeos por categoría
	std::unordered_map<std::string, AmbienceMapping> npcNames;
	std::unordered_map<std::string, AmbienceMapping> placeNames;
	std::unordered_map<std::string, AmbienceMapping> itemNames;
	
	// Configuración
	bool renameNPCs = true;
	bool renamePlaces = true;
	bool renameItems = true;
	bool darkDialogs = true;
};

// Estado global
extern NightmareAmbienceState g_nightmareAmbience;

/**
 * @brief Inicializa el sistema de ambientación nightmare
 */
void InitNightmareAmbience();

/**
 * @brief Actualiza el sistema de ambientación
 */
void UpdateNightmareAmbience();

/**
 * @brief Obtiene el nombre nightmare de un NPC
 * @param originalName Nombre original del NPC
 * @return Nombre nightmare o nombre original si no hay mapeo
 */
std::string GetNightmareNPCName(const std::string& originalName);

/**
 * @brief Obtiene el nombre nightmare de un lugar
 * @param originalName Nombre original del lugar
 * @return Nombre nightmare o nombre original si no hay mapeo
 */
std::string GetNightmarePlaceName(const std::string& originalName);

/**
 * @brief Obtiene el nombre nightmare de un item
 * @param originalName Nombre original del item
 * @return Nombre nightmare o nombre original si no hay mapeo
 */
std::string GetNightmareItemName(const std::string& originalName);

/**
 * @brief Habilita/deshabilita el sistema de ambientación
 * @param enabled Estado a establecer
 */
void SetNightmareAmbienceEnabled(bool enabled);

/**
 * @brief Registra un mapeo de nombre
 * @param type Tipo de elemento
 * @param original Nombre original
 * @param nightmare Nombre nightmare
 * @param description Descripción del cambio
 */
void RegisterAmbienceMapping(AmbienceElementType type, 
                           const std::string& original, 
                           const std::string& nightmare,
                           const std::string& description = "");

/**
 * @brief Carga mapeos predefinidos de ambientación
 */
void LoadDefaultAmbienceMappings();

/**
 * @brief Limpia el sistema de ambientación
 */
void CleanupNightmareAmbience();

// Macros de conveniencia
#define NIGHTMARE_NPC_NAME(original) GetNightmareNPCName(original)
#define NIGHTMARE_PLACE_NAME(original) GetNightmarePlaceName(original)
#define NIGHTMARE_ITEM_NAME(original) GetNightmareItemName(original)

} // namespace devilution