/**
 * @file nightmare_items.cpp
 * @brief Nightmare Edition - Unique Items Implementation
 */

#include "nightmare_items.h"

#include <cstdlib>
#include <vector>

#include "nightmare_config.h"
#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema
NightmareItemSystem g_nightmareItems;

void InitNightmareItems()
{
	LogVerbose("⚔️ Initializing Nightmare Unique Items System");
	
	g_nightmareItems.initialized = true;
	g_nightmareItems.enabled = NIGHTMARE_ITEMS_ENABLED();
	g_nightmareItems.registeredItemCount = 0;
	
	RegisterNightmareItems();
	
	LogVerbose("⚔️ Nightmare Items initialized with {} unique items", g_nightmareItems.registeredItemCount);
}

void RegisterNightmareItems()
{
	LogVerbose("💎 Registering Nightmare Items...");
	
	// Nightmare Blade
	auto &blade = g_nightmareItems.itemDatabase[0];
	blade.id = NightmareItemID::NIGHTMARE_BLADE;
	blade.name = "Nightmare Blade";
	blade.description = "Sword forged in nightmares";
	blade.flavorText = "The blade whispers secrets of the damned";
	blade.minLevel = 25;
	blade.damage = 50;
	blade.strengthBonus = 15;
	blade.lifeBonus = 100;
	blade.propertyCount = 2;
	blade.specialProperties[0] = NightmareItemProperty::LIFE_DRAIN;
	blade.specialProperties[1] = NightmareItemProperty::MANA_DRAIN;
	
	// Veil of Shadows
	auto &veil = g_nightmareItems.itemDatabase[1];
	veil.id = NightmareItemID::VEIL_OF_SHADOWS;
	veil.name = "Veil of Shadows";
	veil.description = "Armor woven from darkness";
	veil.flavorText = "Walk unseen among enemies";
	veil.minLevel = 30;
	veil.armor = 40;
	veil.dexterityBonus = 20;
	veil.propertyCount = 1;
	veil.specialProperties[0] = NightmareItemProperty::PARTIAL_INVISIBILITY;
	
	// Band of Dark Sight
	auto &ring = g_nightmareItems.itemDatabase[2];
	ring.id = NightmareItemID::BAND_OF_DARK_SIGHT;
	ring.name = "Band of Dark Sight";
	ring.description = "Ring that reveals hidden secrets";
	ring.flavorText = "See what others cannot";
	ring.minLevel = 20;
	ring.magicBonus = 25;
	ring.manaBonus = 50;
	ring.propertyCount = 1;
	ring.specialProperties[0] = NightmareItemProperty::REVEAL_SECRETS;
	
	// Crown of Nightmares
	auto &crown = g_nightmareItems.itemDatabase[3];
	crown.id = NightmareItemID::CROWN_OF_NIGHTMARES;
	crown.name = "Crown of Nightmares";
	crown.description = "Crown that terrorizes enemies";
	crown.flavorText = "Fear incarnate";
	crown.minLevel = 35;
	crown.armor = 25;
	crown.vitalityBonus = 30;
	crown.propertyCount = 1;
	crown.specialProperties[0] = NightmareItemProperty::TERRORIZE_ENEMIES;
	
	// Elixir of Tormented Souls
	auto &elixir = g_nightmareItems.itemDatabase[4];
	elixir.id = NightmareItemID::ELIXIR_TORMENTED_SOULS;
	elixir.name = "Elixir of Tormented Souls";
	elixir.description = "Potion of complete restoration";
	elixir.flavorText = "Souls of the damned grant power";
	elixir.minLevel = 15;
	elixir.propertyCount = 1;
	elixir.specialProperties[0] = NightmareItemProperty::FULL_RESTORATION;
	
	// Shadowbane Weapon
	auto &shadowbane = g_nightmareItems.itemDatabase[5];
	shadowbane.id = NightmareItemID::SHADOWBANE_WEAPON;
	shadowbane.name = "Shadowbane";
	shadowbane.description = "Weapon effective against shadows";
	shadowbane.flavorText = "Light pierces darkness";
	shadowbane.minLevel = 28;
	shadowbane.damage = 45;
	shadowbane.strengthBonus = 20;
	shadowbane.propertyCount = 1;
	shadowbane.specialProperties[0] = NightmareItemProperty::SHADOW_IMMUNITY;
	
	// Void Walker Boots
	auto &boots = g_nightmareItems.itemDatabase[6];
	boots.id = NightmareItemID::VOID_WALKER_BOOTS;
	boots.name = "Void Walker Boots";
	boots.description = "Boots that allow void walking";
	boots.flavorText = "Step between dimensions";
	boots.minLevel = 32;
	boots.armor = 15;
	boots.dexterityBonus = 25;
	boots.propertyCount = 1;
	boots.specialProperties[0] = NightmareItemProperty::VOID_WALKING;
	
	// Soul Vessel
	auto &vessel = g_nightmareItems.itemDatabase[7];
	vessel.id = NightmareItemID::SOUL_VESSEL;
	vessel.name = "Soul Vessel";
	vessel.description = "Container for soul fragments";
	vessel.flavorText = "Collect the essence of evil";
	vessel.minLevel = 25;
	vessel.magicBonus = 30;
	vessel.propertyCount = 1;
	vessel.specialProperties[0] = NightmareItemProperty::SOUL_COLLECTION;
	
	// Purification Amulet
	auto &amulet = g_nightmareItems.itemDatabase[8];
	amulet.id = NightmareItemID::PURIFICATION_AMULET;
	amulet.name = "Purification Amulet";
	amulet.description = "Amulet that purifies corruption";
	amulet.flavorText = "Cleanse the tainted lands";
	amulet.minLevel = 30;
	amulet.vitalityBonus = 20;
	amulet.lifeBonus = 75;
	amulet.propertyCount = 1;
	amulet.specialProperties[0] = NightmareItemProperty::AREA_PURIFICATION;
	
	// Defender's Crown
	auto &defenderCrown = g_nightmareItems.itemDatabase[9];
	defenderCrown.id = NightmareItemID::DEFENDERS_CROWN;
	defenderCrown.name = "Defender's Crown";
	defenderCrown.description = "Crown of the last defender";
	defenderCrown.flavorText = "Stand against the darkness";
	defenderCrown.minLevel = 40;
	defenderCrown.armor = 35;
	defenderCrown.vitalityBonus = 25;
	defenderCrown.propertyCount = 1;
	defenderCrown.specialProperties[0] = NightmareItemProperty::ENHANCED_DEFENSE;
	
	// Artifact Ring
	auto &artifactRing = g_nightmareItems.itemDatabase[10];
	artifactRing.id = NightmareItemID::ARTIFACT_RING;
	artifactRing.name = "Artifact Ring";
	artifactRing.description = "Ring that detects artifacts";
	artifactRing.flavorText = "Seek the lost treasures";
	artifactRing.minLevel = 22;
	artifactRing.magicBonus = 15;
	artifactRing.propertyCount = 1;
	artifactRing.specialProperties[0] = NightmareItemProperty::ARTIFACT_DETECTION;
	
	// Voidbane Sword
	auto &voidbane = g_nightmareItems.itemDatabase[11];
	voidbane.id = NightmareItemID::VOIDBANE_SWORD;
	voidbane.name = "Voidbane Sword";
	voidbane.description = "Sword that cuts through void";
	voidbane.flavorText = "Pierce the endless darkness";
	voidbane.minLevel = 45;
	voidbane.damage = 65;
	voidbane.strengthBonus = 30;
	voidbane.dexterityBonus = 15;
	voidbane.propertyCount = 2;
	voidbane.specialProperties[0] = NightmareItemProperty::VOID_WALKING;
	voidbane.specialProperties[1] = NightmareItemProperty::SHADOW_IMMUNITY;
	
	// Shadowproof Armor
	auto &shadowproof = g_nightmareItems.itemDatabase[12];
	shadowproof.id = NightmareItemID::SHADOWPROOF_ARMOR;
	shadowproof.name = "Shadowproof Armor";
	shadowproof.description = "Armor immune to shadow attacks";
	shadowproof.flavorText = "Light eternal protects";
	shadowproof.minLevel = 42;
	shadowproof.armor = 55;
	shadowproof.vitalityBonus = 35;
	shadowproof.propertyCount = 1;
	shadowproof.specialProperties[0] = NightmareItemProperty::SHADOW_IMMUNITY;
	
	// Eternal Victory Ring
	auto &victoryRing = g_nightmareItems.itemDatabase[13];
	victoryRing.id = NightmareItemID::ETERNAL_VICTORY_RING;
	victoryRing.name = "Eternal Victory Ring";
	victoryRing.description = "Ring of ultimate triumph";
	victoryRing.flavorText = "Victory over all evil";
	victoryRing.minLevel = 50;
	victoryRing.strengthBonus = 20;
	victoryRing.dexterityBonus = 20;
	victoryRing.magicBonus = 20;
	victoryRing.vitalityBonus = 20;
	victoryRing.propertyCount = 1;
	victoryRing.specialProperties[0] = NightmareItemProperty::ULTIMATE_POWER;
	
	// Nightmare Master Regalia
	auto &regalia = g_nightmareItems.itemDatabase[14];
	regalia.id = NightmareItemID::NIGHTMARE_MASTER_REGALIA;
	regalia.name = "Nightmare Master Regalia";
	regalia.description = "Ultimate nightmare equipment";
	regalia.flavorText = "Master of all nightmares";
	regalia.minLevel = 60;
	regalia.armor = 75;
	regalia.damage = 80;
	regalia.strengthBonus = 40;
	regalia.dexterityBonus = 40;
	regalia.magicBonus = 40;
	regalia.vitalityBonus = 40;
	regalia.lifeBonus = 200;
	regalia.manaBonus = 200;
	regalia.propertyCount = 4;
	regalia.specialProperties[0] = NightmareItemProperty::NIGHTMARE_MASTERY;
	regalia.specialProperties[1] = NightmareItemProperty::ULTIMATE_POWER;
	regalia.specialProperties[2] = NightmareItemProperty::SHADOW_IMMUNITY;
	regalia.specialProperties[3] = NightmareItemProperty::VOID_WALKING;
	
	g_nightmareItems.registeredItemCount = 15;
	
	LogVerbose("💎 Registered {} nightmare items", g_nightmareItems.registeredItemCount);
}

