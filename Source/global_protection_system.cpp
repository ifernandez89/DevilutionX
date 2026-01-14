/**
 * @file global_protection_system.cpp
 * 
 * GLOBAL PROTECTION SYSTEM - SIMPLIFIED IMPLEMENTATION
 * 
 * Sistema simplificado sin singletons ni clases complejas.
 */

#include "global_protection_system.h"
#include "missiles.h"
#include "utils/log.hpp"

namespace devilution {

// ============================================================================
// 🌍 ESTADO GLOBAL SIMPLE
// ============================================================================

namespace {

// Configuración
constexpr int MAX_MISSILES = 500;        // Límite seguro de missiles
constexpr int CRITICAL_THRESHOLD = 450;  // 90% del límite

// Estado
bool g_initialized = false;
bool g_inCriticalState = false;
int g_framesSinceCritical = 0;

} // anonymous namespace

// ============================================================================
// 🔧 IMPLEMENTACIÓN
// ============================================================================

void InitGlobalProtection()
{
    g_initialized = true;
    g_inCriticalState = false;
    g_framesSinceCritical = 0;
    LogVerbose("Global Protection System initialized (simplified)");
}

void ResetGlobalProtection()
{
    g_inCriticalState = false;
    g_framesSinceCritical = 0;
    LogVerbose("Global Protection System reset");
}

void UpdateGlobalProtection()
{
    if (!g_initialized) {
        InitGlobalProtection();
    }
    
    int currentMissiles = static_cast<int>(Missiles.size());
    
    // Detectar estado crítico
    bool wasCritical = g_inCriticalState;
    g_inCriticalState = currentMissiles >= CRITICAL_THRESHOLD;
    
    if (g_inCriticalState) {
        g_framesSinceCritical++;
    } else {
        g_framesSinceCritical = 0;
    }
    
    // Log solo en transiciones
    if (g_inCriticalState && !wasCritical) {
        LogVerbose("GPS: Entering critical state (missiles: {})", currentMissiles);
    } else if (!g_inCriticalState && wasCritical) {
        LogVerbose("GPS: Exiting critical state");
    }
}

bool CanCreateMissile()
{
    return static_cast<int>(Missiles.size()) < MAX_MISSILES;
}

bool CanCastSpell()
{
    // En estado crítico prolongado, bloquear nuevos spells
    if (g_framesSinceCritical > 30) { // ~0.5 segundos en crítico
        return false;
    }
    return true;
}

int GetCurrentMissileCount()
{
    return static_cast<int>(Missiles.size());
}

int GetMaxMissileLimit()
{
    return MAX_MISSILES;
}

bool IsInCriticalState()
{
    return g_inCriticalState;
}

} // namespace devilution
