/**
 * @file safety_metrics.cpp
 * 
 * SAFETY LAYER - Phase 5: Advanced Stress Testing Metrics Implementation
 * 
 * Implementación del sistema de monitoreo avanzado para stress testing.
 */

#include "safety_metrics.h"
#include "safety_limits.h"

#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

#ifdef _DEBUG
#include <fstream>
#include <iomanip>
#endif

namespace devilution {

// ============================================================================
// 🔬 ESTADO GLOBAL DE MÉTRICAS
// ============================================================================

namespace {
    SafetyMetrics g_safetyMetrics;
    std::vector<std::string> g_stressLog;
    EngineCertificationLevel g_certificationLevel = EngineCertificationLevel::UNSTABLE;
    
    // Frame time tracking - moved from function scope for proper reset
    uint32_t g_lastFrameTime = 0;
    std::vector<uint32_t> g_frameTimes;
}

// Reset function for new game
void ResetSafetyMetrics()
{
    g_safetyMetrics = SafetyMetrics();
    g_stressLog.clear();
    g_certificationLevel = EngineCertificationLevel::UNSTABLE;
    g_lastFrameTime = 0;
    g_frameTimes.clear();
}

// ============================================================================
// 🧪 IMPLEMENTACIÓN DE API DE STRESS TESTING
// ============================================================================

SafetyMetrics& GetSafetyMetrics() {
    return g_safetyMetrics;
}

void UpdateSafetyMetrics() {
    SafetyMetrics& metrics = g_safetyMetrics;
    
    // Actualizar contadores en tiempo real
    metrics.currentMissiles = Missiles.size();
    metrics.currentActiveMonsters = ActiveMonsterCount;
    // metrics.currentFloatingNumbers se actualiza desde floatingnumbers.cpp
    
    // Calcular si estamos en zona de peligro
    int missileUsage = GetMissileUsagePercent();
    int monsterUsage = GetMonsterUsagePercent();
    constexpr int DANGER_ZONE_THRESHOLD = 80;
    metrics.isInDangerZone = (missileUsage > DANGER_ZONE_THRESHOLD) || 
                            (monsterUsage > DANGER_ZONE_THRESHOLD);
    
    // Actualizar duración del test si está activo
    if (metrics.testActive) {
        metrics.testDuration = SDL_GetTicks() - metrics.testStartTime;
    }
    
    // Actualizar métricas de performance
    uint32_t currentTime = SDL_GetTicks();
    uint32_t frameTime = currentTime - g_lastFrameTime;
    g_lastFrameTime = currentTime;
    
    // Calcular promedio móvil simple para frame time
    g_frameTimes.push_back(frameTime);
    if (g_frameTimes.size() > 60) { // Mantener últimos 60 frames
        g_frameTimes.erase(g_frameTimes.begin());
    }
    
    uint32_t totalFrameTime = 0;
    for (uint32_t ft : g_frameTimes) {
        totalFrameTime += ft;
    }
    metrics.averageFrameTime = g_frameTimes.empty() ? 0 : totalFrameTime / g_frameTimes.size();
    metrics.peakFrameTime = std::max(metrics.peakFrameTime, frameTime);
}

void StartStressTest(const std::string& testName) {
    SafetyMetrics& metrics = g_safetyMetrics;
    
    // Reset métricas para nuevo test
    metrics.safetyChecksTriggered = 0;
    metrics.spawnsBlocked = 0;
    metrics.gracefulDegradations = 0;
    metrics.peakFrameTime = 0;
    
    // Configurar test
    metrics.testActive = true;
    metrics.currentTest = testName;
    metrics.testStartTime = SDL_GetTicks();
    metrics.testDuration = 0;
    
    // Log inicio del test
    STRESS_LOG(SAFETY_LAYER, "=== STRESS TEST STARTED: " + testName + " ===");
    
    // Limpiar log anterior
    g_stressLog.clear();
}

bool EndStressTest() {
    SafetyMetrics& metrics = g_safetyMetrics;
    
    if (!metrics.testActive) {
        return false; // No hay test activo
    }
    
    // Finalizar test
    metrics.testActive = false;
    uint32_t finalDuration = SDL_GetTicks() - metrics.testStartTime;
    
    // Validar métricas finales
    bool testPassed = ValidateStressMetrics();
    
    // Log resultado del test
    STRESS_LOG(SAFETY_LAYER, "=== STRESS TEST ENDED: " + metrics.currentTest + " ===");
    STRESS_LOG_FMT(SAFETY_LAYER, "Duration: %u ms", finalDuration);
    STRESS_LOG_FMT(SAFETY_LAYER, "Result: %s", testPassed ? "PASSED" : "FAILED");
    STRESS_LOG_FMT(SAFETY_LAYER, "Safety checks: %zu", metrics.safetyChecksTriggered);
    STRESS_LOG_FMT(SAFETY_LAYER, "Spawns blocked: %zu", metrics.spawnsBlocked);
    STRESS_LOG_FMT(SAFETY_LAYER, "Peak frame time: %u ms", metrics.peakFrameTime);
    
    // Actualizar nivel de certificación basado en el test
    UpdateCertificationLevel(metrics.currentTest, testPassed);
    
    return testPassed;
}

bool ValidateStressMetrics() {
    const SafetyMetrics& metrics = g_safetyMetrics;
    
    // Criterios básicos de validación
    bool basicCriteria = 
        metrics.currentMissiles <= SafetyMetrics::MAX_SAFE_MISSILES &&
        metrics.currentFloatingNumbers <= SafetyMetrics::MAX_SAFE_FLOATING_NUMBERS &&
        metrics.currentActiveMonsters <= SafetyMetrics::MAX_SAFE_MONSTERS;
    
    // Criterios de performance
    bool performanceCriteria = 
        metrics.peakFrameTime < 2000 && // Máximo 2 segundos por frame
        metrics.averageFrameTime < 100;  // Promedio menor a 100ms
    
    // Criterios de Safety Layer
    bool safetyCriteria = 
        metrics.safetyChecksTriggered > 0; // Debe haber actividad del Safety Layer
    
    return basicCriteria && performanceCriteria && safetyCriteria;
}

void LogStressEvent(StressEventCategory category, const std::string& message) {
#ifdef _DEBUG
    // Crear timestamp
    uint32_t timestamp = SDL_GetTicks();
    
    // Crear entrada de log
    std::ostringstream logEntry;
    logEntry << "[" << std::setw(8) << timestamp << "] ";
    
    // Agregar categoría
    switch (category) {
        case StressEventCategory::INFERNO: logEntry << "[INFERNO] "; break;
        case StressEventCategory::CHAIN_LIGHTNING: logEntry << "[CHAIN] "; break;
        case StressEventCategory::MULTIMISSILE: logEntry << "[MULTI] "; break;
        case StressEventCategory::TRAP: logEntry << "[TRAP] "; break;
        case StressEventCategory::SAFETY_LAYER: logEntry << "[SAFETY] "; break;
        case StressEventCategory::PERFORMANCE: logEntry << "[PERF] "; break;
        case StressEventCategory::MEMORY: logEntry << "[MEM] "; break;
    }
    
    logEntry << message;
    
    // Agregar al log en memoria
    g_stressLog.push_back(logEntry.str());
    
    // Mantener solo las últimas 1000 entradas
    if (g_stressLog.size() > 1000) {
        g_stressLog.erase(g_stressLog.begin());
    }
    
    // Output a consola en debug
    std::cout << logEntry.str() << std::endl;
    
    // Escribir a archivo si hay test activo
    const SafetyMetrics& metrics = g_safetyMetrics;
    if (metrics.testActive) {
        std::ofstream logFile("stress_test_" + metrics.currentTest + ".log", std::ios::app);
        if (logFile.is_open()) {
            logFile << logEntry.str() << std::endl;
        }
    }
#endif
}

std::string GetSafetyStatusReport() {
    const SafetyMetrics& metrics = g_safetyMetrics;
    
    std::ostringstream report;
    report << "=== SAFETY LAYER STATUS REPORT ===\n";
    report << "Current Missiles: " << metrics.currentMissiles << "/" << SafetyMetrics::MAX_SAFE_MISSILES << "\n";
    report << "Current Floating Numbers: " << metrics.currentFloatingNumbers << "/" << SafetyMetrics::MAX_SAFE_FLOATING_NUMBERS << "\n";
    report << "Current Active Monsters: " << metrics.currentActiveMonsters << "/" << SafetyMetrics::MAX_SAFE_MONSTERS << "\n";
    report << "In Danger Zone: " << (metrics.isInDangerZone ? "YES" : "NO") << "\n";
    report << "Average Frame Time: " << metrics.averageFrameTime << "ms\n";
    report << "Peak Frame Time: " << metrics.peakFrameTime << "ms\n";
    report << "Safety Checks Triggered: " << metrics.safetyChecksTriggered << "\n";
    report << "Spawns Blocked: " << metrics.spawnsBlocked << "\n";
    report << "Graceful Degradations: " << metrics.gracefulDegradations << "\n";
    
    if (metrics.testActive) {
        report << "Active Test: " << metrics.currentTest << "\n";
        report << "Test Duration: " << metrics.testDuration << "ms\n";
    }
    
    report << "Certification Level: " << GetCertificationLevelName(g_certificationLevel) << "\n";
    report << "Ready for Aggressive Optimizations: " << (IsReadyForAggressiveOptimizations() ? "YES" : "NO") << "\n";
    
    return report.str();
}

// ============================================================================
// 🏆 SISTEMA DE CERTIFICACIÓN
// ============================================================================

void UpdateCertificationLevel(const std::string& testName, bool passed) {
    if (!passed) {
        // Si falla cualquier test, bajar certificación
        g_certificationLevel = EngineCertificationLevel::UNSTABLE;
        return;
    }
    
    // Actualizar certificación basado en tests pasados
    if (testName == "INFERNO_ESTACIONARIO" || 
        testName == "CHAIN_LIGHTNING_STORM" || 
        testName == "MULTIMISSILE_BARRAGE" || 
        testName == "TRAP_GAUNTLET") {
        // Tests individuales -> STABLE
        if (g_certificationLevel < EngineCertificationLevel::STABLE) {
            g_certificationLevel = EngineCertificationLevel::STABLE;
        }
    } else if (testName.find("COMBINED") != std::string::npos) {
        // Tests combinados -> ROBUST
        if (g_certificationLevel < EngineCertificationLevel::ROBUST) {
            g_certificationLevel = EngineCertificationLevel::ROBUST;
        }
    } else if (testName == "COMBINED_CHAOS") {
        // Test definitivo -> BULLETPROOF
        g_certificationLevel = EngineCertificationLevel::BULLETPROOF;
    }
}

EngineCertificationLevel GetEngineCertificationLevel() {
    return g_certificationLevel;
}

const char* GetCertificationLevelName(EngineCertificationLevel level) {
    switch (level) {
        case EngineCertificationLevel::UNSTABLE: return "UNSTABLE";
        case EngineCertificationLevel::STABLE: return "STABLE";
        case EngineCertificationLevel::ROBUST: return "ROBUST";
        case EngineCertificationLevel::BULLETPROOF: return "BULLETPROOF";
        default: return "UNKNOWN";
    }
}

// ============================================================================
// 🎯 HELPERS PARA INTEGRACIÓN CON SAFETY LAYER
// ============================================================================

void RecordSafetyCheck() {
    g_safetyMetrics.safetyChecksTriggered++;
}

void RecordSpawnBlocked() {
    g_safetyMetrics.spawnsBlocked++;
}

void RecordGracefulDegradation() {
    g_safetyMetrics.gracefulDegradations++;
}

} // namespace devilution