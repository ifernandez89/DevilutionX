/**
 * @file crash_hunter.h
 * @brief CRASH HUNTER - Sistema de logging agresivo para encontrar al culpable
 * 
 * Este sistema loguea TODOS los eventos críticos relacionados con dungeons:
 * - Generación de niveles
 * - Spawn de monstruos
 * - Spawn de items
 * - Carga de saves
 * - Cambios de nivel
 * - Coordenadas del jugador
 * 
 * OBJETIVO: Encontrar exactamente QUÉ operación causa el crash
 */

#pragma once

#include <string>
#include <fstream>

namespace devilution {

class CrashHunter {
private:
    std::ofstream logFile;
    bool initialized = false;
    int eventCounter = 0;
    
public:
    void Initialize();
    void Shutdown();
    
    // Logging de eventos críticos
    void LogLevelGeneration(int levelNum, const std::string& levelType);
    void LogMonsterSpawn(int monsterType, int x, int y, const std::string& monsterName);
    void LogItemSpawn(int itemType, int x, int y, const std::string& itemName);
    void LogPlayerPosition(int x, int y, const std::string& context);
    void LogLevelChange(int fromLevel, int toLevel);
    void LogSaveLoad(const std::string& saveFile, bool success);
    void LogDungeonData(const std::string& dataType, const std::string& details);
    void LogCriticalOperation(const std::string& operation, const std::string& details);
    
    // Checkpoint system - marca puntos seguros
    void Checkpoint(const std::string& location);
    
    // Logging genérico
    void Log(const std::string& message);
};

// Global instance
extern CrashHunter g_crashHunter;

// Macros convenientes
#define CRASH_HUNTER_INIT() g_crashHunter.Initialize()
#define CRASH_HUNTER_SHUTDOWN() g_crashHunter.Shutdown()
#define CRASH_HUNTER_LOG(msg) g_crashHunter.Log(msg)
#define CRASH_HUNTER_CHECKPOINT(loc) g_crashHunter.Checkpoint(loc)
#define CRASH_HUNTER_LEVEL_GEN(lvl, type) g_crashHunter.LogLevelGeneration(lvl, type)
#define CRASH_HUNTER_MONSTER(type, x, y, name) g_crashHunter.LogMonsterSpawn(type, x, y, name)
#define CRASH_HUNTER_ITEM(type, x, y, name) g_crashHunter.LogItemSpawn(type, x, y, name)
#define CRASH_HUNTER_PLAYER_POS(x, y, ctx) g_crashHunter.LogPlayerPosition(x, y, ctx)
#define CRASH_HUNTER_LEVEL_CHANGE(from, to) g_crashHunter.LogLevelChange(from, to)
#define CRASH_HUNTER_SAVE_LOAD(file, success) g_crashHunter.LogSaveLoad(file, success)
#define CRASH_HUNTER_DUNGEON_DATA(type, details) g_crashHunter.LogDungeonData(type, details)
#define CRASH_HUNTER_CRITICAL(op, details) g_crashHunter.LogCriticalOperation(op, details)

} // namespace devilution
