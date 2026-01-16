/**
 * @file global_protection_system.cpp
 * 
 * Implementación del Sistema Global de Protección
 */

#include "global_protection_system.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>

#include "diablo.h"
#include "missiles.h"
#include "player.h"

namespace devilution {

// ============================================================================
// 🌍 SINGLETON IMPLEMENTATION
// ============================================================================

GlobalProtectionSystem* GlobalProtectionSystem::instance = nullptr;

GlobalProtectionSystem& GlobalProtectionSystem::getInstance() {
    if (instance == nullptr) {
        instance = new GlobalProtectionSystem();
        instance->stats.lastSecondReset = std::chrono::steady_clock::now();
        instance->stats.lastFrameTime = std::chrono::steady_clock::now();
    }
    return *instance;
}

// ============================================================================
// 🔧 ADAPTIVE LIMITS IMPLEMENTATION
// ============================================================================

void AdaptiveLimits::adaptToPerformance(int performanceScore) {
    // 🚀 ALGORITMO ULTRA-MODERNO 2026 - MÁXIMA RESPONSIVIDAD + ANTI-CRASH
    // Hardware moderno puede manejar 15x más que hardware de los 90s
    
    float performanceFactor = performanceScore / 100.0f;
    
    // 🎯 LÍMITES BASE ULTRA-GENEROSOS PARA FAST-CLICK GAMING
    int baseMissiles = 200;     // Era 150, ahora 200 - Hardware ultra-moderno
    int baseSpells = 60;        // Era 40, ahora 60 - Ultra fast-click ready
    int baseMelee = 80;         // Era 60, ahora 80 - Combo ultra-friendly
    int baseActions = 150;      // Era 100, ahora 150 - Gaming ultra-responsivo
    
    // 🚀 ESCALADO ULTRA-AGRESIVO PARA HARDWARE POTENTE
    maxMissiles = static_cast<int>(baseMissiles * (1.0f + performanceFactor * 1.0f));
    maxSpellsPerSecond = static_cast<int>(baseSpells * (1.0f + performanceFactor * 0.8f));
    maxMeleePerSecond = static_cast<int>(baseMelee * (1.0f + performanceFactor * 0.6f));
    maxActionsPerSecond = static_cast<int>(baseActions * (1.0f + performanceFactor * 0.4f));
    
    // 🎮 LÍMITES MÁXIMOS PARA GAMING EXTREMO (Hardware 2026)
    maxMissiles = std::min(maxMissiles, 800);           // 500 → 800 (1.6x más)
    maxSpellsPerSecond = std::min(maxSpellsPerSecond, 200);    // 120 → 200 (1.7x más)
    maxMeleePerSecond = std::min(maxMeleePerSecond, 250);      // 150 → 250 (1.7x más)
    maxActionsPerSecond = std::min(maxActionsPerSecond, 400);  // 250 → 400 (1.6x más)
    
    // 🎯 LÍMITES MÍNIMOS ULTRA-ALTOS - NUNCA MENOS QUE GAMING FLUIDO
    maxMissiles = std::max(maxMissiles, 180);          // 120 → 180 (1.5x más)
    maxSpellsPerSecond = std::max(maxSpellsPerSecond, 50);     // 30 → 50 (1.7x más)
    maxMeleePerSecond = std::max(maxMeleePerSecond, 60);       // 40 → 60 (1.5x más)
    maxActionsPerSecond = std::max(maxActionsPerSecond, 120);   // 80 → 120 (1.5x más)
    
    // 🚨 EMERGENCY THRESHOLD ULTRA-TOLERANTE
    emergencyResetThreshold = 45; // 30 → 45 frames (0.75 segundos)
}

// ============================================================================
// 🔍 MONITOREO Y ANÁLISIS
// ============================================================================

void GlobalProtectionSystem::updateFrameStats() {
    auto now = std::chrono::steady_clock::now();
    
    // Calcular frame time
    auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - stats.lastFrameTime).count();
    stats.lastFrameTime = now;
    
    // Actualizar promedio de frame time (smoothed)
    stats.averageFrameTime = (stats.averageFrameTime * 9 + static_cast<int>(frameTime)) / 10;
    
    // Calcular performance score basado en frame time
    // 16ms = 60 FPS = 100 score, 33ms = 30 FPS = 50 score
    if (stats.averageFrameTime <= 16) {
        stats.performanceScore = 100;
    } else if (stats.averageFrameTime >= 33) {
        stats.performanceScore = 30;
    } else {
        stats.performanceScore = 100 - ((stats.averageFrameTime - 16) * 70 / 17);
    }
    
    // Adaptar límites basado en performance
    limits.adaptToPerformance(stats.performanceScore);
    
