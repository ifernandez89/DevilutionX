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
    // DISABLED: Crash diagnostics system disabled after successful crash fix
    // g_crashDiagnostics = CrashDiagnosticsState{};
    // g_crashDiagnostics.sessionStartTime = SDL_GetTicks();
    // g_crashDiagnostics.enabled = true;
    // g_crashDiagnostics.verboseLogging = true;
    // LogInfo("🔍 CRASH DIAGNOSTICS SYSTEM INITIALIZED");
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
    // DISABLED: Crash diagnostics system disabled after successful crash fix
    // All diagnostic updates are now disabled to prevent file generation
}

void RegisterPlayerClick()
{
    // DISABLED: Crash diagnostics system disabled after successful crash fix
}

void RegisterInfernoClick()
{
    // DISABLED: Crash diagnostics system disabled after successful crash fix
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
    // DISABLED: Crash diagnostics system disabled after successful crash fix
}

void AnalyzeCrashPatterns()
{
    // DISABLED: Crash diagnostics system disabled after successful crash fix
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
    // DISABLED: No longer saving diagnostic files after successful crash fix
    // std::string filename = fmt::format("crash_diagnostics_{}.txt", SDL_GetTicks());
    // std::string filepath = paths::PrefPath() + filename;
    // ... (file creation disabled)
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