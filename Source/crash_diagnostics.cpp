#include "crash_diagnostics.h"
#include "diablo.h"
#include "player.h"
#include "monster.h"
#include "missiles.h"
#include "objects.h"
#include "items.h"
#include "utils/log.hpp"
#include "utils/file_util.h"
#include "utils/paths.h"
#include <fstream>
#include <chrono>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// 🔍 GLOBAL STATE
// ============================================================================

static CrashDiagnosticsState g_crashDiagnostics;

// ============================================================================
// 🔍 IMPLEMENTATION
// ============================================================================

void InitCrashDiagnostics()
{
    g_crashDiagnostics = CrashDiagnosticsState{};
    g_crashDiagnostics.sessionStartTime = SDL_GetTicks();
    g_crashDiagnostics.enabled = true;
    g_crashDiagnostics.verboseLogging = true; // Habilitar logging detallado por defecto
    
    LogInfo("🔍 CRASH DIAGNOSTICS SYSTEM INITIALIZED");
    LogInfo("   Session start time: {}", g_crashDiagnostics.sessionStartTime);
    LogInfo("   Verbose logging: {}", g_crashDiagnostics.verboseLogging ? "ENABLED" : "DISABLED");
    
    // Registrar evento de inicio
    RegisterCrashEvent("SYSTEM_INIT", "Crash diagnostics system initialized", "");
}

GameStateSnapshot CaptureGameStateSnapshot()
{
    GameStateSnapshot snapshot;
    snapshot.timestamp = SDL_GetTicks();
    
    // Player info
    if (MyPlayer != nullptr) {
        snapshot.playerPosition = MyPlayer->position.tile;
        snapshot.playerHealth = MyPlayer->_pHitPoints >> 6;
        snapshot.playerMana = MyPlayer->_pMana >> 6;
        snapshot.currentLevel = MyPlayer->plrlevel;
    }
    
    // Monster analysis
    snapshot.monsters.totalMonsters = 0;
    for (const Monster& monster : Monsters) {
        if (monster.mode == MonsterMode::Stand || IsMonsterModeMove(monster.mode) ||
            monster.mode == MonsterMode::MeleeAttack || monster.mode == MonsterMode::RangedAttack ||
            monster.mode == MonsterMode::HitRecovery) {
            snapshot.monsters.totalMonsters++;
            
            // Clasificar por tipo (aproximado por nombre)
            const char* monsterName = monster.name().data();
            if (strstr(monsterName, "Spitter") != nullptr || strstr(monsterName, "Acid") != nullptr) {
                snapshot.monsters.spitters++;
            } else if (strstr(monsterName, "Skeleton") != nullptr) {
                snapshot.monsters.skeletons++;
            } else if (strstr(monsterName, "Zombie") != nullptr) {
                snapshot.monsters.zombies++;
            } else if (strstr(monsterName, "Goat") != nullptr) {
                snapshot.monsters.goatmen++;
            } else {
                snapshot.monsters.other++;
            }
            
            // Clasificar por estado
            switch (monster.mode) {
                case MonsterMode::MeleeAttack:
                case MonsterMode::RangedAttack:
                case MonsterMode::SpecialMeleeAttack:
                case MonsterMode::SpecialRangedAttack:
                    snapshot.monsters.attacking++;
                    break;
                case MonsterMode::MoveNorthwards:
                case MonsterMode::MoveSouthwards:
                case MonsterMode::MoveSideways:
                    snapshot.monsters.moving++;
                    break;
                default:
                    snapshot.monsters.idle++;
                    break;
            }
        }
    }
    
    // Missile analysis
    snapshot.missiles.totalMissiles = 0;
    for (const Missile& missile : Missiles) {
        if (missile._mitype != MissileID::Null) {
            snapshot.missiles.totalMissiles++;
            
            switch (missile._mitype) {
                case MissileID::InfernoControl:
                    snapshot.missiles.infernoControls++;
                    break;
                case MissileID::Inferno:
                    snapshot.missiles.infernos++;
                    break;
                case MissileID::Fireball:
                    snapshot.missiles.fireballs++;
                    break;
                case MissileID::LightningControl:
                    snapshot.missiles.lightningControls++;
                    break;
                case MissileID::ChainLightning:
                    snapshot.missiles.chainLightnings++;
                    break;
                case MissileID::Acid:
                case MissileID::AcidSplat:
                case MissileID::AcidPuddle:
                    snapshot.missiles.spitterProjectiles++;
                    break;
                default:
                    snapshot.missiles.other++;
                    break;
            }
        }
    }
    
    // Click tracking
    uint32_t currentTime = SDL_GetTicks();
    snapshot.clicksInLastSecond = 0;
    for (uint32_t clickTime : g_crashDiagnostics.recentClicks) {
        if (currentTime - clickTime <= 1000) { // Último segundo
            snapshot.clicksInLastSecond++;
        }
    }
    snapshot.totalClicksThisSession = g_crashDiagnostics.totalClicks;
    snapshot.lastClickTime = g_crashDiagnostics.recentClicks.empty() ? 0 : g_crashDiagnostics.recentClicks.back();
    
    // Engine state
    snapshot.activeObjects = ActiveObjectCount;
    snapshot.activeItems = ActiveItemCount;
    
    return snapshot;
}

