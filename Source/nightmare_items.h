/**
 * @file nightmare_items.h
 * @brief Nightmare Edition - Unique Items System
 * 
 * SISTEMA DE ITEMS ÚNICOS NIGHTMARE
 * ================================
 * 
 * Items legendarios con poderes únicos que solo aparecen en Nightmare Edition:
 * - Nightmare Blade - Espada que drena vida y mana
 * - Veil of Shadows - Armadura de invisibilidad parcial
 * - Band of Dark Sight - Anillo que revela secretos ocultos
 * - Crown of Nightmares - Casco que aterroriza enemigos
 * - Elixir of Tormented Souls - Poción que restaura todo
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Items verdaderamente únicos y poderosos
 * - Efectos visuales impactantes
 * - Mecánicas de juego innovadoras
 * - Recompensas dignas de leyenda
 */

#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace devilution {

/**
 * @brief IDs de items únicos nightmare
 */
enum class NightmareItemID {
	NIGHTMARE_BLADE = 2000,         ///< Espada legendaria
	VEIL_OF_SHADOWS = 2001,         ///< Armadura de sombras
	BAND_OF_DARK_SIGHT = 2002,      ///< Anillo de visión oscura
	CROWN_OF_NIGHTMARES = 2003,     ///< Corona del terror
	ELIXIR_TORMENTED_SOULS = 2004,  ///< Elixir de almas
	SHADOWBANE_WEAPON = 2005,       ///< Arma anti-sombra
	VOID_WALKER_BOOTS = 2006,       ///< Botas del vacío
	SOUL_VESSEL = 2007,             ///< Recipiente de almas
	PURIFICATION_AMULET = 2008,     ///< Amuleto purificador
	DEFENDERS_CROWN = 2009,         ///< Corona del defensor
	ARTIFACT_RING = 2010,           ///< Anillo de artefactos
	VOIDBANE_SWORD = 2011,          ///< Espada anti-vacío
	SHADOWPROOF_ARMOR = 2012,       ///< Armadura anti-sombra
	ETERNAL_VICTORY_RING = 2013,    ///< Anillo de victoria eterna
	NIGHTMARE_MASTER_REGALIA = 2014 ///< Atuendo del maestro
};

/**
 * @brief Propiedades especiales de items nightmare
 */
enum class NightmareItemProperty {
	LIFE_DRAIN,                     ///< Drena vida de enemigos
	MANA_DRAIN,                     ///< Drena mana de enemigos
	PARTIAL_INVISIBILITY,           ///< Invisibilidad parcial
	REVEAL_SECRETS,                 ///< Revela puertas/tesoros ocultos
	TERRORIZE_ENEMIES,              ///< Aterroriza enemigos cercanos
	FULL_RESTORATION,               ///< Restaura vida y mana completamente
	SHADOW_IMMUNITY,                ///< Inmunidad a ataques de sombra
	VOID_WALKING,                   ///< Caminar por el vacío
	SOUL_COLLECTION,                ///< Recolecta fragmentos de alma
	AREA_PURIFICATION,              ///< Purifica áreas corrompidas
	ENHANCED_DEFENSE,               ///< Defensa mejorada contra jefes
	ARTIFACT_DETECTION,             ///< Detecta artefactos nightmare
	ULTIMATE_POWER,                 ///< Poder definitivo
	NIGHTMARE_MASTERY               ///< Maestría nightmare completa
};

/**
 * @brief Datos de un item único nightmare
 */
struct NightmareItemData {
	NightmareItemID id;
	std::string name;
	std::string description;
	std::string flavorText;
	
	// Propiedades base
	int itemType = 0;                       ///< Tipo de item base
	int minLevel = 1;                       ///< Nivel mínimo requerido
	int rarity = 100;                       ///< Rareza (1-100, 100 = único)
	
	// Stats base
	int damage = 0;                         ///< Daño (armas)
	int armor = 0;                          ///< Armadura (defensivos)
	int durability = 255;                   ///< Durabilidad máxima
	
	// Bonificaciones
	int strengthBonus = 0;
	int dexterityBonus = 0;
	int magicBonus = 0;
	int vitalityBonus = 0;
	int lifeBonus = 0;
	int manaBonus = 0;
	
	// Propiedades especiales
	std::array<NightmareItemProperty, 4> specialProperties;
	int propertyCount = 0;
	
	// Efectos visuales
	bool hasGlow = true;                    ///< Tiene resplandor especial
	uint8_t glowColor = 255;                ///< Color del resplandor
	bool hasParticles = true;               ///< Tiene efectos de partículas
};

/**
 * @brief Estado del sistema de items nightmare
 */
struct NightmareItemSystem {
	bool initialized = false;
	bool enabled = true;
	
	std::array<NightmareItemData, 15> itemDatabase;
	int registeredItemCount = 0;
	
	// Estadísticas
	int nightmareItemsGenerated = 0;
	int nightmareItemsFound = 0;
	int uniqueItemsCollected = 0;
	
	// Configuración de spawn
	float spawnChanceMultiplier = 1.0f;     ///< Multiplicador de probabilidad
	bool guaranteedDrops = false;           ///< Drops garantizados (testing)
};

// Estado global
extern NightmareItemSystem g_nightmareItems;

/**
 * @brief Inicializa el sistema de items nightmare
 */
void InitNightmareItems();

/**
 * @brief Actualiza el sistema de items nightmare
 */
void UpdateNightmareItems();

/**
 * @brief Registra todos los items únicos nightmare
 */
void RegisterNightmareItems();

/**
 * @brief Genera un item nightmare específico
 * @param itemId ID del item a generar
 * @param x Posición X
 * @param y Posición Y
 * @return true si se generó exitosamente
 */
bool GenerateNightmareItem(NightmareItemID itemId, int x, int y);

/**
 * @brief Genera un item nightmare aleatorio
 * @param x Posición X
 * @param y Posición Y
 * @param minLevel Nivel mínimo del item
 * @return true si se generó exitosamente
 */
bool GenerateRandomNightmareItem(int x, int y, int minLevel = 1);

/**
 * @brief Verifica si un item es un item nightmare
 * @param itemId ID del item
 * @return true si es un item nightmare
 */
bool IsNightmareItem(int itemId);

/**
 * @brief Obtiene datos de un item nightmare
 * @param itemId ID del item
 * @return Puntero a los datos o nullptr si no existe
 */
NightmareItemData* GetNightmareItemData(NightmareItemID itemId);

/**
 * @brief Aplica efectos especiales de un item nightmare
 * @param itemId ID del item
 * @param playerId ID del jugador que lo usa
 */
void ApplyNightmareItemEffects(NightmareItemID itemId, int playerId);

/**
 * @brief Obtiene descripción completa de un item nightmare
 * @param itemId ID del item
 * @return Descripción formateada
 */
std::string GetNightmareItemDescription(NightmareItemID itemId);

/**
 * @brief Habilita/deshabilita el sistema de items nightmare
 * @param enabled Estado a establecer
 */
void SetNightmareItemsEnabled(bool enabled);

/**
 * @brief Establece multiplicador de probabilidad de spawn
 * @param multiplier Multiplicador (1.0 = normal, 2.0 = doble probabilidad)
 */
void SetNightmareItemSpawnRate(float multiplier);

/**
 * @brief Limpia el sistema de items nightmare
 */
void CleanupNightmareItems();

// Funciones de eventos
void OnNightmareItemGenerated(NightmareItemID itemId);
void OnNightmareItemPickedUp(NightmareItemID itemId, int playerId);
void OnNightmareItemUsed(NightmareItemID itemId, int playerId);

} // namespace devilution