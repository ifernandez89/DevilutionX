#pragma once

/**
 * @file apocalypse_crash_debug.h
 * 
 * APOCALYPSE CRASH DEBUGGING SYSTEM - LATENT MODE
 * 
 * Sistema de debugging específico para capturar información detallada
 * sobre crashes del spell Apocalypse. Genera reportes .txt con datos
 * completos del estado del engine en el momento del crash.
 * 
 * MODO LATENTE: El sistema permanece inactivo durante gameplay normal
 * y solo se activa cuando detecta condiciones peligrosas que podrían
 * causar crashes. Esto minimiza el impacto en performance.
 * 
 * UBICACIÓN DE REPORTES: debug_logs/apocalypse_crash_debug_*.txt
 * 
 * USO FUTURO: Este sistema puede extenderse fácilmente para debuggear
 * otros tipos de crashes modificando las condiciones de activación.
 */

#include <cstdint>
#include <string>
#include <fstream>

namespace devilution {

// ============================================================================
// 🔍 APOCALYPSE CRASH DEBUGGING
// ============================================================================

/**
 * Estructura para capturar el estado completo del engine
 * en el momento de un crash de Apocalypse
 */
struct ApocalypseCrashState {
    // Información temporal
    uint32_t timestamp;
    uint32_t frameNumber;
    
    // Estado del jugador
    int playerLevel;
    int playerX, playerY;
    int playerMana;
    int spellLevel;
    
    // Estado de missiles
    int totalMissiles;
    int apocalypseMissiles;
    int apocalypseBoomMissiles;
    int infernoMissiles;
    int otherMissiles;
    
    // Estado de monsters
    int totalMonsters;
    int activeMonsters;
    int monstersInRange;
    
    // Estado de luces
    int activeLights;
    int lightLoad;
    
    // Estado del nivel
    int currentLevel;
    int levelType;
    int dungeonSeed;
    
    // Información de memoria
    size_t missileListSize;
    size_t monsterListSize;
    size_t availableMemory;
    
    // Estado del MAL
    int malEstimatedLoad;
    bool malIsHealthy;
    bool malNeedsThrottling;
    bool malInCriticalState;
    
    // Información del crash
    std::string crashLocation;
    std::string lastFunction;
    std::string errorMessage;
};

// ============================================================================
// 🔧 DEBUGGING FUNCTIONS
// ============================================================================

/**
 * Inicializa el sistema de debugging de Apocalypse
 */
void InitApocalypseCrashDebug();

/**
 * Captura el estado actual del engine antes de procesar Apocalypse
 */
void CapturePreApocalypseState();

/**
 * Captura el estado durante el procesamiento de Apocalypse
 */
void CaptureApocalypseProcessingState(const char* location);

/**
 * Captura el estado en caso de crash y genera reporte
 */
void CaptureApocalypseCrashState(const char* crashLocation, const char* errorMsg);

/**
 * Genera un reporte detallado del crash
 */
void GenerateApocalypseCrashReport();

/**
 * Limpia el sistema de debugging
 */
void CleanupApocalypseCrashDebug();

// ============================================================================
// 🎯 MACROS DE DEBUGGING - SISTEMA LATENTE
// ============================================================================

// Sistema latente: Solo se activa cuando hay problemas reales
// Para uso futuro en otros crashes, simplemente cambiar las condiciones

#define APOCALYPSE_DEBUG_INIT() InitApocalypseCrashDebug()

#define APOCALYPSE_DEBUG_PRE_CAST() \
    CapturePreApocalypseState()

#define APOCALYPSE_DEBUG_PROCESSING(location) \
    CaptureApocalypseProcessingState(location)

#define APOCALYPSE_DEBUG_CRASH(location, error) \
    CaptureApocalypseCrashState(location, error)

#define APOCALYPSE_DEBUG_CLEANUP() CleanupApocalypseCrashDebug()

// ============================================================================
// 🔧 SISTEMA LATENTE PARA OTROS CRASHES FUTUROS
// ============================================================================

// Para activar debugging de otros spells/sistemas, usar estos macros:
#define GENERIC_CRASH_DEBUG_INIT(system) InitApocalypseCrashDebug()
#define GENERIC_CRASH_DEBUG_LOG(system, location) \
    do { \
        if (gEngineHealth.inCriticalState) { \
            CaptureApocalypseProcessingState(location); \
        } \
    } while(0)

// ============================================================================
// 🚨 CRASH DETECTION HELPERS
// ============================================================================

/**
 * Verifica si el estado actual es peligroso para Apocalypse
 */
bool IsApocalypseDangerousState();

/**
 * Registra una operación peligrosa
 */
void LogDangerousOperation(const char* operation);

/**
 * Verifica la integridad de la lista de missiles
 */
bool ValidateMissileListIntegrity();

/**
 * Verifica la integridad de los iteradores
 */
bool ValidateIteratorIntegrity();

} // namespace devilution