    // Reset contadores cada segundo
    auto timeSinceReset = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - stats.lastSecondReset).count();
    
    if (timeSinceReset >= 1000) {
        stats.spellsThisSecond = 0;
        stats.meleeAttacksThisSecond = 0;
        stats.totalActionsThisSecond = 0;
        stats.lastSecondReset = now;
        actionCounters.clear();
    }
    
    // Actualizar conteo de missiles actuales
    stats.currentMissiles = static_cast<int>(Missiles.size());
    
    // 🎮 THRESHOLDS ULTRA-TOLERANTES PARA GAMING MODERNO
    // Solo intervenir cuando realmente estemos en peligro extremo
    // Ahora que tenemos detección ultra-agresiva en DoSpell, GPS puede ser más permisivo
    AlertLevel newAlert = AlertLevel::NORMAL;
    
    // 🚨 CRITICAL: Solo cuando estemos al 99.5% - casi overflow real
    if (stats.currentMissiles > limits.maxMissiles * 0.995f ||
        stats.spellsThisSecond > limits.maxSpellsPerSecond * 0.995f ||
        stats.totalActionsThisSecond > limits.maxActionsPerSecond * 0.995f) {
        newAlert = AlertLevel::CRITICAL;
    } 
    // ⚠️ WARNING: Solo al 99% - muy cerca del límite
    else if (stats.currentMissiles > limits.maxMissiles * 0.99f ||
               stats.spellsThisSecond > limits.maxSpellsPerSecond * 0.99f ||
               stats.totalActionsThisSecond > limits.maxActionsPerSecond * 0.99f) {
        newAlert = AlertLevel::WARNING;
    } 
    // 🔶 CAUTION: Solo al 97% - aún muy tolerante
    else if (stats.currentMissiles > limits.maxMissiles * 0.97f ||
               stats.spellsThisSecond > limits.maxSpellsPerSecond * 0.97f ||
               stats.totalActionsThisSecond > limits.maxActionsPerSecond * 0.97f) {
        newAlert = AlertLevel::CAUTION;
    }
    
    // Gestionar modo de emergencia
    if (newAlert >= AlertLevel::CRITICAL) {
        emergencyFrameCount++;
        if (emergencyFrameCount >= limits.emergencyResetThreshold && !emergencyMode) {
            activateEmergencyMode("Critical resource usage detected");
        }
    } else {
        emergencyFrameCount = 0;
        if (emergencyMode && newAlert <= AlertLevel::CAUTION) {
            deactivateEmergencyMode();
        }
    }
    
    stats.currentAlert = newAlert;
}

bool GlobalProtectionSystem::registerAction(const std::string& actionType, const std::string& details) {
    // Incrementar contadores
    actionCounters[actionType]++;
    stats.totalActionsThisSecond++;
    
    // Contadores específicos
    if (actionType.find("spell") != std::string::npos) {
        stats.spellsThisSecond++;
    } else if (actionType.find("melee") != std::string::npos) {
        stats.meleeAttacksThisSecond++;
    }
    
    // Log SOLO si estamos en estado crítico o emergency
    if (stats.currentAlert >= AlertLevel::CRITICAL) {
        logCriticalEvent("Action registered: " + actionType + " " + details, stats.currentAlert);
    }
    
    return true;
}

bool GlobalProtectionSystem::isActionSafe(const std::string& actionType) {
    // En modo de emergencia, solo permitir acciones críticas
    if (emergencyMode) {
        return actionType == "emergency_cleanup" || actionType == "player_movement";
    }
    
    // Verificar límites específicos
    if (actionType.find("spell") != std::string::npos) {
        return stats.spellsThisSecond < limits.maxSpellsPerSecond;
    } else if (actionType.find("melee") != std::string::npos) {
        return stats.meleeAttacksThisSecond < limits.maxMeleePerSecond;
    }
    
    // Verificar límite general de acciones
    return stats.totalActionsThisSecond < limits.maxActionsPerSecond;
}

AlertLevel GlobalProtectionSystem::getCurrentAlertLevel() {
    return stats.currentAlert;
}

// ============================================================================
// 🛡️ PROTECCIÓN ESPECÍFICA
// ============================================================================

bool GlobalProtectionSystem::canCastSpell(const std::string& spellName) {
    // 🎮 FAST-CLICK GAMING: Solo verificar si realmente estamos en peligro extremo
    if (!isActionSafe("spell_cast")) {
        return false;
    }
    
    // SIMPLIFIED: No spell restrictions - trust original engine
    return true;
}

bool GlobalProtectionSystem::canPerformMeleeAttack() {
    return isActionSafe("melee_attack") && stats.meleeAttacksThisSecond < limits.maxMeleePerSecond;
}

bool GlobalProtectionSystem::canCreateMissile(const std::string& missileType) {
    // 🎮 GAMING MODERNO: Solo limitar cuando realmente estemos en overflow
    if (stats.currentMissiles >= limits.maxMissiles) {
        return false;
    }
    
    // SIMPLIFIED: No missile restrictions - trust original engine
    return true;
}

bool GlobalProtectionSystem::canChangePlayerMode(const std::string& newMode) {
    // Prevenir cambios de modo en emergencia
    if (emergencyMode && newMode != "PM_STAND") {
        return false;
    }
    
    return true;
}

