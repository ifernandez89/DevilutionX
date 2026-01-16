/**
 * @file nightmare_post_diablo.cpp
 * @brief Nightmare Edition - Post-Diablo Content Implementation
 * 
 * IMPLEMENTACIÓN DEL CONTENIDO POST-DIABLO
 * ========================================
 * 
 * Sistema épico que desbloquea The Void y contenido nightmare.
 */

#include "nightmare_post_diablo.h"

#include <cstdlib>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "nightmare_config.h"
#include "utils/log.hpp"
#include "quests.h"
#include "levels/gendung.h"
#include "monster.h"
#include "items.h"

namespace devilution {

// Estado global del sistema
PostDiabloSystemState g_postDiablo;

void InitPostDiabloSystem()
{
	LogVerbose("🗝️ Initializing Nightmare Post-Diablo System");
	
	g_postDiablo.initialized = true;
	g_postDiablo.config.enabled = NIGHTMARE_POST_DIABLO_ENABLED();
	
	// Verificar estado inicial
	if (g_postDiablo.config.enabled) {
		// Verificar si Diablo ya fue derrotado
		if (IsDiabloDefeated()) {
			g_postDiablo.diabloDefeated = true;
			g_postDiablo.currentState = PostDiabloState::PORTAL_AVAILABLE;
			LogVerbose("🔥 Diablo already defeated - Post-Diablo content unlocked!");
		} else {
			g_postDiablo.currentState = PostDiabloState::LOCKED;
			LogVerbose("🔒 Post-Diablo content locked - defeat Diablo first");
		}
	}
	
	LogVerbose("🗝️ Nightmare Post-Diablo System initialized");
	LogVerbose("🌌 The Void Level: {}", g_postDiablo.config.voidLevelNumber);
	LogVerbose("👹 Shadow Boss: {}", g_postDiablo.config.shadowBossEnabled ? "enabled" : "disabled");
	LogVerbose("📊 Current State: {}", static_cast<int>(g_postDiablo.currentState));
}

void UpdatePostDiabloSystem()
{
	if (!g_postDiablo.initialized) {
		return;
	}
	
	// Verificar si la feature está habilitada
	bool shouldBeEnabled = NIGHTMARE_POST_DIABLO_ENABLED();
	if (g_postDiablo.config.enabled != shouldBeEnabled) {
		g_postDiablo.config.enabled = shouldBeEnabled;
		LogVerbose("🗝️ Nightmare Post-Diablo: {}", shouldBeEnabled ? "ENABLED" : "DISABLED");
	}
	
	if (!g_postDiablo.config.enabled) {
		return;
	}
	
	// Verificar progreso automáticamente
	if (!g_postDiablo.diabloDefeated && IsDiabloDefeated()) {
		OnDiabloDefeated();
	}
	
	// Crear portal automáticamente si está en Hell Level 16
	if (g_postDiablo.currentState == PostDiabloState::PORTAL_AVAILABLE && 
	    !g_postDiablo.voidPortalCreated && 
	    currlevel == g_postDiablo.config.portalRequiredLevel && 
	    leveltype == DTYPE_HELL) {
		CreateVoidPortal();
	}
}

bool IsDiabloDefeated()
{
	// Verificar si la quest de Diablo está completada
	return Quests[Q_DIABLO]._qactive == QUEST_DONE;
}

void OnDiabloDefeated()
{
	if (!POST_DIABLO_ENABLED()) {
		return;
	}
	
	LogVerbose("🔥 DIABLO DEFEATED! Unlocking Nightmare Post-Diablo content...");
	
	g_postDiablo.diabloDefeated = true;
	g_postDiablo.currentState = PostDiabloState::PORTAL_AVAILABLE;
	
	// Mostrar mensaje épico al jugador
	LogVerbose("🌌 The fabric of reality tears... The Void awaits in Hell Level 16");
	LogVerbose("👹 Ancient evils stir in the darkness beyond...");
	LogVerbose("🗝️ Seek the Nightmare Portal to face your ultimate challenge");
	
	// En una implementación completa, aquí mostraríamos un mensaje in-game
	// AddInfoBoxString("The Void Portal has opened in Hell Level 16...");
}

bool CreateVoidPortal()
{
	if (!POST_DIABLO_ENABLED() || g_postDiablo.voidPortalCreated) {
		return false;
	}
	
	LogVerbose("🌌 Creating Void Portal in Hell Level 16...");
	
	// Buscar una posición válida para el portal
	// En una implementación completa, buscaríamos una posición libre
	int portalX = 40; // Posición placeholder
	int portalY = 40;
	
	// Crear el portal (en implementación completa usaríamos el sistema de portales real)
	LogVerbose("🗝️ Void Portal created at position ({}, {})", portalX, portalY);
	LogVerbose("🌌 Portal leads to The Void (Level {})", g_postDiablo.config.voidLevelNumber);
	
	g_postDiablo.voidPortalCreated = true;
	g_postDiablo.currentState = PostDiabloState::VOID_ACCESSIBLE;
	
	// En implementación completa:
	// - Crear objeto portal visual
	// - Agregar efectos de partículas
	// - Reproducir sonido épico
	// - Mostrar mensaje al jugador
	
	return true;
}

bool IsVoidPortalAvailable()
{
	return POST_DIABLO_ENABLED() && 
	       g_postDiablo.currentState >= PostDiabloState::VOID_ACCESSIBLE;
}

void GenerateVoidLevel()
{
	if (!POST_DIABLO_ENABLED()) {
		return;
	}
	
	LogVerbose("🌌 Generating The Void - Level {} (Nightmare Dimension)", g_postDiablo.config.voidLevelNumber);
	
	g_postDiablo.voidLevelVisited = true;
	g_postDiablo.voidVisitCount++;
	
	// ============================================================================
	// 🌌 THE VOID - CONFIGURACIÓN DEL NIVEL MÁS ÉPICO
	// ============================================================================
	
	LogVerbose("🌫️ Void Atmosphere: Endless darkness with floating platforms");
	LogVerbose("👻 Void Enemies: Nightmare Wraiths, Shadow Demons, Void Stalkers");
	LogVerbose("💎 Void Treasures: Nightmare Items, Soul Fragments, Dark Artifacts");
	LogVerbose("🏰 Void Architecture: Floating ruins, twisted spires, abyssal chasms");
	
	// En implementación completa:
	// - Generar mapa único con plataformas flotantes
	// - Spawner enemigos únicos nightmare
	// - Colocar tesoros especiales
	// - Configurar iluminación especial (muy tenue)
	// - Agregar efectos de partículas únicos
	// - Música/audio atmosférico especial
	
	// Inicializar NPCs especiales
	InitVoidNPCs();
	
	// Generar boss si está habilitado
	if (g_postDiablo.config.shadowBossEnabled) {
		SpawnDiablosShadow();
	}
	
	LogVerbose("🗝️ The Void generation complete - Welcome to Nightmare's Heart");
}

void InitVoidNPCs()
{
	if (!POST_DIABLO_ENABLED()) {
		return;
	}
	
	LogVerbose("👻 Initializing Void NPCs...");
	
	// ============================================================================
	// 👥 NPCs ÚNICOS DE THE VOID
	// ============================================================================
	
	if (!g_postDiablo.lostWarriorSpawned) {
		// Guerrero Perdido - Vende items únicos
		LogVerbose("⚔️ Spawning Lost Warrior - Nightmare Item Merchant");
		LogVerbose("💬 'I have been trapped here since Diablo's first defeat...'");
		LogVerbose("🛡️ Sells: Nightmare Blade, Veil of Shadows, Band of Dark Sight");
		g_postDiablo.lostWarriorSpawned = true;
	}
	
	if (!g_postDiablo.shadowMerchantSpawned) {
		// Comerciante de Sombras - Intercambia Soul Fragments
		LogVerbose("🌑 Spawning Shadow Merchant - Soul Fragment Trader");
		LogVerbose("💬 'Collect the fragments of Diablo's shattered soul...'");
		LogVerbose("💎 Trades: Soul Fragments for Nightmare Artifacts");
		g_postDiablo.shadowMerchantSpawned = true;
	}
	
	if (!g_postDiablo.voidGuardianSpawned) {
		// Guardián del Vacío - Da quest final
		LogVerbose("🛡️ Spawning Void Guardian - Final Quest Giver");
		LogVerbose("💬 'Face Diablo's Shadow to end the Nightmare forever...'");
		LogVerbose("🎯 Quest: 'The True Evil' - Defeat Diablo's Shadow");
		g_postDiablo.voidGuardianSpawned = true;
	}
	
	LogVerbose("👻 Void NPCs initialized - {} unique characters spawned", 3);
}

void SpawnDiablosShadow()
{
	if (!POST_DIABLO_ENABLED() || !g_postDiablo.config.shadowBossEnabled) {
		return;
	}
	
	LogVerbose("👹 Spawning DIABLO'S SHADOW - Ultimate Nightmare Boss");
	
	// ============================================================================
	// 👹 DIABLO'S SHADOW - BOSS FINAL ÉPICO
	// ============================================================================
	
	LogVerbose("🔥 Boss Name: Diablo's Shadow - The True Evil");
	LogVerbose("💀 Boss Level: 50+ (Nightmare Difficulty)");
	LogVerbose("⚡ Special Abilities:");
	LogVerbose("   🌑 Shadow Apocalypse - Area devastation");
	LogVerbose("   👻 Nightmare Summon - Spawns shadow minions");
	LogVerbose("   🔥 Void Fire - Piercing dark flames");
	LogVerbose("   ⚫ Reality Tear - Teleports and disorients");
	LogVerbose("💎 Drops: Ultimate Nightmare Items, Soul Crown, Void Essence");
	
	// Posición del boss (centro del nivel)
	int bossX = 50; // Placeholder
	int bossY = 50;
	
	LogVerbose("👹 Diablo's Shadow spawned at The Void's heart ({}, {})", bossX, bossY);
	LogVerbose("⚠️ WARNING: This is the ultimate challenge of Nightmare Edition");
	
	// En implementación completa:
	// - Crear monster único con stats épicos
	// - Configurar habilidades especiales
	// - Agregar efectos visuales únicos
	// - Música de boss épica
	// - Mecánicas de combate especiales
}

void OnDiablosShadowDefeated()
{
	if (!POST_DIABLO_ENABLED()) {
		return;
	}
	
	LogVerbose("🏆 DIABLO'S SHADOW DEFEATED! NIGHTMARE EDITION COMPLETED!");
	
	g_postDiablo.shadowBossDefeated = true;
	g_postDiablo.currentState = PostDiabloState::NIGHTMARE_COMPLETE;
	
	// ============================================================================
	// 🏆 VICTORIA ÉPICA - NIGHTMARE EDITION COMPLETADA
	// ============================================================================
	
	LogVerbose("👑 CONGRATULATIONS! You have conquered the ultimate evil!");
	LogVerbose("🌟 Nightmare Edition Status: MASTERED");
	LogVerbose("💎 Ultimate Rewards Unlocked:");
	LogVerbose("   👑 Crown of Nightmares - Ultimate helmet");
	LogVerbose("   ⚔️ Voidbane Sword - Ultimate weapon");
	LogVerbose("   🛡️ Shadowproof Armor - Ultimate protection");
	LogVerbose("   💍 Ring of Eternal Victory - Ultimate accessory");
	
	// Generar recompensas épicas
	GenerateNightmareItem(50, 50, 1); // Crown of Nightmares
	GenerateNightmareItem(51, 50, 2); // Voidbane Sword
	GenerateNightmareItem(50, 51, 3); // Shadowproof Armor
	GenerateNightmareItem(51, 51, 4); // Ring of Eternal Victory
	
	LogVerbose("🎉 Thank you for playing Nightmare Edition!");
	LogVerbose("🌙 The darkness has been conquered... for now.");
}

void GenerateNightmareItem(int x, int y, int itemType)
{
	if (!POST_DIABLO_ENABLED()) {
		return;
	}
	
	g_postDiablo.nightmareItemsFound++;
	
	// Nombres de items únicos nightmare
	const char* itemNames[] = {
		"Unknown Nightmare Item",
		"Crown of Nightmares",
		"Voidbane Sword", 
		"Shadowproof Armor",
		"Ring of Eternal Victory",
		"Nightmare Blade",
		"Veil of Shadows",
		"Band of Dark Sight",
		"Elixir of Tormented Souls",
		"Essence of Nightmare"
	};
	
	const char* itemName = (itemType < 10) ? itemNames[itemType] : itemNames[0];
	
	LogVerbose("💎 Generated Nightmare Item: {} at ({}, {})", itemName, x, y);
	LogVerbose("✨ Item Properties: Unique nightmare stats and abilities");
	
	// En implementación completa:
	// - Crear item con stats únicos
	// - Agregar efectos especiales
	// - Configurar rareza máxima
	// - Efectos visuales únicos
}

PostDiabloState GetPostDiabloState()
{
	return g_postDiablo.currentState;
}

void SetPostDiabloEnabled(bool enabled)
{
	g_postDiablo.config.enabled = enabled;
	LogVerbose("🗝️ Nightmare Post-Diablo manually set to: {}", enabled ? "ENABLED" : "DISABLED");
}

std::string GetPostDiabloProgressText()
{
	switch (g_postDiablo.currentState) {
	case PostDiabloState::LOCKED:
		return "Defeat Diablo to unlock Nightmare content";
	case PostDiabloState::PORTAL_AVAILABLE:
		return "The Void Portal awaits in Hell Level 16";
	case PostDiabloState::VOID_ACCESSIBLE:
		return "The Void is open - Face your ultimate challenge";
	case PostDiabloState::SHADOW_DEFEATED:
		return "Diablo's Shadow defeated - Nightmare nearly complete";
	case PostDiabloState::NIGHTMARE_COMPLETE:
		return "NIGHTMARE EDITION MASTERED - You are the ultimate champion!";
	default:
		return "Unknown state";
	}
}

void CleanupPostDiabloSystem()
{
	LogVerbose("🗝️ Cleaning up Nightmare Post-Diablo System");
	
	g_postDiablo.initialized = false;
	g_postDiablo.currentState = PostDiabloState::LOCKED;
	g_postDiablo.diabloDefeated = false;
	g_postDiablo.voidPortalCreated = false;
	g_postDiablo.voidLevelVisited = false;
	g_postDiablo.shadowBossDefeated = false;
}

} // namespace devilution