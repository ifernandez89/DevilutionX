#include "architectural_shield.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include "player.h"
#include "levels/gendung.h"

namespace devilution {

namespace {

void LogArchitecturalProtection(const std::string& message)
{
    std::ofstream logFile("debug_logs/architectural_shield.log", std::ios::app);
    if (logFile.is_open()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        logFile << std::put_time(&tm, "%H:%M:%S") << " [ARCH_SHIELD] " << message << std::endl;
        logFile.close();
    }
}

int SanitizeCoordinate(int coord, const std::string& coordName)
{
    // 🚨 CRITICAL FIX: Coordenadas de Tristram van hasta ~80
    // No clampar coordenadas válidas de town
    if (coord < -50 || coord > 120) {  // Rango expandido para Tristram
        LogArchitecturalProtection("🛡️ SANITIZED " + coordName + ": " + std::to_string(coord) + " -> 75");
        return 75; // Coordenada segura en Tristram
    }
    
    // Si está ligeramente fuera de rango pero no es extremo, permitirlo
    if (coord < 0) {
        LogArchitecturalProtection("🛡️ CLAMPED " + coordName + ": " + std::to_string(coord) + " -> 0");
        return 0;
    }
    
    return coord; // Coordenada es válida
}

} // anonymous namespace

bool SanitizeCoordinates(int& x, int& y)
{
    // 🚫 DESACTIVADO - Sistema agresivo causaba más problemas que soluciones
    // NO sanitizar coordenadas - dejar que el juego maneje sus propios datos
    return false;
}

void SanitizePaletteData(void* paletteData, size_t dataSize)
{
    // 🚫 DESACTIVADO - Sistema agresivo causaba más problemas que soluciones
    // NO sanitizar paletas - dejar que el juego maneje sus propios datos
    return;
}

void InterceptAndSanitizeData(void* data, const std::string& dataType, size_t dataSize)
{
    // 🚫 DESACTIVADO COMPLETAMENTE - Sistema agresivo causaba más problemas que soluciones
    // NO interceptar ni sanitizar NADA - dejar que el juego maneje sus propios datos
    return;
}

void InitializeArchitecturalShield()
{
    // 🚫 DESACTIVADO COMPLETAMENTE - Sistema agresivo causaba más problemas que soluciones
    std::ofstream logFile("debug_logs/architectural_shield.log", std::ios::trunc);
    if (logFile.is_open()) {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        logFile << "=== ARCHITECTURAL SHIELD - DISABLED ===" << std::endl;
        logFile << "Timestamp: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
        logFile << "Status: COMPLETELY DISABLED" << std::endl;
        logFile << "Reason: Aggressive sanitization was causing more problems than solving" << std::endl;
        logFile << "Philosophy: Let the game handle its own data - TRUST DEVILUTIONX ORIGINAL CODE" << std::endl;
        logFile << "=========================================" << std::endl << std::endl;
        logFile.close();
    }
    
    LogArchitecturalProtection("🚫 ARCHITECTURAL SHIELD DISABLED - NO SANITIZATION ACTIVE");
}

} // namespace devilution