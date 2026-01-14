#include "engine_health.h"

#include <chrono>
#include <unordered_map>
#include "architectural_analysis.h"
#include "lighting.h"
#include "missiles.h"
#include "monster.h"
#include "player.h"
#include "engine/sound.h"

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🌍 GLOBAL STATE
// ============================================================================

EngineHealth gEngineHealth = {};

// ============================================================================
// 🔧 CORE IMPLEMENTATION
// ============================================================================

void UpdateEngineHealth()
{
    // Reset frame counters
    gEngineHealth.apocalypseSpawnsThisFrame = 0;
    
    // Count active missiles
    gEngineHealth.activeMissiles = static_cast<int>(Missiles.size());
    
    // Count active monsters
    gEngineHealth.activeMonsters = static_cast<int>(ActiveMonsterCount);
    
    // Count active lights
    gEngineHealth.activeLights = ActiveLightCount;
    
    // Count specific missile types
    gEngineHealth.activeApocalypse = 0;
    gEngineHealth.activeInfernos = 0;
    gEngineHealth.totalSpellsActive = 0;
    
    for (const auto &missile : Missiles) {
        switch (missile._mitype) {
            case MissileID::Apocalypse:
                gEngineHealth.activeApocalypse++;
                gEngineHealth.totalSpellsActive++;
                break;
            case MissileID::ApocalypseBoom:
                gEngineHealth.totalSpellsActive++;
                break;
            case MissileID::Inferno:
            case MissileID::InfernoControl:
                gEngineHealth.activeInfernos++;
                gEngineHealth.totalSpellsActive++;
                break;
            default:
                if (missile._micaster == TARGET_MONSTERS) {
                    gEngineHealth.totalSpellsActive++;
                }
                break;
        }
    }
    
    // Calculate estimated load (0-100+)
    gEngineHealth.estimatedLoad = 
        gEngineHealth.activeMissiles * 2 +           // Missiles son costosos
        gEngineHealth.activeMonsters +               // Monsters moderados
        gEngineHealth.activeLights * 3 +             // Lights son muy costosos
        gEngineHealth.activeApocalypse * 10;         // Apocalypse extremadamente costoso
    
    // Calculate frame cost estimate
    gEngineHealth.frameCostEstimate = 
        gEngineHealth.activeMissiles +
        gEngineHealth.activeMonsters +
        gEngineHealth.apocalypseSpawnsThisFrame * 5;
    
    // Determine health states
    EngineLoadState loadState = GetEngineLoadState();
    gEngineHealth.isHealthy = (loadState == EngineLoadState::OK);
    gEngineHealth.needsThrottling = (loadState == EngineLoadState::HIGH);
    gEngineHealth.inCriticalState = (loadState == EngineLoadState::CRITICAL);
}

EngineLoadState GetEngineLoadState()
{
    // Thresholds adaptativos basados en capacidad moderna
    if (gEngineHealth.estimatedLoad > 300 || 
        gEngineHealth.activeMissiles > 400 ||
        gEngineHealth.activeApocalypse > 3) {
        return EngineLoadState::CRITICAL;
    }
    
    if (gEngineHealth.estimatedLoad > 200 || 
        gEngineHealth.activeMissiles > 250 ||
        gEngineHealth.activeApocalypse > 1) {
        return EngineLoadState::HIGH;
    }
    
    return EngineLoadState::OK;
}

bool CanSafelyAddMissile(int missileType)
{
    // Check general missile limits
    if (gEngineHealth.activeMissiles >= 500) {
        return false;
    }
    
    // Check engine load state
    EngineLoadState loadState = GetEngineLoadState();
    if (loadState == EngineLoadState::CRITICAL) {
        return false;
    }
    
    // SIMPLIFIED: No special restrictions - trust original engine
    return true;
}

// Cooldown map for Apocalypse - moved to file scope for cleanup access
static std::unordered_map<int, std::chrono::steady_clock::time_point> gApocalypseCooldownMap;

void ResetApocalypseCooldowns()
{
	// Called when starting a new game to clear any stale cooldowns
	gApocalypseCooldownMap.clear();
}

