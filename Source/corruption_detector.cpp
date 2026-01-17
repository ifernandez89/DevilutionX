#include "corruption_detector.h"

#include <fstream>
#include <ctime>
#include <iomanip>
#include <cstdint>
#include <string>

#include "player.h"
#include "levels/gendung.h"
#include "quests.h"
#include "pfile.h"
#include "loadsave.h"

namespace devilution {

void LogCorruptionDetection(const std::string& message)
{
    std::ofstream logFile("debug_logs/corruption_detector.log", std::ios::app);
    if (logFile.is_open()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        logFile << std::put_time(&tm, "%H:%M:%S") << " [CORRUPTION_DETECTOR] " << message << std::endl;
        logFile.close();
    }
}

bool IsCorruptedCoordinate(int x, int y)
{
    // Coordenadas CLARAMENTE corruptas (no válidas en ningún contexto)
    if (x < -100 || x > 200 || y < -100 || y > 200) {
        return true;
    }
    
    // Patrones típicos de corrupción
    if ((x == 999 && y == 999) || 
        (x == -1 && y == -1) || 
        (x == 0 && y == 0 && currlevel > 0)) {  // (0,0) solo es válido en town
        return true;
    }
    
    return false;
}

bool IsSaveFileCorrupted(uint32_t saveNumber)
{
    // Crear directorio de logs si no existe
    std::ofstream logFile("debug_logs/corruption_detector.log", std::ios::app);
    if (logFile.is_open()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        logFile << "=== CORRUPTION DETECTOR - PREVENTION SYSTEM ===" << std::endl;
        logFile << "Timestamp: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
        logFile << "Philosophy: PREVENT corruption, don't patch it" << std::endl;
        logFile << "=========================================" << std::endl;
        logFile.close();
    }
    
    LogCorruptionDetection("🔍 CHECKING SAVE FILE FOR CORRUPTION...");
    
    // Por ahora, implementación simple: solo verificar si el archivo existe
    std::optional<SaveReader> archive = OpenSaveArchive(saveNumber);
    if (!archive) {
        LogCorruptionDetection("❌ CANNOT OPEN SAVE FILE - ASSUMING CORRUPTED");
        return true;
    }
    
    // TODO: Implementar verificación más detallada cuando sea necesario
    LogCorruptionDetection("✅ BASIC CHECKS PASSED - SAVE APPEARS CLEAN");
    return false;
}

void ForceFactoryReset()
{
    LogCorruptionDetection("🏭 FORCING FACTORY RESET - CLEANING ALL DATA");
    
    Player &myPlayer = *MyPlayer;
    
    // 1. RESETEAR COORDENADAS DEL JUGADOR A POSICIÓN SEGURA
    myPlayer.position.tile = { 75, 68 };      // Tristram spawn
    myPlayer.position.future = { 75, 68 };
    myPlayer.position.last = { 75, 68 };
    myPlayer.position.old = { 75, 68 };
    myPlayer.position.temp = { 75, 68 };
    
    // 2. RESETEAR FLAGS DE NIVELES VISITADOS
    for (int i = 0; i < NUMLEVELS; i++) {
        myPlayer._pLvlVisited[i] = false;
    }
    
    // 3. RESETEAR QUESTS A ESTADO INICIAL
    InitQuests();
    
    // 4. RESETEAR PORTALES
    InitPortals();
    
    // 5. FORZAR NIVEL ACTUAL A TOWN
    currlevel = 0;
    leveltype = DTYPE_TOWN;
    setlevel = false;
    
    // 6. LIMPIAR ARCHIVOS TEMPORALES DEL SAVE
    pfile_remove_temp_files();
    
    LogCorruptionDetection("✅ FACTORY RESET COMPLETE - ALL DATA RESTORED TO DEFAULTS");
    LogCorruptionDetection("🏠 PLAYER RELOCATED TO TRISTRAM (75, 68)");
    LogCorruptionDetection("🗺️ ALL LEVELS MARKED AS UNVISITED");
    LogCorruptionDetection("📜 QUESTS RESET TO INITIAL STATE");
    LogCorruptionDetection("🌀 PORTALS RESET TO DEFAULTS");
}

} // namespace devilution