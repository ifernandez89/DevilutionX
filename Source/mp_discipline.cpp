/**
 * @file mp_discipline.cpp
 * 
 * 🎯 FASE MP1 - MULTIPLAYER DISCIPLINE RESCUE 🎯
 * 
 * Implementación completa del sistema de disciplina multiplayer
 */

#include "mp_discipline.h"

#include <algorithm>
#include <cmath>
#include <chrono>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/random.hpp"

// Forward declarations to avoid heavy includes
extern int MyPlayerId;

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔒 ESTADO GLOBAL DEL SISTEMA
// ============================================================================

namespace {
    MPDisciplineState g_mpState;
    MPDisciplineStats g_mpStats;
    
#ifdef _DEBUG
    bool g_debugLogging = false;
    GameContext g_forcedContext = GameContext::Normal;
    PressureLevel g_forcedPressure = PressureLevel::Low;
    bool g_contextForced = false;
    bool g_pressureForced = false;
#endif
}

// ============================================================================
// 🎯 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitMPDiscipline()
{
    // Inicializar estado
    g_mpState = {};
    g_mpStats = {};
    
    // Configuración por defecto (preset balanceado)
    ApplyBalancedPreset();
    
    uint32_t currentTime = SDL_GetTicks();
    g_mpState.lastSpawnTime = currentTime;
    g_mpState.lastDotTick = currentTime;
    g_mpState.lastBuffTick = currentTime;
    g_mpState.lastStateValidation = currentTime;
    
#ifdef _DEBUG
    std::cout << "MP Discipline System initialized" << std::endl;
#endif
}