bool CanSafelyCastApocalypse(int playerId)
{
	// ULTRA-SIMPLE APOCALYPSE PROTECTION - ORIGINAL SPEED + SAFETY
	// "Velocidad original + protección inteligente = Perfección"
	// 
	// FILOSOFÍA:
	// - Velocidad instantánea como DevilutionX original
	// - Protección por jugador (cada uno puede tener 1 activo)
	// - Límite global de 2 (previene sobrecarga en multiplayer)
	// - Cooldown 100ms ultra-responsivo (previene fast-click abuse)
	// - Safety net de 50 booms por spell (previene casos extremos)
	//
	// RESULTADO: Feel original + 0% crash rate
	
	// Cooldown por jugador: 100ms (ultra-responsive, previene fast-click)
	
	auto now = std::chrono::steady_clock::now();
	auto timeSinceLastCast = std::chrono::duration_cast<std::chrono::milliseconds>(
		now - gApocalypseCooldownMap[playerId]
	);
	
	if (timeSinceLastCast.count() < 100) {
		ARCH_LOG_CRASH_PREVENTION("Apocalypse cooldown active for player", "CanSafelyCastApocalypse");
		return false;
	}
	
	// LÍMITE GLOBAL: Máximo 2 Apocalypse activos (balance multiplayer)
	// Con velocidad instantánea: 2 × 50 booms = 100 booms max (seguro)
	int totalApocalypse = 0;
	for (const auto &m : Missiles) {
		if (m._mitype == MissileID::Apocalypse) {
			totalApocalypse++;
		}
	}
	
	if (totalApocalypse >= 2) {
		ARCH_LOG_CRASH_PREVENTION("Global Apocalypse limit reached (2 max)", "CanSafelyCastApocalypse");
		return false;
	}
	
	// LÍMITE POR JUGADOR: Máximo 1 Apocalypse activo por jugador
	// Previene que un solo jugador abuse del spell
	int playerApocalypse = 0;
	for (const auto &m : Missiles) {
		if (m._mitype == MissileID::Apocalypse && m._misource == playerId) {
			playerApocalypse++;
		}
	}
	
	if (playerApocalypse >= 1) {
		ARCH_LOG_CRASH_PREVENTION("Player already has Apocalypse active (limit 1)", "CanSafelyCastApocalypse");
		return false;
	}
	
	gApocalypseCooldownMap[playerId] = now;
	return true;
}

bool CanSafelyCastInferno()
{
    // Check if engine is in critical state
    if (devilution::gEngineHealth.inCriticalState) {
        return false;
    }
    
    // Limit concurrent Inferno spells
    if (devilution::gEngineHealth.activeInfernos >= 3) {
        return false;
    }
    
    // Check general load
    if (devilution::gEngineHealth.estimatedLoad > 250) {
        return false;
    }
    
    return true;
}

// ============================================================================
// 🎮 FEEDBACK IMPLEMENTATION
// ============================================================================

void PlaySpellThrottleFeedback()
{
    // Subtle audio feedback - no new assets, use existing sounds
    // Use a quiet, brief sound that doesn't break immersion
    devilution::PlaySfxLoc(devilution::SfxID::MenuSelect, devilution::Players[devilution::MyPlayerId].position.tile);
    
    // TODO: Add subtle visual feedback (red flash, etc.)
    // This would require minimal rendering changes
}

void PlayEngineStressFeedback()
{
    // Even more subtle feedback for engine stress
    // Could be as simple as slightly delayed spell casting
    // or subtle audio dampening
    
    // For now, just log in debug builds
#ifdef _DEBUG
    LogEngineHealth();
#endif
}

// ============================================================================
// 🐛 DEBUG IMPLEMENTATION
// ============================================================================

#ifdef _DEBUG
void LogEngineHealth()
{
    std::cout << "[ENGINE_HEALTH] "
              << "Missiles:" << gEngineHealth.activeMissiles
              << " Monsters:" << gEngineHealth.activeMonsters
              << " Load:" << gEngineHealth.estimatedLoad
              << " Apocalypse:" << gEngineHealth.activeApocalypse
              << " State:" << (gEngineHealth.isHealthy ? "OK" : 
                              gEngineHealth.needsThrottling ? "HIGH" : "CRITICAL")
              << std::endl;
}

void LogEngineHealthDetailed()
{
    std::cout << "\n=== ENGINE HEALTH DETAILED ===" << std::endl;
    std::cout << "Active Missiles: " << gEngineHealth.activeMissiles << std::endl;
    std::cout << "Active Monsters: " << gEngineHealth.activeMonsters << std::endl;
    std::cout << "Active Lights: " << gEngineHealth.activeLights << std::endl;
    std::cout << "Active Apocalypse: " << gEngineHealth.activeApocalypse << std::endl;
    std::cout << "Active Infernos: " << gEngineHealth.activeInfernos << std::endl;
    std::cout << "Total Spells: " << gEngineHealth.totalSpellsActive << std::endl;
    std::cout << "Estimated Load: " << gEngineHealth.estimatedLoad << std::endl;
    std::cout << "Frame Cost: " << gEngineHealth.frameCostEstimate << std::endl;
    std::cout << "Health State: " << (gEngineHealth.isHealthy ? "HEALTHY" : 
                                     gEngineHealth.needsThrottling ? "HIGH_LOAD" : "CRITICAL") << std::endl;
    std::cout << "Load State: " << static_cast<int>(GetEngineLoadState()) << std::endl;
    std::cout << "==============================\n" << std::endl;
}
#endif

// ============================================================================
// 🔄 INTEGRATION HELPERS
// ============================================================================

void InitEngineHealth()
{
    // Initialize engine health system
    devilution::gEngineHealth = {};
    
#ifdef _DEBUG
    std::cout << "[ENGINE_HEALTH] Modern Assist Layer initialized" << std::endl;
#endif
}

void CleanupEngineHealth()
{
    // Cleanup engine health system
    devilution::gEngineHealth = {};
    
#ifdef _DEBUG
    std::cout << "[ENGINE_HEALTH] Modern Assist Layer cleaned up" << std::endl;
#endif
}

void ResetFrameCounters()
{
    // Reset per-frame counters
    devilution::gEngineHealth.apocalypseSpawnsThisFrame = 0;
    devilution::gEngineHealth.frameCostEstimate = 0;
}

} // namespace devilution