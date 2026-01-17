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
    
    // 🚨 NUEVOS LOGS PARA SISTEMAS COLATERALES
    void logBoomProcess(int x, int y, int duration, int var1);
    void logCollision(int missileType, int monsterId, int damage, int hp);
    void logMonsterDeath(int monsterId, int monsterType, int killCount);
    void logAudio(int monsterId, int soundType, int volume, int pan);
    
    // 🎯 APOCALYPSE CRASH FIX: Deferred Loot Logging
    void logDeferredLoot(const std::string& action, int monsterCount = 0);
    
    // 🔍 GAME LOOP DEBUG: Track which system is running
    void logGameLoop(const std::string& system);
    
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

// 🚨 NUEVOS LOGS PARA SISTEMAS COLATERALES
#define ARCH_LOG_BOOM_PROCESS(x, y, duration, var1) \
    ArchitecturalAnalyzer::getInstance().logBoomProcess(x, y, duration, var1)

#define ARCH_LOG_COLLISION(missileType, monsterId, damage, hp) \
    ArchitecturalAnalyzer::getInstance().logCollision(missileType, monsterId, damage, hp)

#define ARCH_LOG_MONSTER_DEATH(monsterId, monsterType, killCount) \
    ArchitecturalAnalyzer::getInstance().logMonsterDeath(monsterId, monsterType, killCount)

#define ARCH_LOG_AUDIO(monsterId, soundType, volume, pan) \
    ArchitecturalAnalyzer::getInstance().logAudio(monsterId, soundType, volume, pan)

// 🎯 APOCALYPSE CRASH FIX: Deferred Loot Logging
#define ARCH_LOG_DEFERRED_LOOT(action, count) \
    ArchitecturalAnalyzer::getInstance().logDeferredLoot(action, count)

// 🔍 GAME LOOP DEBUG: Track which system is running
#define ARCH_LOG_GAME_LOOP(system) \
    ArchitecturalAnalyzer::getInstance().logGameLoop(system)

} // namespace devilution