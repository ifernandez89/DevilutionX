/**
 * @file global_protection_system.h
 * 
 * GLOBAL PROTECTION SYSTEM - SIMPLIFIED 2026
 * 
 * Sistema simplificado de protección que previene crashes.
 * 
 * FILOSOFÍA: "Simple es mejor que complejo"
 * - Sin singletons
 * - Sin clases complejas
 * - Funciones simples y directas
 * - Integrado con engine_health.h
 */

#pragma once

#include <cstdint>

namespace devilution {

// ============================================================================
// 🛡️ GLOBAL PROTECTION SYSTEM - SIMPLIFIED API
// ============================================================================

/**
 * Inicializa el sistema de protección global
 * Llamar una vez al inicio del juego
 */
void InitGlobalProtection();

/**
 * Resetea el sistema de protección (para nueva partida)
 */
void ResetGlobalProtection();

/**
 * Actualiza estadísticas del sistema cada frame
 * Llamar una vez por frame
 */
void UpdateGlobalProtection();

/**
 * Verifica si es seguro crear un missile
 * @return true si es seguro, false si estamos en límite
 */
bool CanCreateMissile();

/**
 * Verifica si es seguro castear un spell
 * @return true si es seguro
 */
bool CanCastSpell();

/**
 * Obtiene el conteo actual de missiles
 */
int GetCurrentMissileCount();

/**
 * Obtiene el límite máximo de missiles
 */
int GetMaxMissileLimit();

/**
 * Verifica si el sistema está en estado crítico
 */
bool IsInCriticalState();

// ============================================================================
// 🔧 MACRO DE CONVENIENCIA (compatibilidad con código existente)
// ============================================================================

// Macro GPS simplificado - solo actualiza stats
#define GPS_UPDATE() UpdateGlobalProtection()

} // namespace devilution
