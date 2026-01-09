#pragma once

/**
 * @file safety_metrics.h
 * 
 * SAFETY LAYER - Phase 5: Advanced Stress Testing Metrics
 * 
 * Sistema de monitoreo avanzado para validación de stress testing.
 * - Métricas en tiempo real del Safety Layer
 * - Logging específico para stress testing
 * - Validación automatizada de criterios de éxito
 * - Dashboard de estado del engine
 */

#include <cstdint>
#include <string>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// 🔬 SISTEMA DE MÉTRICAS AVANZADO
// ============================================================================

/**
 * Estructura para métricas del Safety Layer en tiempo real
 */
struct SafetyMetrics {
    // Contadores en tiempo real
    size_t currentMissiles = 0;
    size_t currentFloatingNumbers = 0;
    size_t currentActiveMonsters = 0;
    
    // Contadores de eventos del Safety Layer
    size_t safetyChecksTriggered = 0;
    size_t spawnsBlocked = 0;
    size_t gracefulDegradations = 0;
    
    // Performance metrics
    uint32_t averageFrameTime = 0;
    uint32_t peakFrameTime = 0;
    bool isInDangerZone = false;
    
    // Stress test specific
    uint32_t testStartTime = 0;
    uint32_t testDuration = 0;
    bool testActive = false;
    std::string currentTest = "";
    
    // Límites y umbrales
    static constexpr size_t MAX_SAFE_MISSILES = 500;
    static constexpr size_t MAX_SAFE_FLOATING_NUMBERS = 8;
    static constexpr size_t MAX_SAFE_MONSTERS = 190; // 200 - 10 buffer
    static constexpr uint32_t DANGER_ZONE_THRESHOLD = 80; // 80% de uso
};

/**
 * Categorías de eventos para logging de stress testing
 */
enum class StressEventCategory {
    INFERNO,
    CHAIN_LIGHTNING,
    MULTIMISSILE,
    TRAP,
    SAFETY_LAYER,
    PERFORMANCE,
    MEMORY
};

// ============================================================================
// 🧪 API DE STRESS TESTING
// ============================================================================

/**
 * Obtiene las métricas actuales del Safety Layer
 */
SafetyMetrics& GetSafetyMetrics();

/**
 * Actualiza las métricas con los valores actuales del juego
 */
void UpdateSafetyMetrics();

/**
 * Inicia un test de stress con nombre específico
 */
void StartStressTest(const std::string& testName);

/**
 * Finaliza el test de stress actual
 */
bool EndStressTest();

/**
 * Verifica si las métricas actuales pasan los criterios de éxito
 */
bool ValidateStressMetrics();

/**
 * Registra un evento de stress testing
 */
void LogStressEvent(StressEventCategory category, const std::string& message);

/**
 * Obtiene un reporte completo del estado actual
 */
std::string GetSafetyStatusReport();

// ============================================================================
// 🎯 VALIDADORES ESPECÍFICOS
// ============================================================================

/**
 * Valida métricas específicas para test de Inferno
 */
inline bool ValidateInfernoMetrics() {
    const SafetyMetrics& metrics = GetSafetyMetrics();
    return metrics.currentMissiles <= SafetyMetrics::MAX_SAFE_MISSILES
        && metrics.currentFloatingNumbers <= SafetyMetrics::MAX_SAFE_FLOATING_NUMBERS;
}

/**
 * Valida métricas específicas para test de Chain Lightning
 */
inline bool ValidateChainMetrics() {
    const SafetyMetrics& metrics = GetSafetyMetrics();
    return metrics.currentMissiles <= SafetyMetrics::MAX_SAFE_MISSILES
        && metrics.spawnsBlocked < metrics.safetyChecksTriggered; // Algunos spawns deben pasar
}

/**
 * Valida métricas específicas para test de Multimissiles
 */
inline bool ValidateMultimissileMetrics() {
    const SafetyMetrics& metrics = GetSafetyMetrics();
    return metrics.currentMissiles <= SafetyMetrics::MAX_SAFE_MISSILES
        && metrics.gracefulDegradations > 0; // Debe haber degradación bajo presión
}

/**
 * Valida métricas críticas para test combinado
 */
inline bool ValidateCriticalMetrics() {
    const SafetyMetrics& metrics = GetSafetyMetrics();
    return metrics.currentMissiles <= SafetyMetrics::MAX_SAFE_MISSILES
        && metrics.currentFloatingNumbers <= SafetyMetrics::MAX_SAFE_FLOATING_NUMBERS
        && metrics.currentActiveMonsters <= SafetyMetrics::MAX_SAFE_MONSTERS
        && metrics.peakFrameTime < 1000; // Máximo 1 segundo por frame
}

// ============================================================================
// 🔍 MACROS DE LOGGING PARA STRESS TESTING
// ============================================================================

#ifdef _DEBUG
/**
 * Macro para logging de eventos de stress testing (solo en debug)
 */
#define STRESS_LOG(category, message) \
    do { \
        LogStressEvent(StressEventCategory::category, message); \
    } while(0)

/**
 * Macro para logging con formato (solo en debug)
 */
#define STRESS_LOG_FMT(category, format, ...) \
    do { \
        char buffer[256]; \
        snprintf(buffer, sizeof(buffer), format, __VA_ARGS__); \
        LogStressEvent(StressEventCategory::category, std::string(buffer)); \
    } while(0)

/**
 * Macro para verificación de métricas con logging automático
 */
#define STRESS_VALIDATE_OR_FAIL(validator, testName) \
    do { \
        if (!validator()) { \
            STRESS_LOG(SAFETY_LAYER, "VALIDATION FAILED: " testName); \
            return false; \
        } \
    } while(0)

#else
// En release, las macros de stress testing no hacen nada
#define STRESS_LOG(category, message) 
#define STRESS_LOG_FMT(category, format, ...)
#define STRESS_VALIDATE_OR_FAIL(validator, testName) \
    do { if (!validator()) return false; } while(0)
#endif

// ============================================================================
// 🏆 CERTIFICACIÓN DE ENGINE
// ============================================================================

/**
 * Niveles de certificación del engine
 */
enum class EngineCertificationLevel {
    UNSTABLE,    // Falla tests básicos
    STABLE,      // Pasa tests individuales
    ROBUST,      // Pasa tests combinados
    BULLETPROOF  // Pasa todos los tests incluyendo "Combined Chaos"
};

/**
 * Obtiene el nivel de certificación actual del engine
 */
EngineCertificationLevel GetEngineCertificationLevel();

/**
 * Obtiene descripción textual del nivel de certificación
 */
const char* GetCertificationLevelName(EngineCertificationLevel level);

/**
 * Verifica si el engine está certificado para optimizaciones agresivas
 */
inline bool IsReadyForAggressiveOptimizations() {
    return GetEngineCertificationLevel() >= EngineCertificationLevel::BULLETPROOF;
}

} // namespace devilution