void UpdateCrashDiagnostics()
{
    if (!g_crashDiagnostics.enabled) return;
    
    uint32_t currentTime = SDL_GetTicks();
    
    // Capturar snapshot periódico
    if (currentTime - g_crashDiagnostics.lastSnapshotTime >= CrashDiagnostics::SNAPSHOT_INTERVAL_MS) {
        GameStateSnapshot snapshot = CaptureGameStateSnapshot();
        
        g_crashDiagnostics.snapshots.push_back(snapshot);
        g_crashDiagnostics.lastSnapshotTime = currentTime;
        g_crashDiagnostics.totalSnapshots++;
        
        // Mantener solo las últimas entradas
        if (g_crashDiagnostics.snapshots.size() > CrashDiagnostics::MAX_LOG_ENTRIES) {
            g_crashDiagnostics.snapshots.erase(g_crashDiagnostics.snapshots.begin());
        }
        
        // Actualizar estadísticas máximas
        if (snapshot.missiles.totalMissiles > g_crashDiagnostics.maxMissilesRecorded) {
            g_crashDiagnostics.maxMissilesRecorded = snapshot.missiles.totalMissiles;
        }
        if (snapshot.monsters.totalMonsters > g_crashDiagnostics.maxMonstersRecorded) {
            g_crashDiagnostics.maxMonstersRecorded = snapshot.monsters.totalMonsters;
        }
        
        // Logging verbose
        if (g_crashDiagnostics.verboseLogging) {
            LogVerbose("🔍 Snapshot #{}: Missiles={}, Monsters={}, Clicks/sec={}", 
                      g_crashDiagnostics.totalSnapshots,
                      snapshot.missiles.totalMissiles,
                      snapshot.monsters.totalMonsters,
                      snapshot.clicksInLastSecond);
        }
    }
    
    // Limpiar clicks antiguos
    auto& clicks = g_crashDiagnostics.recentClicks;
    clicks.erase(std::remove_if(clicks.begin(), clicks.end(), 
                               [currentTime](uint32_t clickTime) {
                                   return currentTime - clickTime > 5000; // Mantener últimos 5 segundos
                               }), clicks.end());
    
    // Limpiar clicks de Inferno antiguos
    auto& infernoClicks = g_crashDiagnostics.recentInfernoClicks;
    infernoClicks.erase(std::remove_if(infernoClicks.begin(), infernoClicks.end(), 
                                      [currentTime](uint32_t clickTime) {
                                          return currentTime - clickTime > 5000; // Mantener últimos 5 segundos
                                      }), infernoClicks.end());
    
    // Análisis de patrones críticos
    AnalyzeCrashPatterns();
}

void RegisterPlayerClick()
{
    if (!g_crashDiagnostics.enabled) return;
    
    uint32_t currentTime = SDL_GetTicks();
    g_crashDiagnostics.recentClicks.push_back(currentTime);
    g_crashDiagnostics.totalClicks++;
    
    // Detectar spam de clicks
    int clicksInLastSecond = 0;
    for (uint32_t clickTime : g_crashDiagnostics.recentClicks) {
        if (currentTime - clickTime <= 1000) {
            clicksInLastSecond++;
        }
    }
    
    if (clicksInLastSecond >= CrashDiagnostics::CLICK_SPAM_THRESHOLD) {
        RegisterCrashEvent("CLICK_SPAM", 
                          fmt::format("High click frequency detected: {} clicks/sec", clicksInLastSecond),
                          fmt::format("Total session clicks: {}", g_crashDiagnostics.totalClicks));
    }
}

