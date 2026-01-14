/**
 * @file mp_discipline.h
 * 
 * 🎯 FASE MP1 - MULTIPLAYER DISCIPLINE RESCUE 🎯
 * 
 * Sistema para rescatar la disciplina y estabilidad del código multiplayer
 * como fallback en situaciones de alto estrés del engine, sin afectar
 * las reglas base del single player.
 */

#pragma once

#include <cstdint>
#include <array>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/point.hpp"

namespace devilution {

// ============================================================================
// 🎯 ESTRATEGIAS DE BALANCE
// ============================================================================

/**
 * @brief Estrategias de balance disponibles
 */
enum class BalanceStrategy : uint8_t {
	Single = 0,     // Reglas originales de single player
	Multi,          // Reglas disciplinadas de multiplayer
	Hybrid,         // Combinación inteligente según contexto
	Auto            // Detección automática según estrés del engine
};

/**
 * @brief Contexto de juego para determinar estrategia
 */
enum class GameContext : uint8_t {
	Normal = 0,     // Juego normal, usar single rules
	Endgame,        // Niveles altos, considerar MP balance
	Inferno,        // Hechizo Inferno activo, usar MP spawn logic
	ElitePack,      // Pack de élites, usar MP balance
	HighStress,     // Engine bajo estrés, usar MP fallbacks
	Emergency       // Situación crítica, máxima disciplina MP
};

// ============================================================================
// 🎯 DETECCIÓN DE PRESIÓN DEL ENGINE
// ============================================================================

/**
 * @brief Métricas de presión del engine
 */
struct EnginePressureMetrics {
	uint32_t activeMonsters = 0;        // Monstruos activos
	uint32_t activeMissiles = 0;        // Misiles activos
	uint32_t activeEffects = 0;         // Efectos visuales activos
	uint32_t spawnRequests = 0;         // Requests de spawn este frame
	uint32_t damageCalculations = 0;    // Cálculos de daño este frame
	uint32_t frameTime = 0;             // Tiempo del frame actual (ms)
	
	// Límites de presión
	static constexpr uint32_t MAX_SAFE_MONSTERS = 80;
	static constexpr uint32_t MAX_SAFE_MISSILES = 150;
	static constexpr uint32_t MAX_SAFE_EFFECTS = 200;
	static constexpr uint32_t MAX_SAFE_SPAWNS_PER_FRAME = 5;
	static constexpr uint32_t MAX_SAFE_DAMAGE_CALCS = 50;
	static constexpr uint32_t MAX_SAFE_FRAME_TIME = 33; // ~30 FPS
};

/**
 * @brief Niveles de presión del engine
 */
enum class PressureLevel : uint8_t {
	Low = 0,        // Engine funcionando normalmente
	Medium,         // Presión moderada, considerar MP fallbacks
	High,           // Alta presión, usar MP discipline
	Critical        // Presión crítica, máxima disciplina MP
};

// ============================================================================
// 🎯 TABLAS DE BALANCE MULTIPLAYER
// ============================================================================

/**
 * @brief Configuración de balance rescatada del multiplayer
 */
struct MultiplayerBalance {
	// Modificadores de daño más conservadores
	float damageMultiplier = 0.85f;     // Reducir picos de daño
	float healthMultiplier = 1.15f;     // Aumentar vida para estabilidad
	float spellDamageMultiplier = 0.8f; // Spells más conservadores
	
	// Límites de spawn más disciplinados
	uint32_t maxSpawnsPerFrame = 3;     // Máximo spawns simultáneos
	uint32_t spawnCooldown = 100;       // Cooldown entre spawns (ms)
	uint32_t maxActiveMonsters = 60;    // Límite total de monstruos
	
	// Timers discretos (menos frame-dependent)
	uint32_t dotTickInterval = 1000;    // DOT cada 1 segundo
	uint32_t buffTickInterval = 500;    // Buffs cada 0.5 segundos
	uint32_t stateValidationInterval = 2000; // Validación cada 2 segundos
	
	// Factores de degradación
	float emergencyDamageReduction = 0.5f;  // Reducir daño en emergencia
	float emergencySpawnReduction = 0.3f;   // Reducir spawns en emergencia
};

// ============================================================================
// 🎯 ESTADO DEL SISTEMA
// ============================================================================

/**
 * @brief Estado global del sistema de disciplina MP
 */
struct MPDisciplineState {
	bool enabled = true;
	BalanceStrategy currentStrategy = BalanceStrategy::Auto;
	GameContext currentContext = GameContext::Normal;
	PressureLevel currentPressure = PressureLevel::Low;
	
	// Métricas actuales
	EnginePressureMetrics metrics;
	MultiplayerBalance balance;
	
	// Contadores de uso
	uint32_t mpRulesActivations = 0;    // Veces que se activaron reglas MP
	uint32_t emergencyFallbacks = 0;    // Veces que se usó fallback de emergencia
	uint32_t stateValidations = 0;      // Validaciones de estado realizadas
	
