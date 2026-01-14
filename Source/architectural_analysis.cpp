/**
 * @file architectural_analysis.cpp
 * 
 * Implementación del sistema permanente de análisis arquitectónico
 */

#include "architectural_analysis.h"
#include "missiles.h"
#include <iostream>

namespace devilution {

ArchitecturalAnalyzer* ArchitecturalAnalyzer::instance = nullptr;

ArchitecturalAnalyzer& ArchitecturalAnalyzer::getInstance() {
    if (instance == nullptr) {
        instance = new ArchitecturalAnalyzer();
        instance->initialize();
    }
    return *instance;
}

void ArchitecturalAnalyzer::initialize() {
    logPath = "debug_logs/architectural_analysis.log";
    isActive = true;
    
    // Limpiar log anterior y escribir header
    std::ofstream logFile(logPath, std::ios::trunc);
    if (logFile.is_open()) {
        logFile << "=== ARCHITECTURAL ANALYSIS - PERMANENT LOGGING ===" << std::endl;
        logFile << "Initialized at: " << getCurrentTimestamp() << std::endl;
        logFile << "Purpose: Understand how our design coexists with original code" << std::endl;
        logFile << "Goal: Detect interference and over-engineering" << std::endl;
        logFile << "Philosophy: Log to redesign, not to patch" << std::endl;
        logFile << "======================================================" << std::endl << std::endl;
        logFile.close();
    }
    
    std::cout << "[ARCH_ANALYZER] Permanent architectural logging initialized" << std::endl;
}

void ArchitecturalAnalyzer::shutdown() {
    if (isActive) {
        generateArchitecturalReport();
        isActive = false;
    }
}

void ArchitecturalAnalyzer::logApocalypseCast(int playerId, int spellLevel, int currentMissiles) {
    apocalypseCasts++;
    lastApocalypseCast = std::chrono::steady_clock::now();
    
    std::string message = "[APOCALYPSE_CAST] Player:" + std::to_string(playerId) + 
                         " Level:" + std::to_string(spellLevel) + 
                         " CurrentMissiles:" + std::to_string(currentMissiles) +
                         " CastCount:" + std::to_string(apocalypseCasts);
    writeLog(message);
}

void ArchitecturalAnalyzer::logMissileCreation(const std::string& missileType, bool success, int totalMissiles) {
    if (success) {
        missileCreations++;
    } else {
        missileFailures++;
    }
    lastMissileCreation = std::chrono::steady_clock::now();
    
    std::string message = "[MISSILE_CREATION] Type:" + missileType + 
                         " Success:" + (success ? "YES" : "NO") + 
                         " Total:" + std::to_string(totalMissiles) +
                         " Successes:" + std::to_string(missileCreations) +
                         " Failures:" + std::to_string(missileFailures);
    writeLog(message);
}

void ArchitecturalAnalyzer::logProcessApocalypse(int var2, int var3, int var4, int var5, int currentMissiles) {
    processApocalypseCalls++;
    
    std::string message = "[PROCESS_APOCALYPSE] Call#" + std::to_string(processApocalypseCalls) +
                         " var2:" + std::to_string(var2) + 
                         " var3:" + std::to_string(var3) +
                         " var4:" + std::to_string(var4) + 
                         " var5:" + std::to_string(var5) +
                         " Missiles:" + std::to_string(currentMissiles);
    writeLog(message);
}

void ArchitecturalAnalyzer::logApocalypseBoomCreation(int x, int y, int totalMissiles) {
    boomCreations++;
    
    std::string message = "[BOOM_CREATION] Pos:(" + std::to_string(x) + "," + std::to_string(y) + ")" +
                         " BoomCount:" + std::to_string(boomCreations) +
                         " TotalMissiles:" + std::to_string(totalMissiles);
    writeLog(message);
}

void ArchitecturalAnalyzer::logCrashPrevention(const std::string& reason, const std::string& location) {
    std::string message = "[CRASH_PREVENTION] Location:" + location + " Reason:" + reason;
    writeLog(message);
}

void ArchitecturalAnalyzer::logArchitecturalConflict(const std::string& ourSystem, const std::string& originalSystem, const std::string& conflict) {
    std::string message = "[ARCHITECTURAL_CONFLICT] Our:" + ourSystem + 
                         " Original:" + originalSystem + 
                         " Conflict:" + conflict;
    writeLog(message);
}

void ArchitecturalAnalyzer::analyzePatterns() {
    std::string analysis = "\n=== PATTERN ANALYSIS ===\n";
    analysis += "Apocalypse Casts: " + std::to_string(apocalypseCasts) + "\n";
    analysis += "Missile Creations: " + std::to_string(missileCreations) + "\n";
    analysis += "Missile Failures: " + std::to_string(missileFailures) + "\n";
    analysis += "Process Apocalypse Calls: " + std::to_string(processApocalypseCalls) + "\n";
    analysis += "Boom Creations: " + std::to_string(boomCreations) + "\n";
    
    // Análisis de ratios
    if (apocalypseCasts > 0) {
        float processRatio = (float)processApocalypseCalls / apocalypseCasts;
        float boomRatio = (float)boomCreations / apocalypseCasts;
        analysis += "Process/Cast Ratio: " + std::to_string(processRatio) + "\n";
        analysis += "Boom/Cast Ratio: " + std::to_string(boomRatio) + "\n";
        
        if (processRatio > 100) {
            analysis += "⚠️ WARNING: Excessive ProcessApocalypse calls - possible infinite loop\n";
        }
        if (boomRatio > 256) {
            analysis += "⚠️ WARNING: Excessive boom creation - possible memory issue\n";
        }
    }
    
    if (missileCreations + missileFailures > 0) {
        float failureRate = (float)missileFailures / (missileCreations + missileFailures);
        analysis += "Missile Failure Rate: " + std::to_string(failureRate * 100) + "%\n";
        
        if (failureRate > 0.5) {
            analysis += "⚠️ WARNING: High missile failure rate - capacity issues\n";
        }
    }
    
    analysis += "========================\n\n";
    writeLog(analysis);
}

void ArchitecturalAnalyzer::generateArchitecturalReport() {
    analyzePatterns();
    
    std::string report = "\n=== FINAL ARCHITECTURAL REPORT ===\n";
    report += "Session Summary:\n";
    report += "- Total Apocalypse casts: " + std::to_string(apocalypseCasts) + "\n";
    report += "- Total missile operations: " + std::to_string(missileCreations + missileFailures) + "\n";
    report += "- Total boom creations: " + std::to_string(boomCreations) + "\n";
    
    // Diagnóstico arquitectónico
    report += "\nArchitectural Diagnosis:\n";
    
    if (apocalypseCasts == 0) {
        report += "- No Apocalypse casts detected - crash before spell execution\n";
    } else if (processApocalypseCalls == 0) {
        report += "- Apocalypse cast but never processed - crash in spell initialization\n";
    } else if (boomCreations == 0) {
        report += "- ProcessApocalypse called but no booms created - crash in boom creation\n";
    } else {
        report += "- Normal execution flow detected\n";
    }
    
    if (isRapidFire()) {
        report += "- Rapid-fire pattern detected - possible infinite loop\n";
    }
    
    report += "\nRecommendations:\n";
    if (processApocalypseCalls > apocalypseCasts * 10) {
        report += "- CRITICAL: ProcessApocalypse called too many times - check for infinite loops\n";
    }
    if (missileFailures > missileCreations) {
        report += "- CRITICAL: More failures than successes - capacity or logic issue\n";
    }
    if (boomCreations > 1000) {
        report += "- WARNING: Excessive boom creation - consider limiting\n";
    }
    
    report += "=====================================\n";
    writeLog(report);
}

void ArchitecturalAnalyzer::writeLog(const std::string& message) {
    if (!isActive) return;
    
    std::ofstream logFile(logPath, std::ios::app);
    if (logFile.is_open()) {
        logFile << getCurrentTimestamp() << " " << message << std::endl;
        logFile.close();
    }
}

std::string ArchitecturalAnalyzer::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    return ss.str();
}

bool ArchitecturalAnalyzer::isRapidFire() {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastCast = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastApocalypseCast).count();
    auto timeSinceLastMissile = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMissileCreation).count();
    
    return (timeSinceLastCast < 100 && apocalypseCasts > 5) || 
           (timeSinceLastMissile < 10 && missileCreations > 50);
}

} // namespace devilution