void RegisterInfernoClick()
{
    if (!g_crashDiagnostics.enabled) return;
    
    uint32_t currentTime = SDL_GetTicks();
    g_crashDiagnostics.recentInfernoClicks.push_back(currentTime);
    g_crashDiagnostics.totalInfernoClicks++;
    
    // También registrar como click normal
    RegisterPlayerClick();
    
    // Detectar spam específico de Inferno
    int infernoClicksInLastSecond = 0;
    for (uint32_t clickTime : g_crashDiagnostics.recentInfernoClicks) {
        if (currentTime - clickTime <= 1000) {
            infernoClicksInLastSecond++;
        }
    }
    
    if (infernoClicksInLastSecond >= CrashDiagnostics::INFERNO_CLICK_THRESHOLD) {
        RegisterCrashEvent("INFERNO_SPAM", 
                          fmt::format("High Inferno click frequency detected: {} clicks/sec", infernoClicksInLastSecond),
                          fmt::format("Total Inferno clicks: {}, Active InfernoControls: {}", 
                                    g_crashDiagnostics.totalInfernoClicks, CountActiveInfernoControls()));
        
        // Forzar guardado de diagnósticos en caso de crash inminente
        SaveDiagnosticsToFile();
    }
}

int CountActiveInfernoControls()
{
    int count = 0;
    for (const auto &missile : Missiles) {
        if (missile._mitype == MissileID::InfernoControl) {
            count++;
        }
    }
    return count;
}

void RegisterCrashEvent(const std::string& eventType, const std::string& description, const std::string& additionalInfo)
{
    if (!g_crashDiagnostics.enabled) return;
    
    CrashEvent event;
    event.timestamp = SDL_GetTicks();
    event.eventType = eventType;
    event.description = description;
    event.additionalInfo = additionalInfo;
    event.stateBefore = CaptureGameStateSnapshot();
    
    g_crashDiagnostics.events.push_back(event);
    g_crashDiagnostics.totalEvents++;
    
    // Mantener solo las últimas entradas
    if (g_crashDiagnostics.events.size() > CrashDiagnostics::MAX_LOG_ENTRIES) {
        g_crashDiagnostics.events.erase(g_crashDiagnostics.events.begin());
    }
    
    // Log del evento
    LogInfo("🚨 CRASH EVENT: {} - {}", eventType, description);
    if (!additionalInfo.empty()) {
        LogInfo("   Additional info: {}", additionalInfo);
    }
    
    // Si es un evento crítico, guardar diagnósticos
    if (eventType == "CRITICAL_STATE" || eventType == "CRASH_IMMINENT" || eventType == "CLICK_SPAM") {
        SaveDiagnosticsToFile();
    }
}

void AnalyzeCrashPatterns()
{
    if (g_crashDiagnostics.snapshots.empty()) return;
    
    const GameStateSnapshot& latest = g_crashDiagnostics.snapshots.back();
    
    // Detectar estado crítico
    bool wasCritical = g_crashDiagnostics.criticalStateDetected;
    g_crashDiagnostics.criticalStateDetected = false;
    
    std::string criticalReason;
    
    // Patrón 1: Demasiados missiles
    if (latest.missiles.totalMissiles >= CrashDiagnostics::CRITICAL_MISSILE_COUNT) {
        g_crashDiagnostics.criticalStateDetected = true;
        criticalReason += fmt::format("High missile count: {}; ", latest.missiles.totalMissiles);
    }
    
    // Patrón 2: Demasiados monsters
    if (latest.monsters.totalMonsters >= CrashDiagnostics::CRITICAL_MONSTER_COUNT) {
        g_crashDiagnostics.criticalStateDetected = true;
        criticalReason += fmt::format("High monster count: {}; ", latest.monsters.totalMonsters);
    }
    
    // Patrón 3: Spam de clicks + muchos missiles
    if (latest.clicksInLastSecond >= CrashDiagnostics::CLICK_SPAM_THRESHOLD && 
        latest.missiles.totalMissiles >= 10) {
        g_crashDiagnostics.criticalStateDetected = true;
        criticalReason += fmt::format("Click spam + missiles: {}/sec + {} missiles; ", 
                                    latest.clicksInLastSecond, latest.missiles.totalMissiles);
    }
    
    // Patrón 4: Combinación específica Spitters + InfernoControl
    if (latest.monsters.spitters >= 3 && latest.missiles.infernoControls >= 2) {
        g_crashDiagnostics.criticalStateDetected = true;
        criticalReason += fmt::format("Spitters + Inferno: {} spitters + {} inferno controls; ", 
                                    latest.monsters.spitters, latest.missiles.infernoControls);
    }
    
    // Si entramos en estado crítico, registrar evento
    if (g_crashDiagnostics.criticalStateDetected && !wasCritical) {
        g_crashDiagnostics.lastCriticalPattern = criticalReason;
        RegisterCrashEvent("CRITICAL_STATE", "Critical crash pattern detected", criticalReason);
    }
}

