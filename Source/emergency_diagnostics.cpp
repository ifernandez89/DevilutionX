#include "emergency_diagnostics.h"
#include "utils/paths.h"
#include <iostream>

namespace devilution {

std::ofstream EmergencyDiagnostics::logFile;
bool EmergencyDiagnostics::initialized = false;

void EmergencyDiagnostics::Initialize() {
    if (initialized) return;
    
    std::string logPath = paths::PrefPath() + "emergency_diagnostics.txt";
    logFile.open(logPath, std::ios::out | std::ios::app);
    
    if (logFile.is_open()) {
        logFile << "\n=== EMERGENCY DIAGNOSTICS SESSION START ===" << std::endl;
        logFile.flush();
        initialized = true;
        std::cout << "Emergency diagnostics initialized: " << logPath << std::endl;
    }
}

void EmergencyDiagnostics::Log(const std::string& message) {
    if (!initialized) Initialize();
    
    if (logFile.is_open()) {
        logFile << "[LOG] " << message << std::endl;
        logFile.flush();
    }
    
    // También a consola para debugging inmediato
    std::cout << "[EMERGENCY] " << message << std::endl;
    LogInfo("EMERGENCY: {}", message);
}

void EmergencyDiagnostics::LogCritical(const std::string& message) {
    if (!initialized) Initialize();
    
    if (logFile.is_open()) {
        logFile << "[CRITICAL] " << message << std::endl;
        logFile.flush();
    }
    
    // También a consola para debugging inmediato
    std::cout << "[EMERGENCY CRITICAL] " << message << std::endl;
    LogError("EMERGENCY CRITICAL: {}", message);
}

void EmergencyDiagnostics::Shutdown() {
    if (logFile.is_open()) {
        logFile << "=== EMERGENCY DIAGNOSTICS SESSION END ===" << std::endl;
        logFile.close();
    }
    initialized = false;
}

} // namespace devilution