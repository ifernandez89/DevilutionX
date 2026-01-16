/**
 * @file nightmare_ambience.cpp
 * @brief Nightmare Edition - Ambience System Implementation
 * 
 * IMPLEMENTACIÓN DEL SISTEMA DE AMBIENTACIÓN
 * ==========================================
 * 
 * Transforma nombres y textos para crear atmósfera nightmare.
 */

#include "nightmare_ambience.h"

#include "nightmare_config.h"
#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema
NightmareAmbienceState g_nightmareAmbience;

void InitNightmareAmbience()
{
	LogVerbose("🌙 Initializing Nightmare Ambience System");
	
	g_nightmareAmbience.initialized = true;
	g_nightmareAmbience.enabled = NIGHTMARE_AMBIENCE_ENABLED();
	
	// Limpiar mapeos existentes
	g_nightmareAmbience.npcNames.clear();
	g_nightmareAmbience.placeNames.clear();
	g_nightmareAmbience.itemNames.clear();
	
	// Cargar mapeos predefinidos
	LoadDefaultAmbienceMappings();
	
	LogVerbose("🌙 Nightmare Ambience System initialized");
	LogVerbose("👥 NPC mappings: {}", g_nightmareAmbience.npcNames.size());
	LogVerbose("🏰 Place mappings: {}", g_nightmareAmbience.placeNames.size());
	LogVerbose("⚔️ Item mappings: {}", g_nightmareAmbience.itemNames.size());
}

void UpdateNightmareAmbience()
{
	if (!g_nightmareAmbience.initialized) {
		return;
	}
	
	// Verificar si la feature está habilitada
	bool shouldBeEnabled = NIGHTMARE_AMBIENCE_ENABLED();
	if (g_nightmareAmbience.enabled != shouldBeEnabled) {
		g_nightmareAmbience.enabled = shouldBeEnabled;
		LogVerbose("🌙 Nightmare Ambience: {}", shouldBeEnabled ? "ENABLED" : "DISABLED");
	}
}

std::string GetNightmareNPCName(const std::string& originalName)
{
	if (!g_nightmareAmbience.initialized || !g_nightmareAmbience.enabled || !g_nightmareAmbience.renameNPCs) {
		return originalName;
	}
	
	auto it = g_nightmareAmbience.npcNames.find(originalName);
	if (it != g_nightmareAmbience.npcNames.end() && it->second.enabled) {
		return it->second.nightmareName;
	}
	
	return originalName;
}

std::string GetNightmarePlaceName(const std::string& originalName)
{
	if (!g_nightmareAmbience.initialized || !g_nightmareAmbience.enabled || !g_nightmareAmbience.renamePlaces) {
		return originalName;
	}
	
	auto it = g_nightmareAmbience.placeNames.find(originalName);
	if (it != g_nightmareAmbience.placeNames.end() && it->second.enabled) {
		return it->second.nightmareName;
	}
	
	return originalName;
}

std::string GetNightmareItemName(const std::string& originalName)
{
	if (!g_nightmareAmbience.initialized || !g_nightmareAmbience.enabled || !g_nightmareAmbience.renameItems) {
		return originalName;
	}
	
	auto it = g_nightmareAmbience.itemNames.find(originalName);
	if (it != g_nightmareAmbience.itemNames.end() && it->second.enabled) {
		return it->second.nightmareName;
	}
	
	return originalName;
}

void SetNightmareAmbienceEnabled(bool enabled)
{
	g_nightmareAmbience.enabled = enabled;
	LogVerbose("🌙 Nightmare Ambience manually set to: {}", enabled ? "ENABLED" : "DISABLED");
}

void RegisterAmbienceMapping(AmbienceElementType type, 
                           const std::string& original, 
                           const std::string& nightmare,
                           const std::string& description)
{
	AmbienceMapping mapping;
	mapping.originalName = original;
	mapping.nightmareName = nightmare;
	mapping.description = description;
	mapping.enabled = true;
	
	switch (type) {
	case AmbienceElementType::NPC_NAME:
		g_nightmareAmbience.npcNames[original] = mapping;
		LogVerbose("👥 Registered NPC mapping: {} → {}", original, nightmare);
		break;
	case AmbienceElementType::PLACE_NAME:
		g_nightmareAmbience.placeNames[original] = mapping;
		LogVerbose("🏰 Registered Place mapping: {} → {}", original, nightmare);
		break;
	case AmbienceElementType::ITEM_NAME:
		g_nightmareAmbience.itemNames[original] = mapping;
		LogVerbose("⚔️ Registered Item mapping: {} → {}", original, nightmare);
		break;
	default:
		LogVerbose("⚠️ Unknown ambience element type: {}", static_cast<int>(type));
		break;
	}
}

