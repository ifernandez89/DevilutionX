/**
 * @file crash_hunter.cpp
 * @brief CRASH HUNTER - Implementación del sistema de logging agresivo
 */

#include "crash_hunter.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace devilution {

CrashHunter g_crashHunter;

void CrashHunter::Initialize()
{
    logFile.open("debug_logs/crash_hunter.log", std::ios::trunc);
    if (logFile.is_open()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        
        logFile << "========================================" << std::endl;
        logFile << "    CRASH HUNTER - AGGRESSIVE LOGGING   " << std::endl;
        logFile << "========================================" << std::endl;
        logFile << "Timestamp: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
        logFile << "Mission: Find the crash culprit" << std::endl;
        logFile << "Strategy: Log EVERYTHING related to dungeons" << std::endl;
        logFile << "========================================" << std::endl << std::endl;
        
        initialized = true;
        eventCounter = 0;
    }
}

void CrashHunter::Shutdown()
{
    if (logFile.is_open()) {
        logFile << std::endl << "========================================" << std::endl;
        logFile << "CRASH HUNTER SHUTDOWN - Total events: " << eventCounter << std::endl;
        logFile << "========================================" << std::endl;
        logFile.close();
    }
    initialized = false;
}

void CrashHunter::LogLevelGeneration(int levelNum, const std::string& levelType)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "🏗️ LEVEL GENERATION: Level " << levelNum << " (" << levelType << ")" << std::endl;
    logFile.flush();
}

void CrashHunter::LogMonsterSpawn(int monsterType, int x, int y, const std::string& monsterName)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "👹 MONSTER SPAWN: " << monsterName << " (Type:" << monsterType 
            << ") at (" << x << "," << y << ")" << std::endl;
    logFile.flush();
}

void CrashHunter::LogItemSpawn(int itemType, int x, int y, const std::string& itemName)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "💎 ITEM SPAWN: " << itemName << " (Type:" << itemType 
            << ") at (" << x << "," << y << ")" << std::endl;
    logFile.flush();
}

void CrashHunter::LogPlayerPosition(int x, int y, const std::string& context)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "🚶 PLAYER POSITION: (" << x << "," << y << ") - " << context << std::endl;
    logFile.flush();
}

void CrashHunter::LogLevelChange(int fromLevel, int toLevel)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "🚪 LEVEL CHANGE: From Level " << fromLevel << " to Level " << toLevel << std::endl;
    logFile.flush();
}

void CrashHunter::LogSaveLoad(const std::string& saveFile, bool success)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "💾 SAVE LOAD: " << saveFile << " - " << (success ? "SUCCESS" : "FAILED") << std::endl;
    logFile.flush();
}

void CrashHunter::LogDungeonData(const std::string& dataType, const std::string& details)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "📊 DUNGEON DATA: " << dataType << " - " << details << std::endl;
    logFile.flush();
}

void CrashHunter::LogCriticalOperation(const std::string& operation, const std::string& details)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [EVENT #" << ++eventCounter << "] "
            << "⚠️ CRITICAL: " << operation << " - " << details << std::endl;
    logFile.flush();
}

void CrashHunter::Checkpoint(const std::string& location)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [CHECKPOINT #" << ++eventCounter << "] "
            << "✅ " << location << std::endl;
    logFile.flush();
}

void CrashHunter::Log(const std::string& message)
{
    if (!initialized) return;
    
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    logFile << std::put_time(&tm, "%H:%M:%S") << " [LOG #" << ++eventCounter << "] "
            << message << std::endl;
    logFile.flush();
}

} // namespace devilution
