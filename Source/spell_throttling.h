#pragma once

/**
 * @file spell_throttling.h
 * 
 * 🎯 UNIVERSAL SPELL THROTTLING SYSTEM
 * 
 * PROBLEMA IDENTIFICADO:
 * Spam de clicks en cualquier hechizo puede causar:
 * - Múltiples missiles simultáneos sin control
 * - Explosión combinatorial en spells complejos
 * - Crashes por saturación del engine
 * 
 * SOLUCIÓN UNIVERSAL:
 * Sistema de throttling inteligente que:
 * - Detecta spam de casts automáticamente
 * - Aplica límites específicos por tipo de spell
 * - Degrada elegantemente sin afectar experiencia
 * - Protege contra crashes de cualquier hechizo
 * 
 * PRINCIPIOS DE DISEÑO:
 * 1. UNIVERSAL - Protege todos los hechizos
 * 2. INTELIGENTE - Límites específicos por spell
 * 3. INVISIBLE - El jugador no nota el throttling
 * 4. CONFIGURABLE - Fácil ajustar por hechizo
 * 5. EXTENSIBLE - Fácil agregar nuevos spells
 */

#include <cstdint>
#include <unordered_map>
#include "spells.h"
#include "missiles.h"

namespace devilution {

// ============================================================================
// 🎯 SPELL THROTTLING CONSTANTS
// ============================================================================

namespace SpellThrottling {
    // Límites generales por defecto
    constexpr int DEFAULT_MAX_MISSILES_PER_SPELL = 5;     // Máximo missiles por spell
    constexpr int DEFAULT_CAST_COOLDOWN_MS = 100;         // Cooldown entre casts (ms)
    constexpr int DEFAULT_EMERGENCY_THRESHOLD = 10;       // Missiles para modo emergencia
    
    // Límites específicos para spells problemáticos
    constexpr int INFERNO_MAX_CONTROLS = 3;               // Máximo InfernoControls
    constexpr int CHAIN_LIGHTNING_MAX_CHAINS = 4;         // Máximo ChainLightnings
    constexpr int FIREBALL_MAX_MISSILES = 6;              // Máximo Fireballs
    constexpr int LIGHTNING_MAX_CONTROLS = 3;             // Máximo LightningControls
    constexpr int FIREWALL_MAX_CONTROLS = 4;              // Máximo FireWallControls
    
    // Cooldowns específicos por spell (ms)
    constexpr int INFERNO_CAST_COOLDOWN = 150;            // Inferno más restrictivo
    constexpr int CHAIN_LIGHTNING_COOLDOWN = 120;         // Chain Lightning restrictivo
    constexpr int FIREBALL_COOLDOWN = 80;                 // Fireball moderado
    constexpr int LIGHTNING_COOLDOWN = 100;               // Lightning normal
    constexpr int FIREWALL_COOLDOWN = 200;                // FireWall muy restrictivo
}

// ============================================================================
// 🎯 SPELL THROTTLING CONFIGURATION
// ============================================================================

struct SpellThrottleConfig {
    int maxMissiles;           // Máximo missiles simultáneos
    int castCooldownMs;        // Cooldown entre casts (ms)
    int emergencyThreshold;    // Threshold para modo emergencia
    bool enabled;              // Si el throttling está activo
    
    SpellThrottleConfig(int maxMissiles = SpellThrottling::DEFAULT_MAX_MISSILES_PER_SPELL,
                       int cooldown = SpellThrottling::DEFAULT_CAST_COOLDOWN_MS,
                       int emergency = SpellThrottling::DEFAULT_EMERGENCY_THRESHOLD,
                       bool enabled = true)
        : maxMissiles(maxMissiles), castCooldownMs(cooldown), 
          emergencyThreshold(emergency), enabled(enabled) {}
};

// ============================================================================
// 🎯 SPELL THROTTLING STATE
// ============================================================================

struct SpellThrottlingState {
    // Timestamps de último cast por spell
    std::unordered_map<SpellID, uint32_t> lastCastTime;
    
    // Contadores de missiles activos por spell
    std::unordered_map<SpellID, int> activeMissileCount;
    
    // Configuración por spell
    std::unordered_map<SpellID, SpellThrottleConfig> spellConfigs;
    
    // Estadísticas de throttling
    int totalCastsPrevented = 0;
    int totalSpellsThrottled = 0;
    
    // Estado global
    bool globalThrottlingActive = false;
    uint32_t lastUpdateTime = 0;
};

// ============================================================================
// 🎯 SPELL THROTTLING API
// ============================================================================

/**
 * Inicializa el sistema universal de throttling de spells
 * Llamar una vez al inicio del juego
 */
void InitSpellThrottling();

/**
 * Actualiza el estado del sistema por frame
 * Llamar al inicio de cada frame de juego
 */
void UpdateSpellThrottling();

/**
 * Verifica si se puede castear un spell de forma segura
 * @param spellId El spell que se quiere castear
 * @param playerId ID del jugador que castea
 * @return true si es seguro castear, false si debe throttling
 */
bool CanCastSpell(SpellID spellId, int playerId);

/**
 * Registra que se casteó un spell
 * Para tracking y cooldowns
 * @param spellId El spell que se casteó
 * @param playerId ID del jugador que casteó
 */
void RegisterSpellCast(SpellID spellId, int playerId);

/**
 * Cuenta missiles activos de un tipo específico
 * @param missileType Tipo de missile a contar
 * @return Número de missiles activos de ese tipo
 */
int CountActiveMissiles(MissileID missileType);

/**
 * Obtiene la configuración de throttling para un spell
 * @param spellId El spell
 * @return Configuración de throttling
 */
const SpellThrottleConfig& GetSpellThrottleConfig(SpellID spellId);

/**
 * Configura throttling personalizado para un spell
 * @param spellId El spell a configurar
 * @param config Nueva configuración
 */
void SetSpellThrottleConfig(SpellID spellId, const SpellThrottleConfig& config);

/**
 * Obtiene el estado actual del sistema de throttling
 * Para debugging y métricas
 */
const SpellThrottlingState& GetSpellThrottlingState();

/**
 * Resetea contadores por frame
 * Llamar al final de cada frame
 */
void ResetSpellThrottlingFrameCounters();

// ============================================================================
// 🎯 MACROS DE CONVENIENCIA
// ============================================================================

/**
 * Macro para verificación rápida antes de cast
 */
#define SPELL_SAFE_CAST(spellId, playerId) \
    (CanCastSpell(spellId, playerId) ? (RegisterSpellCast(spellId, playerId), true) : false)

/**
 * Macro para verificación con logging
 */
#define SPELL_SAFE_CAST_LOG(spellId, playerId, spellName) \
    (CanCastSpell(spellId, playerId) ? (RegisterSpellCast(spellId, playerId), true) : \
     (LogVerbose("Spell throttled: {}", spellName), false))

} // namespace devilution