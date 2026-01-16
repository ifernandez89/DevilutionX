#pragma once

/**
 * @file crash_diagnostics.h
 * 
 * 🔍 CRASH DIAGNOSTICS SYSTEM
 * 
 * OBJETIVO:
 * Identificar la causa exacta del crash persistente mediante
 * logging detallado de todos los factores relevantes.
 * 
 * FACTORES A MONITOREAR:
 * - Tipo y cantidad de enemigos (Spitters, etc.)
 * - Nivel actual (Level 9, etc.)
 * - Missiles activos por tipo
 * - Spam de clicks y frecuencia
 * - Estado del engine (memoria, arrays, etc.)
 * - Combinaciones específicas que causan crash
 * 
 * PRINCIPIOS:
 * 1. LOGGING DETALLADO - Capturar todo antes del crash
 * 2. IDENTIFICACIÓN PRECISA - Encontrar el patrón exacto
 * 3. ANÁLISIS TEMPORAL - Secuencia de eventos
 * 4. FACTORES MÚLTIPLES - No solo clicks, sino contexto
 * 5. DOCUMENTACIÓN COMPLETA - Para corrección definitiva
 */

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "engine/point.hpp"
#include "missiles.h"
#include "monster.h"

namespace devilution {

// ============================================================================
// 🔍 CRASH DIAGNOSTICS CONSTANTS
// ============================================================================

namespace CrashDiagnostics {
    constexpr int MAX_LOG_ENTRIES = 1000;           // Máximo entradas de log
    constexpr int SNAPSHOT_INTERVAL_MS = 50;        // REDUCIDO: Snapshot cada 50ms (era 100ms)
    constexpr int CRITICAL_MISSILE_COUNT = 10;      // REDUCIDO: Umbral crítico de missiles (era 15)
    constexpr int CRITICAL_MONSTER_COUNT = 150;     // INCREASED: Real crash threshold (was 15, crashes at 200)
    constexpr int CLICK_SPAM_THRESHOLD = 3;         // REDUCIDO: Clicks por segundo para spam (era 5)
    constexpr int INFERNO_CLICK_THRESHOLD = 2;      // NUEVO: Clicks de Inferno por segundo
}

// ============================================================================
// 🔍 DIAGNOSTIC DATA STRUCTURES
// ============================================================================

struct MonsterSnapshot {
    int totalMonsters = 0;
    int spitters = 0;
    int skeletons = 0;
    int zombies = 0;
    int goatmen = 0;
    int other = 0;
    int attacking = 0;
    int moving = 0;
    int idle = 0;
};

struct MissileSnapshot {
    int totalMissiles = 0;
    int infernoControls = 0;
    int infernos = 0;
    int fireballs = 0;
    int lightningControls = 0;
    int chainLightnings = 0;
    int spitterProjectiles = 0;
    int other = 0;
};

struct GameStateSnapshot {
    uint32_t timestamp = 0;
    int currentLevel = 0;
    Point playerPosition = {0, 0};
    int playerHealth = 0;
    int playerMana = 0;
    
    MonsterSnapshot monsters;
    MissileSnapshot missiles;
    
    // Click tracking
    int clicksInLastSecond = 0;
    int totalClicksThisSession = 0;
    uint32_t lastClickTime = 0;
    
    // Engine state
    int memoryUsage = 0;
    int activeObjects = 0;
    int activeItems = 0;
    
    // Throttling state
    bool globalThrottlingActive = false;
    int throttledCasts = 0;
    int preventedCasts = 0;
};

struct CrashEvent {
    uint32_t timestamp = 0;
    std::string eventType;
    std::string description;
    GameStateSnapshot stateBefore;
    std::string additionalInfo;
};

// ============================================================================
// 🔍 CRASH DIAGNOSTICS STATE
// ============================================================================

struct CrashDiagnosticsState {
    bool enabled = true;
    bool verboseLogging = false;
    
    std::vector<GameStateSnapshot> snapshots;
    std::vector<CrashEvent> events;
    
    uint32_t lastSnapshotTime = 0;
    uint32_t sessionStartTime = 0;
    
    // Click tracking
    std::vector<uint32_t> recentClicks;
    std::vector<uint32_t> recentInfernoClicks;  // NUEVO: Track específico de clicks de Inferno
    int totalClicks = 0;
    int totalInfernoClicks = 0;  // NUEVO: Contador total de clicks de Inferno
    
    // Pattern detection
    bool criticalStateDetected = false;
    std::string lastCriticalPattern;
    
    // Statistics
    int totalSnapshots = 0;
    int totalEvents = 0;
    int maxMissilesRecorded = 0;
    int maxMonstersRecorded = 0;
};

// ============================================================================
// 🔍 CRASH DIAGNOSTICS API
// ============================================================================

/**
 * Inicializa el sistema de diagnósticos de crash
 */
void InitCrashDiagnostics();

/**
 * Actualiza el sistema por frame
 * Captura snapshots periódicos del estado del juego
 */
void UpdateCrashDiagnostics();

/**
 * Registra un click del jugador para tracking de spam
 */
void RegisterPlayerClick();

/**
 * Registra específicamente un click de Inferno para tracking especializado
 */
void RegisterInfernoClick();

/**
 * Cuenta cuántos InfernoControls están activos actualmente
 */
int CountActiveInfernoControls();

/**
 * Registra un evento específico para análisis
 */
void RegisterCrashEvent(const std::string& eventType, const std::string& description, const std::string& additionalInfo = "");

/**
 * Captura un snapshot completo del estado actual
 */
GameStateSnapshot CaptureGameStateSnapshot();

/**
 * Analiza patrones que pueden llevar a crash
 */
void AnalyzeCrashPatterns();

/**
 * Detecta si estamos en un estado crítico pre-crash
 */
bool IsInCriticalState();

/**
 * Genera un reporte completo de diagnósticos
 */
std::string GenerateDiagnosticsReport();

/**
 * Guarda diagnósticos a archivo antes de posible crash
 */
void SaveDiagnosticsToFile();

/**
 * Obtiene el estado actual del sistema de diagnósticos
 */
const CrashDiagnosticsState& GetCrashDiagnosticsState();

/**
 * Habilita/deshabilita logging verbose
 */
void SetVerboseLogging(bool enabled);

// ============================================================================
// 🔍 MACROS DE CONVENIENCIA
// ============================================================================

/**
 * Macro para registrar eventos críticos
 */
#define LOG_CRASH_EVENT(type, desc) \
    RegisterCrashEvent(type, desc, __FILE__ ":" + std::to_string(__LINE__))

/**
 * Macro para registrar eventos con información adicional
 */
#define LOG_CRASH_EVENT_INFO(type, desc, info) \
    RegisterCrashEvent(type, desc, info)

/**
 * Macro para verificar estado crítico - DISABLED after successful crash fix
 */
#define CHECK_CRITICAL_STATE() do { } while(0)

} // namespace devilution