void UpdateNightmareItems()
{
	if (!g_nightmareItems.initialized) {
		return;
	}
	
	bool shouldBeEnabled = NIGHTMARE_ITEMS_ENABLED();
	if (g_nightmareItems.enabled != shouldBeEnabled) {
		g_nightmareItems.enabled = shouldBeEnabled;
		LogVerbose("⚔️ Nightmare Items: {}", shouldBeEnabled ? "ENABLED" : "DISABLED");
	}
}

bool GenerateNightmareItem(NightmareItemID itemId, int x, int y)
{
	if (!g_nightmareItems.initialized || !g_nightmareItems.enabled) {
		return false;
	}
	
	auto* itemData = GetNightmareItemData(itemId);
	if (!itemData) {
		return false;
	}
	
	LogVerbose("💎 Generated Nightmare Item: {} at ({}, {})", itemData->name, x, y);
	g_nightmareItems.nightmareItemsGenerated++;
	
	return true;
}

bool GenerateRandomNightmareItem(int x, int y, int minLevel)
{
	if (g_nightmareItems.registeredItemCount == 0) {
		return false;
	}
	
	// Seleccionar item aleatorio
	int randomIndex = rand() % g_nightmareItems.registeredItemCount;
	auto &item = g_nightmareItems.itemDatabase[randomIndex];
	
	if (item.minLevel <= minLevel) {
		return GenerateNightmareItem(item.id, x, y);
	}
	
	return false;
}