bool IsInCriticalState()
{
    return g_crashDiagnostics.criticalStateDetected;
}

std::string GenerateDiagnosticsReport()
{
    std::string report;
    uint32_t currentTime = SDL_GetTicks();
    uint32_t sessionDuration = currentTime - g_crashDiagnostics.sessionStartTime;
    
    report += "🔍 CRASH DIAGNOSTICS REPORT\n";
    report += std::string(50, '=') + "\n\n";
    
    report += fmt::format("Session Duration: {:.2f} seconds\n", sessionDuration / 1000.0f);
    report += fmt::format("Total Snapshots: {}\n", g_crashDiagnostics.totalSnapshots);
    report += fmt::format("Total Events: {}\n", g_crashDiagnostics.totalEvents);
    report += fmt::format("Total Clicks: {}\n", g_crashDiagnostics.totalClicks);
    report += fmt::format("Max Missiles Recorded: {}\n", g_crashDiagnostics.maxMissilesRecorded);
    report += fmt::format("Max Monsters Recorded: {}\n", g_crashDiagnostics.maxMonstersRecorded);
    report += fmt::format("Critical State: {}\n", g_crashDiagnostics.criticalStateDetected ? "YES" : "NO");
    
    if (g_crashDiagnostics.criticalStateDetected) {
        report += fmt::format("Last Critical Pattern: {}\n", g_crashDiagnostics.lastCriticalPattern);
    }
    
    report += "\n🚨 RECENT EVENTS:\n";
    report += std::string(30, '-') + "\n";
    
    int eventCount = 0;
    for (auto it = g_crashDiagnostics.events.rbegin(); 
         it != g_crashDiagnostics.events.rend() && eventCount < 10; 
         ++it, ++eventCount) {
        const CrashEvent& event = *it;
        uint32_t eventAge = currentTime - event.timestamp;
        report += fmt::format("[{:.2f}s ago] {}: {}\n", 
                            eventAge / 1000.0f, event.eventType, event.description);
        if (!event.additionalInfo.empty()) {
            report += fmt::format("  Info: {}\n", event.additionalInfo);
        }
    }
    
    if (!g_crashDiagnostics.snapshots.empty()) {
        const GameStateSnapshot& latest = g_crashDiagnostics.snapshots.back();
        report += "\n📊 CURRENT STATE:\n";
        report += std::string(30, '-') + "\n";
        report += fmt::format("Level: {}\n", latest.currentLevel);
        report += fmt::format("Player Position: ({}, {})\n", latest.playerPosition.x, latest.playerPosition.y);
        report += fmt::format("Player Health: {}\n", latest.playerHealth);
        report += fmt::format("Player Mana: {}\n", latest.playerMana);
        report += fmt::format("Total Missiles: {}\n", latest.missiles.totalMissiles);
        report += fmt::format("  - Inferno Controls: {}\n", latest.missiles.infernoControls);
        report += fmt::format("  - Infernos: {}\n", latest.missiles.infernos);
        report += fmt::format("  - Fireballs: {}\n", latest.missiles.fireballs);
        report += fmt::format("  - Spitter Projectiles: {}\n", latest.missiles.spitterProjectiles);
        report += fmt::format("Total Monsters: {}\n", latest.monsters.totalMonsters);
        report += fmt::format("  - Spitters: {}\n", latest.monsters.spitters);
        report += fmt::format("  - Skeletons: {}\n", latest.monsters.skeletons);
        report += fmt::format("  - Attacking: {}\n", latest.monsters.attacking);
        report += fmt::format("Clicks in Last Second: {}\n", latest.clicksInLastSecond);
    }
    
    return report;
}

void SaveDiagnosticsToFile()
{
    std::string filename = fmt::format("crash_diagnostics_{}.txt", SDL_GetTicks());
    std::string filepath = paths::PrefPath() + filename;
    
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << GenerateDiagnosticsReport();
        file.close();
        
        LogInfo("🔍 Crash diagnostics saved to: {}", filepath);
    } else {
        LogError("Failed to save crash diagnostics to: {}", filepath);
    }
}

const CrashDiagnosticsState& GetCrashDiagnosticsState()
{
    return g_crashDiagnostics;
}

void SetVerboseLogging(bool enabled)
{
    g_crashDiagnostics.verboseLogging = enabled;
    LogInfo("🔍 Crash diagnostics verbose logging: {}", enabled ? "ENABLED" : "DISABLED");
}

} // namespace devilution