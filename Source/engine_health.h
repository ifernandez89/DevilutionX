#pragma once

/**
 * @file engine_health.h
 * 
 * MODERN ASSIST LAYER (MAL) - Engine Health Monitoring
 * 
 * FILOSOFÍA: Diablo 96 intacto, observabilidad 2026
 * 
 * Este sistema NO cambia gameplay, solo observa y mide el estado del engine
 * para permitir decisiones inteligentes y prevenir crashes.
 * 
 * PRINCIPIOS:
 * - No afecta lógica de juego
 * - Solo lectura y medición
 * - Throttling adaptativo, no límites fijos
 * - Feedback visual mínimo
 * - Completamente reversible
 */

#include <cstdint>

namespace devilution {

// ============================================================================
// 🔍 ENGINE HEALTH SNAPSHOT
// ============================================================================

/**
 * Snapshot del estado de salud del engine en tiempo real
 * Actualizado cada frame, solo lectura para sistemas externos
 */
struct EngineHealth {
    // Contadores básicos
    int activeMissiles = 0;        // Missiles activos en Missiles list
    int activeMonsters = 0;        // Monsters vivos (nummonsters)
    int activeApocalypse = 0;      // Apocalypse missiles activos
    int activeInfernos = 0;        // Inferno missiles activos
    int activeLights = 0;          // Luces activas (ActiveLightCount)
    
    // Métricas de carga
    int estimatedLoad = 0;         // Carga estimada del engine (0-100+)
    int frameCostEstimate = 0;     // Costo estimado del frame actual
    
    // Contadores de stress
    int apocalypseSpawnsThisFrame = 0;  // ApocalypseBoom spawns en frame actual
    int totalSpellsActive = 0;          // Total de spells activos
    
    // Estado de salud general
    bool isHealthy = true;         // Engine en estado saludable
    bool needsThrottling = false;  // Engine necesita throttling
    bool inCriticalState = false;  // Engine en estado crítico
};

/**
 * Estado global de salud del engine
 * Actualizado por UpdateEngineHealth() cada frame
 */
extern EngineHealth gEngineHealth;

// ============================================================================
// 🚦 ENGINE LOAD STATES
// ============================================================================

/**
 * Estados de carga del engine para decisiones adaptativas
 */
enum class EngineLoadState {
    OK,        // Engine funcionando normalmente
    HIGH,      // Engine bajo carga alta pero manejable
    CRITICAL   // Engine en estado crítico, necesita throttling
};

// ============================================================================
// 🔧 CORE FUNCTIONS
// ============================================================================

/**
 * Actualiza el snapshot de salud del engine
 * Debe llamarse una vez por frame, preferiblemente al inicio
 */
void UpdateEngineHealth();

/**
 * Obtiene el estado actual de carga del engine
 * Usado para decisiones de throttling adaptativo
 */
EngineLoadState GetEngineLoadState();

/**
 * Verifica si es seguro crear un nuevo missile
 * Considera carga actual y tipo de missile
 */
bool CanSafelyAddMissile(int missileType = -1);

/**
 * Verifica si es seguro castear Apocalypse
 * Considera missiles activos, carga del engine y estado general
 * @param playerId ID del jugador que intenta castear (para límite por jugador)
 */
bool CanSafelyCastApocalypse(int playerId);

/**
 * Limpia los cooldowns de Apocalypse
 * Debe llamarse al iniciar una nueva partida
 */
void ResetApocalypseCooldowns();

/**
 * Verifica si es seguro castear Inferno
 * Considera Inferno missiles activos y carga general
 */
bool CanSafelyCastInferno();

// ============================================================================
// 🎮 FEEDBACK FUNCTIONS
// ============================================================================

/**
 * Reproduce feedback visual/audio mínimo cuando un spell no puede castearse
 * No intrusivo, mantiene inmersión
 */
void PlaySpellThrottleFeedback();

/**
 * Reproduce feedback cuando el engine está bajo carga alta
 * Sutil, no rompe inmersión
 */
void PlayEngineStressFeedback();

// ============================================================================
// 🐛 DEBUG FUNCTIONS (solo en DEBUG builds)
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime estado actual del engine health
 * Solo disponible en builds de debug
 */
void LogEngineHealth();

/**
 * Imprime estadísticas detalladas del engine
 * Para debugging avanzado
 */
void LogEngineHealthDetailed();
#endif

// ============================================================================
// 🔄 INTEGRATION HELPERS
// ============================================================================

/**
 * Inicializa el sistema de engine health
 * Debe llamarse una vez al inicio del juego
 */
void InitEngineHealth();

/**
 * Limpia el sistema de engine health
 * Debe llamarse al cerrar el juego
 */
void CleanupEngineHealth();

/**
 * Resetea contadores de frame
 * Debe llamarse al inicio de cada frame
 */
void ResetFrameCounters();

} // namespace devilution