bool IsNightmareItem(int itemId)
{
	return itemId >= 2000 && itemId <= 2014;
}

NightmareItemData* GetNightmareItemData(NightmareItemID itemId)
{
	for (int i = 0; i < g_nightmareItems.registeredItemCount; i++) {
		if (g_nightmareItems.itemDatabase[i].id == itemId) {
			return &g_nightmareItems.itemDatabase[i];
		}
	}
	return nullptr;
}

void ApplyNightmareItemEffects(NightmareItemID itemId, int playerId)
{
	auto* itemData = GetNightmareItemData(itemId);
	if (!itemData) {
		return;
	}
	
	LogVerbose("✨ Applying nightmare item effects: {}", itemData->name);
	
	// En implementación completa aplicaríamos los efectos reales
}

std::string GetNightmareItemDescription(NightmareItemID itemId)
{
	auto* itemData = GetNightmareItemData(itemId);
	if (!itemData) {
		return "Unknown Nightmare Item";
	}
	
	return itemData->name + " - " + itemData->description;
}

void SetNightmareItemsEnabled(bool enabled)
{
	g_nightmareItems.enabled = enabled;
	LogVerbose("⚔️ Nightmare Items manually set to: {}", enabled ? "ENABLED" : "DISABLED");
}

void SetNightmareItemSpawnRate(float multiplier)
{
	g_nightmareItems.spawnChanceMultiplier = multiplier;
	LogVerbose("📈 Nightmare item spawn rate set to: {:.2f}x", multiplier);
}

void OnNightmareItemGenerated(NightmareItemID itemId)
{
	LogVerbose("🎉 Nightmare item generated: {}", static_cast<int>(itemId));
}

void OnNightmareItemPickedUp(NightmareItemID itemId, int playerId)
{
	g_nightmareItems.nightmareItemsFound++;
	LogVerbose("🎒 Nightmare item picked up by player {}: {}", playerId, static_cast<int>(itemId));
}

void OnNightmareItemUsed(NightmareItemID itemId, int playerId)
{
	LogVerbose("⚡ Nightmare item used by player {}: {}", playerId, static_cast<int>(itemId));
}

void CleanupNightmareItems()
{
	LogVerbose("⚔️ Cleaning up Nightmare Items System");
	g_nightmareItems.initialized = false;
	g_nightmareItems.registeredItemCount = 0;
}

} // namespace devilution