#pragma once

/**
 * @file architectural_analysis.h
 * 
 * SISTEMA PERMANENTE DE ANÁLISIS ARQUITECTÓNICO
 * 
 * PROPÓSITO: Entender cómo convive nuestro diseño con el original
 * OBJETIVO: Detectar interferencias y sobre-ingeniería
 * FILOSOFÍA: Logging permanente para rediseñar, no parchear
 */

#include <fstream>
#include <chrono>
#include <iomanip>
#include <string>

namespace devilution {

/**
 * Sistema de análisis arquitectónico permanente
 * Siempre activo, logging mínimo pero crítico
 */
class ArchitecturalAnalyzer {
private:
    static ArchitecturalAnalyzer* instance;
    std::string logPath;
    bool isActive;
    
    // Contadores para detectar patrones
    int apocalypseCasts = 0;
    int missileCreations = 0;
    int missileFailures = 0;
    int processApocalypseCalls = 0;
    int boomCreations = 0;
    
    // Timestamps para detectar bucles
    std::chrono::steady_clock::time_point lastApocalypseCast;
    std::chrono::steady_clock::time_point lastMissileCreation;
    
public:
    static ArchitecturalAnalyzer& getInstance();
    
    // Inicialización del sistema
    void initialize();
    void shutdown();
    
    // Logging de eventos críticos
    void logApocalypseCast(int playerId, int spellLevel, int currentMissiles);
    void logMissileCreation(const std::string& missileType, bool success, int totalMissiles);
    void logProcessApocalypse(int var2, int var3, int var4, int var5, int currentMissiles);
    void logApocalypseBoomCreation(int x, int y, int totalMissiles);
    void logCrashPrevention(const std::string& reason, const std::string& location);
    void logArchitecturalConflict(const std::string& ourSystem, const std::string& originalSystem, const std::string& conflict);
    
    // Análisis de patrones
    void analyzePatterns();
    void generateArchitecturalReport();
    
private:
    void writeLog(const std::string& message);
    std::string getCurrentTimestamp();
    bool isRapidFire(); // Detecta si hay demasiadas llamadas muy rápidas
};

// Macros para logging fácil
#define ARCH_LOG_APOCALYPSE_CAST(playerId, level, missiles) \
    ArchitecturalAnalyzer::getInstance().logApocalypseCast(playerId, level, missiles)

#define ARCH_LOG_MISSILE_CREATION(type, success, total) \
    ArchitecturalAnalyzer::getInstance().logMissileCreation(type, success, total)

#define ARCH_LOG_PROCESS_APOCALYPSE(v2, v3, v4, v5, missiles) \
    ArchitecturalAnalyzer::getInstance().logProcessApocalypse(v2, v3, v4, v5, missiles)

#define ARCH_LOG_BOOM_CREATION(x, y, total) \
    ArchitecturalAnalyzer::getInstance().logApocalypseBoomCreation(x, y, total)

#define ARCH_LOG_CRASH_PREVENTION(reason, location) \
    ArchitecturalAnalyzer::getInstance().logCrashPrevention(reason, location)

#define ARCH_LOG_CONFLICT(ourSys, origSys, conflict) \
    ArchitecturalAnalyzer::getInstance().logArchitecturalConflict(ourSys, origSys, conflict)

} // namespace devilution