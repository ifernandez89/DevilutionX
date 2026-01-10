#include "inferno_defense.h"

#include "control.h"
#include "engine/random.hpp"
#include "monster.h"
#include "utils/log.hpp"

namespace devilution {

// ============================================================================
// 🔥 GLOBAL STATE
// ============================================================================

static InfernoDefenseState g_infernoDefense;

// ============================================================================
// 🔥 IMPLEMENTATION
// ============================================================================

void InitInfernoDefense()
{
    g_infernoDefense = InfernoDefenseState{};
    LogVerbose("🔥 Inferno Defense System initialized");
}

void UpdateInfernoDefense()
{
    // Obtener tiempo actual
    uint32_t currentTime = SDL_GetTicks();
    
    // Contar InfernoControls activos para ajustar throttling
    int activeInfernoControls = 0;
    for (const auto &missile : Missiles) {
        if (missile._mitype == MissileID::InfernoControl) {
            activeInfernoControls++;
        }
    }
    
    // Contar enemigos cerca del jugador para determinar modo
    if (MyPlayer != nullptr) {
        g_infernoDefense.nearbyEnemyCount = CountNearbyEnemies(MyPlayer->position.tile, 8);
        
        // Determinar modo de operación (más agresivo con múltiples InfernoControls)
        int adjustedThreshold = InfernoDefense::INFERNO_THROTTLE_THRESHOLD;
        int adjustedEmergencyThreshold = InfernoDefense::INFERNO_EMERGENCY_THRESHOLD;
        
        // Si hay múltiples InfernoControls, ser más restrictivo
        if (activeInfernoControls >= 2) {
            adjustedThreshold = 10;  // Más restrictivo
            adjustedEmergencyThreshold = 15;  // Emergencia antes
        }
        
        if (g_infernoDefense.nearbyEnemyCount >= adjustedEmergencyThreshold || activeInfernoControls >= 3) {
            g_infernoDefense.isEmergencyMode = true;
            g_infernoDefense.isThrottlingActive = true;
        } else if (g_infernoDefense.nearbyEnemyCount >= adjustedThreshold || activeInfernoControls >= 2) {
            g_infernoDefense.isEmergencyMode = false;
            g_infernoDefense.isThrottlingActive = true;
        } else {
            g_infernoDefense.isEmergencyMode = false;
            g_infernoDefense.isThrottlingActive = false;
        }
    }
    
    // Actualizar timers
    g_infernoDefense.lastInfernoSpawnTime = currentTime;
    g_infernoDefense.lastInfernoDamageTime = currentTime;
}

bool CanSpawnInfernoMissile(Point position)
{
    // Si no hay throttling activo, permitir siempre
    if (!g_infernoDefense.isThrottlingActive) {
        return true;
    }
    
    // Verificar límite por frame
    if (g_infernoDefense.infernoMissilesSpawnedThisFrame >= InfernoDefense::MAX_INFERNO_MISSILES_PER_FRAME) {
        return false;
    }
    
    // Contar InfernoControls activos para throttling más agresivo
    int activeInfernoControls = 0;
    for (const auto &missile : Missiles) {
        if (missile._mitype == MissileID::InfernoControl) {
            activeInfernoControls++;
        }
    }
    
    // Con múltiples InfernoControls, ser MUY restrictivo
    if (activeInfernoControls >= 2) {
        // Solo permitir 1 de cada 4 spawns con múltiples controles
        if (GenerateRnd(4) != 0) {
            g_infernoDefense.totalInfernoMissilesPrevented++;
            return false;
        }
    }
    
    // En modo emergencia, ser más restrictivo
    if (g_infernoDefense.isEmergencyMode) {
        // Solo permitir 1 de cada 3 spawns en emergencia
        if (GenerateRnd(3) != 0) {
            g_infernoDefense.totalInfernoMissilesPrevented++;
            return false;
        }
    }
    
    // Verificar cooldown temporal
    uint32_t currentTime = SDL_GetTicks();
    if (currentTime - g_infernoDefense.lastInfernoSpawnTime < InfernoDefense::INFERNO_SPAWN_COOLDOWN_MS) {
        // Durante cooldown, solo permitir spawns ocasionales
        if (GenerateRnd(4) != 0) {
            g_infernoDefense.totalInfernoMissilesPrevented++;
            return false;
        }
    }
    
    return true;
}

bool CanDoInfernoCollisionCheck(const Missile &missile, Point position)
{
    // Si no hay throttling activo, permitir siempre
    if (!g_infernoDefense.isThrottlingActive) {
        return true;
    }
    
    // Verificar límite por frame
    if (g_infernoDefense.infernoCollisionChecksThisFrame >= InfernoDefense::MAX_INFERNO_COLLISION_CHECKS) {
        return false;
    }
    
    // En modo emergencia, ser más restrictivo con collision checks
    if (g_infernoDefense.isEmergencyMode) {
        // Solo permitir 1 de cada 2 collision checks en emergencia
        if (GenerateRnd(2) == 0) {
            g_infernoDefense.totalCollisionChecksPrevented++;
            return false;
        }
    }
    
    return true;
}

bool CanApplyInfernoDamage(int damage, Point position)
{
    // Si no hay throttling activo, permitir siempre
    if (!g_infernoDefense.isThrottlingActive) {
        return true;
    }
    
    // Verificar límite por frame
    if (g_infernoDefense.infernoDamageApplicationsThisFrame >= InfernoDefense::MAX_INFERNO_DAMAGE_APPLICATIONS) {
        return false;
    }
    
    // En modo emergencia, throttling más agresivo
    if (g_infernoDefense.isEmergencyMode) {
        // Verificar cooldown temporal para damage
        uint32_t currentTime = SDL_GetTicks();
        if (currentTime - g_infernoDefense.lastInfernoDamageTime < InfernoDefense::INFERNO_DAMAGE_COOLDOWN_MS) {
            g_infernoDefense.totalDamageApplicationsPrevented++;
            return false;
        }
    }
    
    return true;
}

void RegisterInfernoMissileSpawn()
{
    g_infernoDefense.infernoMissilesSpawnedThisFrame++;
}

void RegisterInfernoCollisionCheck()
{
    g_infernoDefense.infernoCollisionChecksThisFrame++;
}

void RegisterInfernoDamageApplication()
{
    g_infernoDefense.infernoDamageApplicationsThisFrame++;
    g_infernoDefense.lastInfernoDamageTime = SDL_GetTicks();
}

int CountNearbyEnemies(Point center, int radius)
{
    int count = 0;
    
    // Iterar en área cuadrada alrededor del centro
    for (int x = center.x - radius; x <= center.x + radius; x++) {
        for (int y = center.y - radius; y <= center.y + radius; y++) {
            Point pos = {x, y};
            
            // Verificar bounds
            if (!InDungeonBounds(pos)) {
                continue;
            }
            
            // Verificar si hay monster en esta posición
            int monsterId = dMonster[x][y];
            if (monsterId != 0) {
                Monster &monster = Monsters[std::abs(monsterId) - 1];
                
                // Solo contar monsters activos y vivos
                if (monster.mode != MonsterMode::Death && 
                    monster.mode != MonsterMode::FadeIn &&
                    monster.mode != MonsterMode::FadeOut &&
                    monster.hitPoints > 0) {
                    count++;
                }
            }
        }
    }
    
    return count;
}

const InfernoDefenseState& GetInfernoDefenseState()
{
    return g_infernoDefense;
}

void ResetInfernoDefenseFrameCounters()
{
    g_infernoDefense.infernoMissilesSpawnedThisFrame = 0;
    g_infernoDefense.infernoCollisionChecksThisFrame = 0;
    g_infernoDefense.infernoDamageApplicationsThisFrame = 0;
}

} // namespace devilution