void UpdateMPDiscipline()
{
    if (!g_mpState.enabled) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    uint32_t currentTime = SDL_GetTicks();
    
    // Actualizar métricas de presión
    g_mpState.currentPressure = DetectEnginePressure();
    g_mpState.currentContext = DetectGameContext();
    g_mpState.currentStrategy = DetermineBalanceStrategy();
    
    // Actualizar estadísticas
    g_mpStats.totalFramesProcessed++;
    g_mpStats.contextActivations[static_cast<size_t>(g_mpState.currentContext)]++;
    g_mpStats.pressureLevelFrames[static_cast<size_t>(g_mpState.currentPressure)]++;
    
    if (g_mpState.currentPressure >= PressureLevel::Medium) {
        g_mpStats.framesWithMPRules++;
    }
    
    if (g_mpState.currentPressure == PressureLevel::Critical) {
        g_mpStats.totalEmergencyActivations++;
        g_mpState.emergencyFallbacks++;
        g_mpState.emergencyMode = true;
    } else {
        g_mpState.emergencyMode = false;
    }
    
    // Validación de estado periódica (rescatada de MP)
    if (currentTime - g_mpState.lastStateValidation >= g_mpState.balance.stateValidationInterval) {
        if (ValidateGameState()) {
            g_mpState.stateValidations++;
            g_mpStats.totalStateValidations++;
        }
        g_mpState.lastStateValidation = currentTime;
    }
    
    // Actualizar peak pressure
    if (static_cast<uint32_t>(g_mpState.currentPressure) > g_mpStats.peakPressureLevel) {
        g_mpStats.peakPressureLevel = static_cast<uint32_t>(g_mpState.currentPressure);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Actualizar tiempo promedio de frame
    float frameTime = duration.count() / 1000.0f; // Convert to milliseconds
    g_mpStats.averageFrameTime = (g_mpStats.averageFrameTime * 0.95f) + (frameTime * 0.05f);
    g_mpState.metrics.frameTime = static_cast<uint32_t>(frameTime);
    
#ifdef _DEBUG
    if (g_debugLogging && g_mpStats.totalFramesProcessed % 300 == 0) {
        std::cout << "MP Discipline: " << static_cast<int>(g_mpState.currentPressure) << " pressure, "
                  << static_cast<int>(g_mpState.currentContext) << " context, "
                  << static_cast<int>(g_mpState.currentStrategy) << " strategy, "
                  << std::fixed << std::setprecision(2) << frameTime << "ms" << std::endl;
    }
#endif
}

// ============================================================================
// 🎯 DETECCIÓN DE PRESIÓN Y CONTEXTO
// ============================================================================

PressureLevel DetectEnginePressure()
{
#ifdef _DEBUG
    if (g_pressureForced) {
        return g_forcedPressure;
    }
#endif
    
    const auto &metrics = g_mpState.metrics;
    
    // Contar factores de presión
    int pressureFactors = 0;
    
    if (metrics.activeMonsters > EnginePressureMetrics::MAX_SAFE_MONSTERS * 0.7f) pressureFactors++;
    if (metrics.activeMissiles > EnginePressureMetrics::MAX_SAFE_MISSILES * 0.7f) pressureFactors++;
    if (metrics.activeEffects > EnginePressureMetrics::MAX_SAFE_EFFECTS * 0.7f) pressureFactors++;
    if (metrics.spawnRequests > EnginePressureMetrics::MAX_SAFE_SPAWNS_PER_FRAME) pressureFactors++;
    if (metrics.damageCalculations > EnginePressureMetrics::MAX_SAFE_DAMAGE_CALCS) pressureFactors++;
    if (metrics.frameTime > EnginePressureMetrics::MAX_SAFE_FRAME_TIME) pressureFactors++;
    
    // Determinar nivel de presión
    if (pressureFactors >= 4) {
        return PressureLevel::Critical;
    } else if (pressureFactors >= 2) {
        return PressureLevel::High;
    } else if (pressureFactors >= 1) {
        return PressureLevel::Medium;
    } else {
        return PressureLevel::Low;
    }
}

GameContext DetectGameContext()
{
#ifdef _DEBUG
    if (g_contextForced) {
        return g_forcedContext;
    }
#endif
    
    // Detectar contexto basado en estado del juego
    if (g_mpState.infernoActive) {
        return GameContext::Inferno;
    }
    
    if (g_mpState.elitePackActive) {
        return GameContext::ElitePack;
    }
    
    if (g_mpState.currentPressure >= PressureLevel::High) {
        return GameContext::HighStress;
    }
    
    if (g_mpState.currentPressure == PressureLevel::Critical) {
        return GameContext::Emergency;
    }
    
    // TODO: Detectar endgame basado en nivel cuando tengamos acceso a leveltype
    // For now, assume normal context
    return GameContext::Normal;
}

BalanceStrategy DetermineBalanceStrategy()
{
    if (g_mpState.currentStrategy != BalanceStrategy::Auto) {
        return g_mpState.currentStrategy; // Estrategia manual
    }
    
    // Determinar estrategia automáticamente
    switch (g_mpState.currentContext) {
    case GameContext::Emergency:
        return BalanceStrategy::Multi; // Máxima disciplina MP
        
    case GameContext::HighStress:
    case GameContext::Inferno:
        return BalanceStrategy::Hybrid; // Combinación inteligente
        
    case GameContext::ElitePack:
    case GameContext::Endgame:
        return BalanceStrategy::Hybrid; // Balance mejorado
        
    case GameContext::Normal:
    default:
        return BalanceStrategy::Single; // Reglas originales
    }
}

// ============================================================================
// 🎯 FUNCIONES DE RESCATE MP
// ============================================================================

bool ShouldUseMultiplayerSpawnLogic()
{
    return g_mpState.currentStrategy == BalanceStrategy::Multi ||
           (g_mpState.currentStrategy == BalanceStrategy::Hybrid && 
            g_mpState.currentPressure >= PressureLevel::Medium);
}

bool ShouldUseMultiplayerBalance()
{
    return g_mpState.currentStrategy == BalanceStrategy::Multi ||
           (g_mpState.currentStrategy == BalanceStrategy::Hybrid && 
            (g_mpState.currentContext == GameContext::Endgame ||
             g_mpState.currentContext == GameContext::ElitePack));
}

bool ShouldUseDiscreteTimers()
{
    return g_mpState.currentStrategy != BalanceStrategy::Single;
}

float ApplyMultiplayerDamageModifier(float originalDamage, bool isSpell)
{
    if (!ShouldUseMultiplayerBalance()) {
        return originalDamage;
    }
    
    float modifier = isSpell ? 
        g_mpState.balance.spellDamageMultiplier : 
        g_mpState.balance.damageMultiplier;
    
    // Aplicar reducción de emergencia si es necesario
    if (g_mpState.emergencyMode) {
        modifier *= g_mpState.balance.emergencyDamageReduction;
    }
    
    return originalDamage * modifier;
}

int ApplyMultiplayerHealthModifier(int originalHealth)
{
    if (!ShouldUseMultiplayerBalance()) {
        return originalHealth;
    }
    
    return static_cast<int>(originalHealth * g_mpState.balance.healthMultiplier);
}

bool CanSpawnMonster()
{
    if (!ShouldUseMultiplayerSpawnLogic()) {
        return true; // Usar lógica original de single player
    }
    
    uint32_t currentTime = SDL_GetTicks();
    
    // Verificar cooldown entre spawns
    if (currentTime - g_mpState.lastSpawnTime < g_mpState.balance.spawnCooldown) {
        return false;
    }
    
    // Verificar límite de spawns por frame
    if (g_mpState.metrics.spawnRequests >= g_mpState.balance.maxSpawnsPerFrame) {
        return false;
    }
    
    // Verificar límite total de monstruos
    if (g_mpState.metrics.activeMonsters >= g_mpState.balance.maxActiveMonsters) {
        return false;
    }
    
    // Aplicar reducción de emergencia
    if (g_mpState.emergencyMode) {
        // Solo permitir spawns ocasionales en modo de emergencia
        return (RandomIntBetween(0, 100) < (g_mpState.balance.emergencySpawnReduction * 100));
    }
    
    return true;
}

void RegisterMonsterSpawn()
{
    g_mpState.lastSpawnTime = SDL_GetTicks();
    g_mpState.metrics.spawnRequests++;
    g_mpState.metrics.activeMonsters++; // Simplificado, debería ser actualizado por el sistema de monstruos
}

// ============================================================================
// 🎯 VALIDACIÓN DE ESTADO (RESCATADO DE MP)
// ============================================================================

bool ValidateGameState()
{
    bool isValid = true;
    
    // Validaciones básicas rescatadas de la disciplina MP
    
    // 1. Verificar que las métricas estén en rangos razonables
    if (g_mpState.metrics.activeMonsters > 200) {
        g_mpState.metrics.activeMonsters = 200; // Cap de seguridad
        isValid = false;
    }
    
    if (g_mpState.metrics.activeMissiles > 500) {
        g_mpState.metrics.activeMissiles = 500; // Cap de seguridad
        isValid = false;
    }
    
    if (g_mpState.metrics.activeEffects > 1000) {
        g_mpState.metrics.activeEffects = 1000; // Cap de seguridad
        isValid = false;
    }
    
    // 2. Verificar que los timers no estén en el futuro
    uint32_t currentTime = SDL_GetTicks();
    if (g_mpState.lastSpawnTime > currentTime + 1000) {
        g_mpState.lastSpawnTime = currentTime;
        isValid = false;
    }
    
    // 3. Reset contadores por frame
    g_mpState.metrics.spawnRequests = 0;
    g_mpState.metrics.damageCalculations = 0;
    
    return isValid;
}

bool ValidateMonsterState(int monsterId)
{
    // Validación simplificada - en implementación real verificaría:
    // - HP válido
    // - Posición válida  
    // - Estado de animación válido
    // - Flags consistentes
    
    return (monsterId >= 0 && monsterId < 200); // Simplificado
}

bool ValidatePlayerState()
{
    // Validación simplificada - en implementación real verificaría:
    // - HP/Mana válidos
    // - Posición válida
    // - Inventario consistente
    // - Stats válidos
    
    return true; // Simplificado por ahora
}

bool RepairCorruptedState()
{
    // Reparaciones básicas rescatadas de MP
    bool repaired = false;
    
    // Reparar métricas fuera de rango
    if (g_mpState.metrics.activeMonsters > 200) {
        g_mpState.metrics.activeMonsters = 100; // Valor seguro
        repaired = true;
    }
    
    if (g_mpState.metrics.activeMissiles > 500) {
        g_mpState.metrics.activeMissiles = 50; // Valor seguro
        repaired = true;
    }
    
    // Reset timers si están corruptos
    uint32_t currentTime = SDL_GetTicks();
    if (g_mpState.lastSpawnTime > currentTime + 10000) {
        g_mpState.lastSpawnTime = currentTime;
        repaired = true;
    }
    
    return repaired;
}

// ============================================================================
// 🎯 TIMERS DISCRETOS (RESCATADOS DE MP)
// ============================================================================

bool ShouldTickDOT()
{
    if (!ShouldUseDiscreteTimers()) {
        return true; // Usar lógica original frame-based
    }
    
    uint32_t currentTime = SDL_GetTicks();
    return (currentTime - g_mpState.lastDotTick >= g_mpState.balance.dotTickInterval);
}

bool ShouldTickBuffs()
{
    if (!ShouldUseDiscreteTimers()) {
        return true; // Usar lógica original frame-based
    }
    
    uint32_t currentTime = SDL_GetTicks();
    return (currentTime - g_mpState.lastBuffTick >= g_mpState.balance.buffTickInterval);
}

void RegisterDOTTick()
{
    g_mpState.lastDotTick = SDL_GetTicks();
}

void RegisterBuffTick()
{
    g_mpState.lastBuffTick = SDL_GetTicks();
}

// ============================================================================
// 🎯 CONFIGURACIÓN Y PRESETS
// ============================================================================

void ApplyConservativePreset()
{
    g_mpState.enabled = true;
    g_mpState.currentStrategy = BalanceStrategy::Auto;
    
    // Balance muy conservador (máxima estabilidad)
    g_mpState.balance = {
        .damageMultiplier = 0.7f,           // Reducir daño significativamente
        .healthMultiplier = 1.3f,           // Aumentar vida considerablemente
        .spellDamageMultiplier = 0.6f,      // Spells muy conservadores
        .maxSpawnsPerFrame = 2,             // Spawns muy limitados
        .spawnCooldown = 200,               // Cooldown largo
        .maxActiveMonsters = 40,            // Límite bajo de monstruos
        .dotTickInterval = 1500,            // DOT más lento
        .buffTickInterval = 750,            // Buffs más lentos
        .stateValidationInterval = 1000,    // Validación frecuente
        .emergencyDamageReduction = 0.3f,   // Reducción drástica en emergencia
        .emergencySpawnReduction = 0.1f     // Spawns mínimos en emergencia
    };
}

void ApplyBalancedPreset()
{
    g_mpState.enabled = true;
    g_mpState.currentStrategy = BalanceStrategy::Auto;
    
    // Balance equilibrado (estabilidad + performance)
    g_mpState.balance = {
        .damageMultiplier = 0.85f,          // Reducción moderada de daño
        .healthMultiplier = 1.15f,          // Aumento moderado de vida
        .spellDamageMultiplier = 0.8f,      // Spells ligeramente conservadores
        .maxSpawnsPerFrame = 3,             // Spawns moderados
        .spawnCooldown = 100,               // Cooldown moderado
        .maxActiveMonsters = 60,            // Límite moderado de monstruos
        .dotTickInterval = 1000,            // DOT cada segundo
        .buffTickInterval = 500,            // Buffs cada medio segundo
        .stateValidationInterval = 2000,    // Validación cada 2 segundos
        .emergencyDamageReduction = 0.5f,   // Reducción moderada en emergencia
        .emergencySpawnReduction = 0.3f     // Spawns reducidos en emergencia
    };
}

void ApplyAggressivePreset()
{
    g_mpState.enabled = true;
    g_mpState.currentStrategy = BalanceStrategy::Auto;
    
    // Balance agresivo (mínima intervención MP)
    g_mpState.balance = {
        .damageMultiplier = 0.95f,          // Reducción mínima de daño
        .healthMultiplier = 1.05f,          // Aumento mínimo de vida
        .spellDamageMultiplier = 0.9f,      // Spells casi originales
        .maxSpawnsPerFrame = 5,             // Spawns más permisivos
        .spawnCooldown = 50,                // Cooldown corto
        .maxActiveMonsters = 80,            // Límite alto de monstruos
        .dotTickInterval = 750,             // DOT más rápido
        .buffTickInterval = 250,            // Buffs más rápidos
        .stateValidationInterval = 3000,    // Validación menos frecuente
        .emergencyDamageReduction = 0.7f,   // Reducción mínima en emergencia
        .emergencySpawnReduction = 0.5f     // Spawns moderados en emergencia
    };
}

void SetBalanceStrategy(BalanceStrategy strategy)
{
    g_mpState.currentStrategy = strategy;
}

const MPDisciplineState &GetMPDisciplineState()
{
    return g_mpState;
}

const MPDisciplineStats &GetMPDisciplineStats()
{
    return g_mpStats;
}

// ============================================================================
// 🎯 DEBUGGING Y UTILIDADES
// ============================================================================

#ifdef _DEBUG
void SetMPDisciplineDebugLogging(bool enabled)
{
    g_debugLogging = enabled;
}

void ForceGameContext(GameContext context)
{
    g_forcedContext = context;
    g_contextForced = (context != GameContext::Normal);
}

void ForcePressureLevel(PressureLevel level)
{
    g_forcedPressure = level;
    g_pressureForced = (level != PressureLevel::Low);
}

std::string GetMPDisciplineDebugInfo()
{
    std::ostringstream oss;
    oss << "MP Discipline Debug Info:\n";
    oss << "  Enabled: " << (g_mpState.enabled ? "Yes" : "No") << "\n";
    oss << "  Strategy: " << static_cast<int>(g_mpState.currentStrategy) << "\n";
    oss << "  Context: " << static_cast<int>(g_mpState.currentContext) << "\n";
    oss << "  Pressure: " << static_cast<int>(g_mpState.currentPressure) << "\n";
    oss << "  Active Monsters: " << g_mpState.metrics.activeMonsters << "\n";
    oss << "  Active Missiles: " << g_mpState.metrics.activeMissiles << "\n";
    oss << "  Frame Time: " << g_mpState.metrics.frameTime << "ms\n";
    oss << "  MP Rules Activations: " << g_mpState.mpRulesActivations << "\n";
    oss << "  Emergency Fallbacks: " << g_mpState.emergencyFallbacks << "\n";
    oss << "  Total Frames: " << g_mpStats.totalFramesProcessed;
    return oss.str();
}

void SimulateHighStressSituation()
{
    // Simular situación de alto estrés para testing
    g_mpState.metrics.activeMonsters = 150;
    g_mpState.metrics.activeMissiles = 300;
    g_mpState.metrics.activeEffects = 500;
    g_mpState.metrics.spawnRequests = 10;
    g_mpState.metrics.damageCalculations = 100;
    g_mpState.metrics.frameTime = 50;
    
    g_mpState.infernoActive = true;
    g_mpState.elitePackActive = true;
    
    std::cout << "High stress situation simulated for testing" << std::endl;
}
#endif

} // namespace devilution