void LoadDefaultAmbienceMappings()
{
	LogVerbose("🌙 Loading default nightmare ambience mappings");
	
	// ============================================================================
	// 👥 MAPEOS DE NPCs - Nombres más tenebrosos y atmosféricos
	// ============================================================================
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Griswold", "Grimforge the Cursed Smith", 
		"Herrero maldito que forja armas en las sombras");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Pepin", "Mortis the Plague Healer", 
		"Sanador que conoce los secretos de la muerte");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Ogden", "Thorne the Innkeeper", 
		"Posadero que guarda secretos oscuros");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Farnham", "Madness the Drunkard", 
		"Borracho enloquecido por visiones del infierno");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Adria", "Shadowmere the Witch", 
		"Bruja que comercia con las fuerzas oscuras");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Cain", "Elderscribe the Ancient", 
		"Anciano escriba que conoce profecías perdidas");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Gillian", "Sorrow the Barmaid", 
		"Sirvienta marcada por la tristeza eterna");
	
	RegisterAmbienceMapping(AmbienceElementType::NPC_NAME, 
		"Wirt", "Shadowtrade the Crippled", 
		"Comerciante tullido que trafica con lo prohibido");
	
	// ============================================================================
	// 🏰 MAPEOS DE LUGARES - Nombres más siniestros
	// ============================================================================
	
	RegisterAmbienceMapping(AmbienceElementType::PLACE_NAME, 
		"Tristram", "Shadowhaven", 
		"Refugio de sombras donde la esperanza muere");
	
	RegisterAmbienceMapping(AmbienceElementType::PLACE_NAME, 
		"Cathedral", "Cursed Sanctum", 
		"Santuario maldito donde el mal anida");
	
	RegisterAmbienceMapping(AmbienceElementType::PLACE_NAME, 
		"Catacombs", "Bone Labyrinth", 
		"Laberinto de huesos donde los muertos no descansan");
	
	RegisterAmbienceMapping(AmbienceElementType::PLACE_NAME, 
		"Caves", "Abyssal Depths", 
		"Profundidades abisales donde la luz no llega");
	
	RegisterAmbienceMapping(AmbienceElementType::PLACE_NAME, 
		"Hell", "The Eternal Torment", 
		"Tormento eterno donde las almas sufren");
	
	// ============================================================================
	// ⚔️ MAPEOS DE ITEMS - Nombres temáticos nightmare
	// ============================================================================
	
	RegisterAmbienceMapping(AmbienceElementType::ITEM_NAME, 
		"Short Sword", "Nightmare Blade", 
		"Hoja forjada en pesadillas");
	
	RegisterAmbienceMapping(AmbienceElementType::ITEM_NAME, 
		"Leather Armor", "Veil of Shadows", 
		"Velo que oculta al portador en sombras");
	
	RegisterAmbienceMapping(AmbienceElementType::ITEM_NAME, 
		"Ring of Truth", "Band of Dark Sight", 
		"Anillo que revela verdades ocultas");
	
	RegisterAmbienceMapping(AmbienceElementType::ITEM_NAME, 
		"Healing Potion", "Elixir of Tormented Souls", 
		"Elixir destilado de almas atormentadas");
	
	RegisterAmbienceMapping(AmbienceElementType::ITEM_NAME, 
		"Mana Potion", "Essence of Nightmare", 
		"Esencia pura extraída de pesadillas");
	
	LogVerbose("🌙 Loaded {} NPC mappings", g_nightmareAmbience.npcNames.size());
	LogVerbose("🌙 Loaded {} Place mappings", g_nightmareAmbience.placeNames.size());
	LogVerbose("🌙 Loaded {} Item mappings", g_nightmareAmbience.itemNames.size());
}

void CleanupNightmareAmbience()
{
	LogVerbose("🌙 Cleaning up Nightmare Ambience System");
	
	g_nightmareAmbience.initialized = false;
	g_nightmareAmbience.enabled = false;
	g_nightmareAmbience.npcNames.clear();
	g_nightmareAmbience.placeNames.clear();
	g_nightmareAmbience.itemNames.clear();
}

} // namespace devilution