// ============================================================================
// 🚨 GESTIÓN DE EMERGENCIAS
// ============================================================================

void GlobalProtectionSystem::activateEmergencyMode(const std::string& reason) {
    if (!emergencyMode) {
        emergencyMode = true;
        logCriticalEvent("EMERGENCY MODE ACTIVATED: " + reason, AlertLevel::EMERGENCY);
        
        // Forzar limpieza inmediata
        forceResourceCleanup();
    }
}

void GlobalProtectionSystem::deactivateEmergencyMode() {
    if (emergencyMode) {
        emergencyMode = false;
        emergencyFrameCount = 0;
        logCriticalEvent("Emergency mode deactivated - system stabilized", AlertLevel::NORMAL);
    }
}

void GlobalProtectionSystem::forceResourceCleanup() {
    // Limpiar missiles excesivos
    if (stats.currentMissiles > limits.maxMissiles) {
        int toRemove = stats.currentMissiles - (limits.maxMissiles / 2);
        int removed = 0;
        
        // SIMPLIFIED: No selective missile removal - let engine handle naturally
        for (auto& missile : Missiles) {
            if (removed >= toRemove) break;
            
            // Remove any missile type equally - no discrimination
            missile._miDelFlag = true;
            removed++;
        }
        
        logCriticalEvent("Emergency cleanup: removed " + std::to_string(removed) + " missiles", AlertLevel::EMERGENCY);
    }
    
    // Reset contadores
    actionCounters.clear();
    stats.spellsThisSecond = 0;
    stats.meleeAttacksThisSecond = 0;
    stats.totalActionsThisSecond = 0;
}

void GlobalProtectionSystem::emergencyReset() {
    logCriticalEvent("EMERGENCY RESET - Full system cleanup", AlertLevel::EMERGENCY);
    
    // Limpiar todos los missiles
    for (auto& missile : Missiles) {
        missile._miDelFlag = true;
    }
    
    // Reset completo de estadísticas
    stats = SystemStats();
    stats.lastSecondReset = std::chrono::steady_clock::now();
    stats.lastFrameTime = std::chrono::steady_clock::now();
    
    actionCounters.clear();
    emergencyMode = false;
    emergencyFrameCount = 0;
    
    // Forzar player a modo seguro
    if (MyPlayer != nullptr) {
        StartStand(*MyPlayer, MyPlayer->_pdir);
        // Reset player state variables manually since ClearStateVariables is not accessible
        MyPlayer->position.temp = { 0, 0 };
        MyPlayer->tempDirection = Direction::South;
        MyPlayer->queuedSpell.spellLevel = 0;
    }
}

// ============================================================================
// 📊 ESTADÍSTICAS Y DEBUGGING
// ============================================================================

std::string GlobalProtectionSystem::generateStatusReport() {
    std::stringstream report;
    
    report << "=== GLOBAL PROTECTION SYSTEM STATUS ===" << std::endl;
    report << "Performance Score: " << stats.performanceScore << "/100" << std::endl;
    report << "Alert Level: " << static_cast<int>(stats.currentAlert) << std::endl;
    report << "Emergency Mode: " << (emergencyMode ? "ACTIVE" : "INACTIVE") << std::endl;
    report << std::endl;
    
    report << "CURRENT USAGE:" << std::endl;
    report << "  Missiles: " << stats.currentMissiles << "/" << limits.maxMissiles << std::endl;
    report << "  Spells/sec: " << stats.spellsThisSecond << "/" << limits.maxSpellsPerSecond << std::endl;
    report << "  Melee/sec: " << stats.meleeAttacksThisSecond << "/" << limits.maxMeleePerSecond << std::endl;
    report << "  Actions/sec: " << stats.totalActionsThisSecond << "/" << limits.maxActionsPerSecond << std::endl;
    report << "  Frame time: " << stats.averageFrameTime << "ms" << std::endl;
    report << std::endl;
    
    if (!actionCounters.empty()) {
        report << "ACTION BREAKDOWN:" << std::endl;
        for (const auto& pair : actionCounters) {
            report << "  " << pair.first << ": " << pair.second << std::endl;
        }
    }
    
    return report.str();
}

void GlobalProtectionSystem::logCriticalEvent(const std::string& event, AlertLevel level) {
    // Solo log eventos REALMENTE importantes para evitar spam
    if (level < AlertLevel::CRITICAL) return; // Cambiado de WARNING a CRITICAL
    
    std::string levelStr;
    switch (level) {
        case AlertLevel::CRITICAL: levelStr = "CRITICAL"; break;
        case AlertLevel::EMERGENCY: levelStr = "EMERGENCY"; break;
        default: levelStr = "INFO"; break;
    }
    
    // Log a archivo solo si es EMERGENCY
    if (level >= AlertLevel::EMERGENCY) {
        std::ofstream logFile("debug_logs/global_protection.log", std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            
            logFile << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] "
                   << levelStr << ": " << event << std::endl;
            logFile.close();
        }
    }
}

} // namespace devilution