#include "apocalypse_crash_debug.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include "diablo.h"
#include "engine_health.h"
#include "lighting.h"
#include "missiles.h"
#include "monster.h"
#include "player.h"

namespace devilution {

// ============================================================================
// 🌍 GLOBAL DEBUG STATE
// ============================================================================

static ApocalypseCrashState g_crashState = {};
static std::string g_debugLogPath;
static bool g_debugInitialized = false;
static uint32_t g_frameCounter = 0;

// ============================================================================
// 🔧 CORE IMPLEMENTATION
// ============================================================================

void InitApocalypseCrashDebug()
{
    if (g_debugInitialized) return;
    
    // Generar nombre único para el archivo de debug
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    // Guardar en subdirectorio debug_logs para organización
    ss << "debug_logs/apocalypse_crash_debug_" 
       << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
       << "_" << ms.count() << ".txt";
    
    g_debugLogPath = ss.str();
    g_debugInitialized = true;
    g_frameCounter = 0;
    
    // Crear directorio debug_logs si no existe
    #ifdef _WIN32
        CreateDirectoryA("debug_logs", NULL);
    #else
        mkdir("debug_logs", 0755);
    #endif
    
    // Crear archivo inicial
    std::ofstream file(g_debugLogPath);
    if (file.is_open()) {
        file << "=== APOCALYPSE CRASH DEBUG SYSTEM ===" << std::endl;
        file << "Initialized at: " << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << std::endl;
        file << "Purpose: Capture detailed state information for Apocalypse spell crashes" << std::endl;
        file << "System: LATENT MODE - Only active when crashes occur" << std::endl;
        file << "Location: This file will be in debug_logs/ directory" << std::endl;
        file << "======================================" << std::endl << std::endl;
        file.close();
    }
}

void CapturePreApocalypseState()
{
    if (!g_debugInitialized) return;
    
    // Capturar timestamp más preciso
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    // LOGGING AGRESIVO: Siempre log cuando se llama esta función
    std::ofstream file(g_debugLogPath, std::ios::app);
    if (file.is_open()) {
        file << "[FRAME " << g_frameCounter << "] 🚨 APOCALYPSE CAST DETECTED - CAPTURING STATE (ms: " << ms.count() << ")" << std::endl;
        file.close();
    }
    
    g_frameCounter++; // Increment frame counter each time this is called
    
    // Capturar timestamp
    g_crashState.timestamp = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
    g_crashState.frameNumber = g_frameCounter;
    
    // Estado del jugador
    if (MyPlayer != nullptr) {
        g_crashState.playerLevel = MyPlayer->getCharacterLevel();
        g_crashState.playerX = MyPlayer->position.tile.x;
        g_crashState.playerY = MyPlayer->position.tile.y;
        g_crashState.playerMana = MyPlayer->_pMana;
        g_crashState.spellLevel = MyPlayer->GetSpellLevel(SpellID::Apocalypse);
    }
    
    // Estado de missiles
    g_crashState.totalMissiles = static_cast<int>(Missiles.size());
    g_crashState.apocalypseMissiles = 0;
    g_crashState.apocalypseBoomMissiles = 0;
    g_crashState.infernoMissiles = 0;
    g_crashState.otherMissiles = 0;
    
    for (const auto &missile : Missiles) {
        switch (missile._mitype) {
            case MissileID::Apocalypse:
                g_crashState.apocalypseMissiles++;
                break;
            case MissileID::ApocalypseBoom:
                g_crashState.apocalypseBoomMissiles++;
                break;
            case MissileID::Inferno:
            case MissileID::InfernoControl:
                g_crashState.infernoMissiles++;
                break;
            default:
                g_crashState.otherMissiles++;
                break;
        }
    }
    
    // Estado de monsters
    g_crashState.totalMonsters = static_cast<int>(MaxMonsters);
    g_crashState.activeMonsters = static_cast<int>(ActiveMonsterCount);
    g_crashState.monstersInRange = 0;
    
    // Contar monsters en rango de Apocalypse (16x16 grid)
    if (MyPlayer != nullptr) {
        Point playerPos = MyPlayer->position.tile;
        for (size_t i = 0; i < ActiveMonsterCount; i++) {
            Monster &monster = Monsters[ActiveMonsters[i]];
            if (monster.position.tile.WalkingDistance(playerPos) <= 8) {
                g_crashState.monstersInRange++;
            }
        }
    }
    
    // Estado de luces
    g_crashState.activeLights = ActiveLightCount;
    g_crashState.lightLoad = ActiveLightCount * 3; // Peso estimado
    
    // Estado del nivel
    g_crashState.currentLevel = currlevel;
    g_crashState.levelType = static_cast<int>(leveltype);
    g_crashState.dungeonSeed = 0; // Placeholder - seed not accessible
    
    // Información de memoria
    g_crashState.missileListSize = Missiles.size();
    g_crashState.monsterListSize = ActiveMonsterCount;
    g_crashState.availableMemory = 0; // Placeholder
    
    // Estado del MAL
    g_crashState.malEstimatedLoad = gEngineHealth.estimatedLoad;
    g_crashState.malIsHealthy = gEngineHealth.isHealthy;
    g_crashState.malNeedsThrottling = gEngineHealth.needsThrottling;
    g_crashState.malInCriticalState = gEngineHealth.inCriticalState;
    
    // Log del estado pre-cast MÁS DETALLADO
    std::ofstream file2(g_debugLogPath, std::ios::app);
    if (file2.is_open()) {
        file2 << "[FRAME " << g_frameCounter << "] PRE-APOCALYPSE STATE CAPTURED" << std::endl;
        file2 << "  Player: Level " << g_crashState.playerLevel 
             << " at (" << g_crashState.playerX << "," << g_crashState.playerY << ")" << std::endl;
        file2 << "  Missiles: Total=" << g_crashState.totalMissiles 
             << " Apocalypse=" << g_crashState.apocalypseMissiles
             << " ApocalypseBoom=" << g_crashState.apocalypseBoomMissiles << std::endl;
        file2 << "  Monsters: Active=" << g_crashState.activeMonsters 
             << " InRange=" << g_crashState.monstersInRange << std::endl;
        file2 << "  MAL: Load=" << g_crashState.malEstimatedLoad 
             << " Critical=" << (g_crashState.malInCriticalState ? "YES" : "NO") << std::endl;
        file2 << "  🚨 READY TO TRACK APOCALYPSE PROCESSING..." << std::endl;
        file2 << std::endl;
        file2.close();
    }
}

void CaptureApocalypseProcessingState(const char* location)
{
    if (!g_debugInitialized) return;
    
    // Increment frame counter each time we process
    g_frameCounter++;
    
    g_crashState.lastFunction = location;
    
    std::ofstream file(g_debugLogPath, std::ios::app);
    if (file.is_open()) {
        file << "[FRAME " << g_frameCounter << "] 🔥 PROCESSING: " << location << std::endl;
        file << "  Current missiles: " << Missiles.size() << std::endl;
        file << "  MAL state: Load=" << gEngineHealth.estimatedLoad 
             << " Critical=" << (gEngineHealth.inCriticalState ? "YES" : "NO") << std::endl;
        
        // Contar missiles por tipo en tiempo real
        int apocalypseCount = 0, apocalypseBoomCount = 0;
        for (const auto &missile : Missiles) {
            if (missile._mitype == MissileID::Apocalypse) apocalypseCount++;
            if (missile._mitype == MissileID::ApocalypseBoom) apocalypseBoomCount++;
        }
        file << "  Real-time: Apocalypse=" << apocalypseCount 
             << " ApocalypseBoom=" << apocalypseBoomCount << std::endl;
        
        file << "  🚨 MONITORING FOR CRASH..." << std::endl;
        file.close();
    }
}

void CaptureApocalypseCrashState(const char* crashLocation, const char* errorMsg)
{
    if (!g_debugInitialized) return;
    
    g_crashState.crashLocation = crashLocation;
    g_crashState.errorMessage = errorMsg;
    
    GenerateApocalypseCrashReport();
}

void GenerateApocalypseCrashReport()
{
    if (!g_debugInitialized) return;
    
    std::ofstream file(g_debugLogPath, std::ios::app);
    if (file.is_open()) {
        file << std::endl;
        file << "🚨🚨🚨 APOCALYPSE CRASH DETECTED 🚨🚨🚨" << std::endl;
        file << "========================================" << std::endl;
        
        // Información del crash
        file << "CRASH INFORMATION:" << std::endl;
        file << "  Location: " << g_crashState.crashLocation << std::endl;
        file << "  Error: " << g_crashState.errorMessage << std::endl;
        file << "  Frame: " << g_crashState.frameNumber << std::endl;
        file << "  Timestamp: " << g_crashState.timestamp << std::endl;
        file << std::endl;
        
        // Estado del jugador
        file << "PLAYER STATE:" << std::endl;
        file << "  Level: " << g_crashState.playerLevel << std::endl;
        file << "  Position: (" << g_crashState.playerX << "," << g_crashState.playerY << ")" << std::endl;
        file << "  Mana: " << g_crashState.playerMana << std::endl;
        file << "  Apocalypse Spell Level: " << g_crashState.spellLevel << std::endl;
        file << std::endl;
        
        // Estado de missiles
        file << "MISSILE STATE:" << std::endl;
        file << "  Total Missiles: " << g_crashState.totalMissiles << std::endl;
        file << "  Apocalypse Missiles: " << g_crashState.apocalypseMissiles << std::endl;
        file << "  ApocalypseBoom Missiles: " << g_crashState.apocalypseBoomMissiles << std::endl;
        file << "  Inferno Missiles: " << g_crashState.infernoMissiles << std::endl;
        file << "  Other Missiles: " << g_crashState.otherMissiles << std::endl;
        file << "  Missile List Size: " << g_crashState.missileListSize << std::endl;
        file << std::endl;
        
        // Estado de monsters
        file << "MONSTER STATE:" << std::endl;
        file << "  Total Monsters: " << g_crashState.totalMonsters << std::endl;
        file << "  Active Monsters: " << g_crashState.activeMonsters << std::endl;
        file << "  Monsters in Apocalypse Range: " << g_crashState.monstersInRange << std::endl;
        file << std::endl;
        
        // Estado del nivel
        file << "LEVEL STATE:" << std::endl;
        file << "  Current Level: " << g_crashState.currentLevel << std::endl;
        file << "  Level Type: " << g_crashState.levelType << std::endl;
        file << "  Dungeon Seed: " << g_crashState.dungeonSeed << std::endl;
        file << std::endl;
        
        // Estado del MAL
        file << "MAL (Modern Assist Layer) STATE:" << std::endl;
        file << "  Estimated Load: " << g_crashState.malEstimatedLoad << std::endl;
        file << "  Is Healthy: " << (g_crashState.malIsHealthy ? "YES" : "NO") << std::endl;
        file << "  Needs Throttling: " << (g_crashState.malNeedsThrottling ? "YES" : "NO") << std::endl;
        file << "  In Critical State: " << (g_crashState.malInCriticalState ? "YES" : "NO") << std::endl;
        file << std::endl;
        
        // Análisis del crash
        file << "CRASH ANALYSIS:" << std::endl;
        file << "  Last Function: " << g_crashState.lastFunction << std::endl;
        
        // Posibles causas
        file << "POSSIBLE CAUSES:" << std::endl;
        if (g_crashState.totalMissiles > 400) {
            file << "  ⚠️  HIGH MISSILE COUNT: " << g_crashState.totalMissiles << " missiles active" << std::endl;
        }
        if (g_crashState.apocalypseBoomMissiles > 100) {
            file << "  ⚠️  HIGH APOCALYPSE BOOM COUNT: " << g_crashState.apocalypseBoomMissiles << " ApocalypseBoom missiles" << std::endl;
        }
        if (g_crashState.malInCriticalState) {
            file << "  ⚠️  MAL CRITICAL STATE: Engine was in critical state" << std::endl;
        }
        if (g_crashState.monstersInRange > 50) {
            file << "  ⚠️  HIGH MONSTER DENSITY: " << g_crashState.monstersInRange << " monsters in range" << std::endl;
        }
        
        file << std::endl;
        file << "========================================" << std::endl;
        file << "End of crash report. Please send this file to the developer." << std::endl;
        file.close();
    }
}

void CleanupApocalypseCrashDebug()
{
    if (!g_debugInitialized) return;
    
    std::ofstream file(g_debugLogPath, std::ios::app);
    if (file.is_open()) {
        file << std::endl << "Debug session ended normally." << std::endl;
        file.close();
    }
    
    g_debugInitialized = false;
}

// ============================================================================
// 🚨 CRASH DETECTION HELPERS
// ============================================================================

bool IsApocalypseDangerousState()
{
    // SISTEMA MÁS AGRESIVO: Activar debugging con condiciones más amplias
    // para capturar el crash que está ocurriendo
    
    // Verificar condiciones críticas que indican posible crash
    if (Missiles.size() > 100) return true;           // Reducido de 250 a 100
    if (gEngineHealth.activeApocalypse > 0) return true;  // Cualquier Apocalypse activo
    if (gEngineHealth.inCriticalState) return true;   // MAL en estado crítico
    if (gEngineHealth.estimatedLoad > 100) return true;   // Reducido de 200 a 100
    
    // Verificar patrones específicos de Apocalypse que causan problemas
    int apocalypseBoomCount = 0;
    for (const auto &missile : Missiles) {
        if (missile._mitype == MissileID::ApocalypseBoom) {
            apocalypseBoomCount++;
        }
    }
    
    if (apocalypseBoomCount > 10) return true;  // Reducido de 50 a 10
    
    // MODO ULTRA AGRESIVO: Activar siempre que haya cualquier actividad de Apocalypse
    for (const auto &missile : Missiles) {
        if (missile._mitype == MissileID::Apocalypse) {
            return true; // Cualquier missile de Apocalypse activa el debugging
        }
    }
    
    return false;
}

void LogDangerousOperation(const char* operation)
{
    if (!g_debugInitialized) return;
    
    std::ofstream file(g_debugLogPath, std::ios::app);
    if (file.is_open()) {
        file << "[FRAME " << g_frameCounter << "] ⚠️  DANGEROUS OPERATION: " << operation << std::endl;
        file << "  Missiles: " << Missiles.size() << std::endl;
        file << "  MAL Load: " << gEngineHealth.estimatedLoad << std::endl;
        file.close();
    }
}

bool ValidateMissileListIntegrity()
{
    // Verificación básica de integridad
    size_t count = 0;
    for (const auto &missile : Missiles) {
        (void)missile; // Evitar warning de variable no usada
        count++;
        if (count > 1000) { // Límite de seguridad
            LogDangerousOperation("Missile list too large during iteration");
            return false;
        }
    }
    return true;
}

bool ValidateIteratorIntegrity()
{
    // Verificación de integridad de iteradores
    auto it = Missiles.begin();
    auto end = Missiles.end();
    size_t distance = std::distance(it, end);
    
    if (distance != Missiles.size()) {
        LogDangerousOperation("Iterator distance mismatch");
        return false;
    }
    
    return true;
}

} // namespace devilution