	// Timers
	uint32_t lastSpawnTime = 0;         // Último spawn realizado
	uint32_t lastDotTick = 0;           // Último tick de DOT
	uint32_t lastBuffTick = 0;          // Último tick de buffs
	uint32_t lastStateValidation = 0;   // Última validación de estado
	
	// Flags de estado
	bool infernoActive = false;         // Si Inferno está activo
	bool elitePackActive = false;       // Si hay pack de élites activo
	bool emergencyMode = false;         // Si está en modo de emergencia
};

/**
 * @brief Estadísticas del sistema para debugging
 */
struct MPDisciplineStats {
	uint64_t totalFramesProcessed = 0;
	uint64_t framesWithMPRules = 0;
	uint64_t totalEmergencyActivations = 0;
	uint64_t totalStateValidations = 0;
	uint32_t peakPressureLevel = 0;
	float averageFrameTime = 0.0f;
	
	// Estadísticas por contexto
	std::array<uint32_t, 6> contextActivations = {}; // Por GameContext
	std::array<uint32_t, 4> pressureLevelFrames = {}; // Por PressureLevel
};

// ============================================================================
// 🎯 API PRINCIPAL
// ============================================================================

/**
 * @brief Inicializar el sistema de disciplina MP
 */
void InitMPDiscipline();

/**
 * @brief Actualizar el sistema cada frame
 */
void UpdateMPDiscipline();

/**
 * @brief Detectar nivel de presión actual del engine
 */
PressureLevel DetectEnginePressure();

/**
 * @brief Determinar contexto de juego actual
 */
GameContext DetectGameContext();

/**
 * @brief Determinar estrategia de balance apropiada
 */
BalanceStrategy DetermineBalanceStrategy();

// ============================================================================
// 🎯 FUNCIONES DE RESCATE MP
// ============================================================================

/**
 * @brief Verificar si se debe usar spawn logic de MP
 */
bool ShouldUseMultiplayerSpawnLogic();

/**
 * @brief Verificar si se debe usar balance de MP
 */
bool ShouldUseMultiplayerBalance();

/**
 * @brief Verificar si se debe usar timers discretos de MP
 */
bool ShouldUseDiscreteTimers();

/**
 * @brief Aplicar modificadores de daño MP si es necesario
 */
float ApplyMultiplayerDamageModifier(float originalDamage, bool isSpell = false);

/**
 * @brief Aplicar modificadores de vida MP si es necesario
 */
int ApplyMultiplayerHealthModifier(int originalHealth);

/**
 * @brief Verificar si se puede realizar spawn (MP logic)
 */
bool CanSpawnMonster();

/**
 * @brief Registrar spawn realizado (MP tracking)
 */
void RegisterMonsterSpawn();

// ============================================================================
// 🎯 VALIDACIÓN DE ESTADO (RESCATADO DE MP)
// ============================================================================

/**
 * @brief Validar estado del juego (anti-corrupción)
 */
bool ValidateGameState();

/**
 * @brief Validar estado de un monstruo
 */
bool ValidateMonsterState(int monsterId);

/**
 * @brief Validar estado del player
 */
bool ValidatePlayerState();

/**
 * @brief Reparar estado corrupto si es posible
 */
bool RepairCorruptedState();

// ============================================================================
// 🎯 TIMERS DISCRETOS (RESCATADOS DE MP)
// ============================================================================

/**
 * @brief Verificar si es momento de tick de DOT
 */
bool ShouldTickDOT();

/**
 * @brief Verificar si es momento de tick de buffs
 */
bool ShouldTickBuffs();

/**
 * @brief Registrar tick de DOT realizado
 */
void RegisterDOTTick();

/**
 * @brief Registrar tick de buffs realizado
 */
void RegisterBuffTick();

// ============================================================================
// 🎯 CONFIGURACIÓN Y PRESETS
// ============================================================================

/**
 * @brief Aplicar preset conservador (máxima estabilidad)
 */
void ApplyConservativePreset();

/**
 * @brief Aplicar preset balanceado (estabilidad + performance)
 */
void ApplyBalancedPreset();

/**
 * @brief Aplicar preset agresivo (mínima intervención MP)
 */
void ApplyAggressivePreset();

/**
 * @brief Configurar estrategia específica
 */
void SetBalanceStrategy(BalanceStrategy strategy);

/**
 * @brief Obtener estado actual del sistema
 */
const MPDisciplineState &GetMPDisciplineState();

/**
 * @brief Obtener estadísticas del sistema
 */
const MPDisciplineStats &GetMPDisciplineStats();

// ============================================================================
// 🎯 DEBUGGING Y UTILIDADES
// ============================================================================

#ifdef _DEBUG
/**
 * @brief Habilitar/deshabilitar logging de debugging
 */
void SetMPDisciplineDebugLogging(bool enabled);

/**
 * @brief Forzar contexto de juego para testing
 */
void ForceGameContext(GameContext context);

/**
 * @brief Forzar nivel de presión para testing
 */
void ForcePressureLevel(PressureLevel level);

/**
 * @brief Obtener información de debugging como string
 */
std::string GetMPDisciplineDebugInfo();

/**
 * @brief Simular situación de alto estrés para testing
 */
void SimulateHighStressSituation();
#endif

} // namespace devilution