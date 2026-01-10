#include "spell_throttling.h"

#include "control.h"
#include "engine/random.hpp"
#include "missiles.h"
#include "utils/log.hpp"

namespace devilution {

// ============================================================================
// 🎯 GLOBAL STATE
// ============================================================================

static SpellThrottlingState g_spellThrottling;

// ============================================================================
// 🎯 SPELL-SPECIFIC MISSILE MAPPINGS
// ============================================================================

/**
 * Mapea spells a sus missiles principales para conteo
 */
static const std::unordered_map<SpellID, MissileID> SPELL_TO_MISSILE = {
    { SpellID::Inferno, MissileID::InfernoControl },
    { SpellID::ChainLightning, MissileID::ChainLightning },
    { SpellID::Fireball, MissileID::Fireball },
    { SpellID::Lightning, MissileID::LightningControl },
    { SpellID::FireWall, MissileID::FireWallControl },
    { SpellID::FlameWave, MissileID::FlameWaveControl },
    { SpellID::Guardian, MissileID::Guardian },
    { SpellID::Nova, MissileID::Nova },
    { SpellID::Apocalypse, MissileID::Apocalypse },
    { SpellID::Elemental, MissileID::Elemental },
    { SpellID::Golem, MissileID::Golem },
    { SpellID::ChargedBolt, MissileID::ChargedBolt },
    { SpellID::HolyBolt, MissileID::HolyBolt },
    { SpellID::Firebolt, MissileID::Firebolt }
};

// ============================================================================
// 🎯 IMPLEMENTATION
// ============================================================================

void InitSpellThrottling()
{
    g_spellThrottling = SpellThrottlingState{};
    
    // Configurar throttling específico para spells problemáticos
    g_spellThrottling.spellConfigs[SpellID::Inferno] = SpellThrottleConfig(
        2,  // REDUCIDO: Solo 2 InfernoControls máximo (era 3)
        250,  // AUMENTADO: 250ms cooldown (era 150ms)
        SpellThrottling::DEFAULT_EMERGENCY_THRESHOLD,
        true
    );
    
    g_spellThrottling.spellConfigs[SpellID::ChainLightning] = SpellThrottleConfig(
        2,  // REDUCIDO: Solo 2 ChainLightnings (era 4)
        200,  // AUMENTADO: 200ms cooldown (era 120ms)
        SpellThrottling::DEFAULT_EMERGENCY_THRESHOLD,
        true
    );
    
    g_spellThrottling.spellConfigs[SpellID::Fireball] = SpellThrottleConfig(
        4,  // REDUCIDO: Solo 4 Fireballs (era 6)
        120,  // AUMENTADO: 120ms cooldown (era 80ms)
        SpellThrottling::DEFAULT_EMERGENCY_THRESHOLD,
        true
    );
    
    g_spellThrottling.spellConfigs[SpellID::Lightning] = SpellThrottleConfig(
        2,  // REDUCIDO: Solo 2 LightningControls (era 3)
        150,  // AUMENTADO: 150ms cooldown (era 100ms)
        SpellThrottling::DEFAULT_EMERGENCY_THRESHOLD,
        true
    );
    
    g_spellThrottling.spellConfigs[SpellID::FireWall] = SpellThrottleConfig(
        3,  // REDUCIDO: Solo 3 FireWallControls (era 4)
        300,  // AUMENTADO: 300ms cooldown (era 200ms)
        SpellThrottling::DEFAULT_EMERGENCY_THRESHOLD,
        true
    );
    
    // Configurar throttling moderado para otros spells
    g_spellThrottling.spellConfigs[SpellID::FlameWave] = SpellThrottleConfig(4, 120, 8, true);
    g_spellThrottling.spellConfigs[SpellID::Guardian] = SpellThrottleConfig(3, 200, 6, true);
    g_spellThrottling.spellConfigs[SpellID::Nova] = SpellThrottleConfig(2, 150, 4, true);
    g_spellThrottling.spellConfigs[SpellID::Apocalypse] = SpellThrottleConfig(1, 300, 2, true);
    g_spellThrottling.spellConfigs[SpellID::Elemental] = SpellThrottleConfig(2, 180, 4, true);
    g_spellThrottling.spellConfigs[SpellID::Golem] = SpellThrottleConfig(1, 500, 2, true);
    
    // Configurar throttling ligero para spells básicos
    g_spellThrottling.spellConfigs[SpellID::ChargedBolt] = SpellThrottleConfig(8, 60, 15, true);
    g_spellThrottling.spellConfigs[SpellID::HolyBolt] = SpellThrottleConfig(6, 70, 12, true);
    g_spellThrottling.spellConfigs[SpellID::Firebolt] = SpellThrottleConfig(6, 70, 12, true);
    
    LogVerbose("🎯 Universal Spell Throttling System initialized with {} spell configs", 
               g_spellThrottling.spellConfigs.size());
}

void UpdateSpellThrottling()
{
    uint32_t currentTime = SDL_GetTicks();
    g_spellThrottling.lastUpdateTime = currentTime;
    
    // Actualizar contadores de missiles activos
    g_spellThrottling.activeMissileCount.clear();
    
    for (const auto& [spellId, missileId] : SPELL_TO_MISSILE) {
        int count = CountActiveMissiles(missileId);
        if (count > 0) {
            g_spellThrottling.activeMissileCount[spellId] = count;
        }
    }
    
    // Determinar si throttling global está activo
    int totalActiveMissiles = 0;
    for (const auto& [spellId, count] : g_spellThrottling.activeMissileCount) {
        totalActiveMissiles += count;
    }
    
    // THROTTLING MÁS AGRESIVO: Activar con menos missiles
    g_spellThrottling.globalThrottlingActive = (totalActiveMissiles > 10);  // Era 20, ahora 10
}

bool CanCastSpell(SpellID spellId, int playerId)
{
    uint32_t currentTime = SDL_GetTicks();
    
    // Obtener configuración del spell
    const SpellThrottleConfig& config = GetSpellThrottleConfig(spellId);
    
    // Si throttling está deshabilitado para este spell, permitir siempre
    if (!config.enabled) {
        return true;
    }
    
    // Verificar cooldown de cast
    auto lastCastIt = g_spellThrottling.lastCastTime.find(spellId);
    if (lastCastIt != g_spellThrottling.lastCastTime.end()) {
        uint32_t timeSinceLastCast = currentTime - lastCastIt->second;
        if (timeSinceLastCast < static_cast<uint32_t>(config.castCooldownMs)) {
            g_spellThrottling.totalCastsPrevented++;
            return false;
        }
    }
    
    // Verificar límite de missiles activos
    auto activeMissilesIt = g_spellThrottling.activeMissileCount.find(spellId);
    if (activeMissilesIt != g_spellThrottling.activeMissileCount.end()) {
        if (activeMissilesIt->second >= config.maxMissiles) {
            g_spellThrottling.totalCastsPrevented++;
            return false;
        }
    }
    
    // Si hay throttling global activo, ser más restrictivo
    if (g_spellThrottling.globalThrottlingActive) {
        // THROTTLING EXTREMO: Solo 20% de probabilidad (era 33%)
        if (GenerateRnd(5) != 0) {  // Solo 1 de cada 5 casts
            g_spellThrottling.totalCastsPrevented++;
            return false;
        }
    }
    
    return true;
}

void RegisterSpellCast(SpellID spellId, int playerId)
{
    uint32_t currentTime = SDL_GetTicks();
    g_spellThrottling.lastCastTime[spellId] = currentTime;
    g_spellThrottling.totalSpellsThrottled++;
}

int CountActiveMissiles(MissileID missileType)
{
    int count = 0;
    for (const auto& missile : Missiles) {
        if (missile._mitype == missileType) {
            count++;
        }
    }
    return count;
}

const SpellThrottleConfig& GetSpellThrottleConfig(SpellID spellId)
{
    auto it = g_spellThrottling.spellConfigs.find(spellId);
    if (it != g_spellThrottling.spellConfigs.end()) {
        return it->second;
    }
    
    // Configuración por defecto para spells no configurados
    static const SpellThrottleConfig defaultConfig;
    return defaultConfig;
}

void SetSpellThrottleConfig(SpellID spellId, const SpellThrottleConfig& config)
{
    g_spellThrottling.spellConfigs[spellId] = config;
}

const SpellThrottlingState& GetSpellThrottlingState()
{
    return g_spellThrottling;
}

void ResetSpellThrottlingFrameCounters()
{
    // Los contadores se resetean automáticamente en UpdateSpellThrottling
    // Esta función existe para consistencia con otros sistemas
}

} // namespace devilution