#pragma once

/**
 * @file emergency_diagnostics.h
 * 
 * 🚨 EMERGENCY DIAGNOSTICS SYSTEM
 * 
 * Sistema de diagnóstico ultra-agresivo para capturar exactamente
 * qué está fallando en tiempo real.
 */

#include "utils/log.hpp"
#include <fstream>
#include <string>

namespace devilution {

class EmergencyDiagnostics {
private:
    static std::ofstream logFile;
    static bool initialized;

public:
    static void Initialize();
    static void Log(const std::string& message);
    static void LogCritical(const std::string& message);
    static void Shutdown();
};

// Macros para logging inmediato - DISABLED after successful crash fix
#define EMERGENCY_LOG(msg) do { } while(0)
#define EMERGENCY_CRITICAL(msg) do { } while(0)

} // namespace devilution