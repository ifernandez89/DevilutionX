#pragma once

/**
 * @file safety_math.h
 * 
 * SAFETY LAYER - Phase 1: Centralized Mathematical Clamping
 * 
 * PRINCIPIO RECTOR: Todo daño y timer pasa por aquí.
 * 
 * Este header proporciona clamp matemático centralizado para prevenir overflow.
 * - Constantes bien definidas para límites máximos
 * - Funciones inline para rendimiento óptimo
 * - Validación de rangos estilo Diablo
 * - Sin dependencias externas
 */

#include <cstdint>
#include <algorithm>

namespace devilution {

// ============================================================================
// 🧱 1.2 CONSTANTES DE LÍMITES MÁXIMOS
// ============================================================================

// Límites conservadores basados en el rango de int16_t para compatibilidad
constexpr int32_t MAX_DAMAGE = 32767;      // Máximo daño seguro
constexpr int32_t MAX_TIMER = 32767;       // Máximo timer seguro
constexpr int32_t MAX_HITPOINTS = 32767;   // Máximo HP seguro
constexpr int32_t MAX_MANA = 32767;        // Máximo mana seguro
constexpr int32_t MAX_EXPERIENCE = 2000000000; // Máximo XP seguro (2 billones)

// Límites mínimos
constexpr int32_t MIN_DAMAGE = 0;
constexpr int32_t MIN_TIMER = 0;
constexpr int32_t MIN_HITPOINTS = 0;
constexpr int32_t MIN_MANA = 0;

// ============================================================================
// 🧱 FUNCIONES DE CLAMP CENTRALIZADAS
// ============================================================================

/**
 * REGLA CRÍTICA: Todo daño pasa por SafeDamage()
 * Previene overflow y underflow en cálculos de daño.
 */
inline int32_t SafeDamage(int32_t dmg) {
    if (dmg < MIN_DAMAGE) return MIN_DAMAGE;
    if (dmg > MAX_DAMAGE) return MAX_DAMAGE;
    return dmg;
}

/**
 * REGLA CRÍTICA: Todo timer pasa por SafeTimer()
 * Previene overflow en timers y delays.
 */
inline int32_t SafeTimer(int32_t timer) {
    if (timer < MIN_TIMER) return MIN_TIMER;
    if (timer > MAX_TIMER) return MAX_TIMER;
    return timer;
}

/**
 * Clamp seguro para hitpoints
 */
inline int32_t SafeHitPoints(int32_t hp) {
    if (hp < MIN_HITPOINTS) return MIN_HITPOINTS;
    if (hp > MAX_HITPOINTS) return MAX_HITPOINTS;
    return hp;
}

/**
 * Clamp seguro para mana
 */
inline int32_t SafeMana(int32_t mana) {
    if (mana < MIN_MANA) return MIN_MANA;
    if (mana > MAX_MANA) return MAX_MANA;
    return mana;
}

/**
 * Clamp seguro para experiencia
 */
inline int32_t SafeExperience(int32_t exp) {
    if (exp < 0) return 0;
    if (exp > MAX_EXPERIENCE) return MAX_EXPERIENCE;
    return exp;
}

// ============================================================================
// 🧱 OPERACIONES MATEMÁTICAS SEGURAS
// ============================================================================

/**
 * Suma segura con protección contra overflow
 */
inline int32_t SafeAdd(int32_t a, int32_t b, int32_t maxVal = MAX_DAMAGE) {
    // Verificar overflow antes de sumar
    if (a > maxVal - b) {
        return maxVal; // Clamp al máximo en caso de overflow
    }
    return a + b;
}

/**
 * Resta segura con protección contra underflow
 */
inline int32_t SafeSubtract(int32_t a, int32_t b, int32_t minVal = 0) {
    // Verificar underflow antes de restar
    if (a < minVal + b) {
        return minVal; // Clamp al mínimo en caso de underflow
    }
    return a - b;
}

/**
 * Multiplicación segura con protección contra overflow
 */
inline int32_t SafeMultiply(int32_t a, int32_t b, int32_t maxVal = MAX_DAMAGE) {
    // Verificar overflow antes de multiplicar
    if (a != 0 && b > maxVal / a) {
        return maxVal; // Clamp al máximo en caso de overflow
    }
    return a * b;
}

/**
 * División segura con protección contra división por cero
 */
inline int32_t SafeDivide(int32_t a, int32_t b, int32_t defaultVal = 0) {
    if (b == 0) {
        return defaultVal; // Retorna valor por defecto si división por cero
    }
    return a / b;
}

// ============================================================================
// 🧱 MACROS PARA COMPATIBILIDAD CON CÓDIGO EXISTENTE
// ============================================================================

/**
 * Macros para reemplazar operaciones matemáticas peligrosas
 */
#define SAFE_DAMAGE(x) SafeDamage(x)
#define SAFE_TIMER(x) SafeTimer(x)
#define SAFE_HP(x) SafeHitPoints(x)
#define SAFE_MANA(x) SafeMana(x)

/**
 * Macro para clamp genérico
 */
#define SAFE_CLAMP(value, min_val, max_val) \
    ((value) < (min_val) ? (min_val) : ((value) > (max_val) ? (max_val) : (value)))

// ============================================================================
// 🧱 VALIDADORES DE RANGO
// ============================================================================

/**
 * Verifica si un valor está en rango seguro para daño
 */
inline bool IsValidDamage(int32_t dmg) {
    return dmg >= MIN_DAMAGE && dmg <= MAX_DAMAGE;
}

/**
 * Verifica si un valor está en rango seguro para timer
 */
inline bool IsValidTimer(int32_t timer) {
    return timer >= MIN_TIMER && timer <= MAX_TIMER;
}

/**
 * Verifica si un valor está en rango seguro para HP
 */
inline bool IsValidHitPoints(int32_t hp) {
    return hp >= MIN_HITPOINTS && hp <= MAX_HITPOINTS;
}

} // namespace devilution