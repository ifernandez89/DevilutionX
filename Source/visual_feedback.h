/**
 * @file visual_feedback.h
 * 
 * 🎮 FASE V3 - FEEDBACK VISUAL REACTIVO 🎮
 * 
 * Sistema completo de feedback visual reactivo que responde a eventos del juego
 * con efectos visuales inmersivos y contextuales.
 */

#pragma once

#include <cstdint>
#include <array>

#ifdef USE_SDL3
#include <SDL3/SDL_pixels.h>
#else
#include <SDL.h>
#endif

#include "engine/point.hpp"

namespace devilution {

// Forward declarations
struct Player;
struct Monster;
struct Item;
enum class SpellID : int8_t;
enum class DamageType : uint8_t;

// ============================================================================
// 🎮 SISTEMA DE FEEDBACK VISUAL REACTIVO
// ============================================================================

/**
 * Tipos de efectos visuales reactivos
 */
enum class VisualFeedbackType {
    None,
    PlayerDamageFlash,      // Flash cuando el jugador recibe daño
    PlayerLowHealthPulse,   // Pulse cuando la vida está baja
    EliteMonsterPulse,      // Pulse en monstruos elite
    CriticalHitFlash,       // Flash en golpes críticos
    SpellCastGlow,          // Brillo al lanzar hechizos
    ItemPickupGlow,         // Brillo al recoger items
    LevelUpFlash,           // Flash al subir de nivel
    DeathFlash,             // Flash al morir
    BossEncounterPulse      // Pulse al encontrar jefes
};

/**
 * Configuración de un efecto visual
 */
struct VisualEffect {
    VisualFeedbackType type = VisualFeedbackType::None;
    uint32_t startTime = 0;
    uint32_t duration = 0;
    float intensity = 1.0f;
    float currentIntensity = 0.0f;
    SDL_Color color = { 255, 255, 255, 255 };
    Point position = { 0, 0 };
    bool active = false;
    bool fadeOut = true;
    
    // Parámetros específicos del efecto
    float pulseSpeed = 1.0f;
    float flashIntensity = 1.0f;
    int radius = 5;
};

/**
 * Estado global del sistema de feedback visual
 */
struct VisualFeedbackState {
    std::array<VisualEffect, 16> activeEffects;
    int activeEffectCount = 0;
    
    // Estado del jugador para efectos reactivos
    int lastPlayerHealth = 0;
    uint32_t lastDamageTime = 0;
    bool lowHealthWarning = false;
    
    // Configuración global
    bool enabled = true;
    float globalIntensity = 1.0f;
    bool contextualEffects = true;
};

// ============================================================================
// 🎯 API PRINCIPAL DEL SISTEMA
// ============================================================================

/**
 * Inicializa el sistema de feedback visual reactivo
 */
void InitVisualFeedback();

/**
 * Actualiza el sistema de feedback visual (llamar una vez por frame)
 */
void UpdateVisualFeedback();

/**
 * Limpia el sistema de feedback visual
 */
void CleanupVisualFeedback();

/**
 * Resetea el estado interno del sistema para nueva transición
 * Mantiene el sistema habilitado pero limpia efectos activos
 */
void ResetVisualFeedbackState();

/**
 * Habilita/deshabilita el sistema de feedback visual
 */
void SetVisualFeedbackEnabled(bool enabled);

/**
 * Establece la intensidad global de los efectos
 */
void SetVisualFeedbackIntensity(float intensity);

// ============================================================================
// 🎮 EFECTOS ESPECÍFICOS - FASE V3
// ============================================================================

/**
 * 🎯 V3.1 - FLASH DE DAÑO AL JUGADOR
 * Activa un flash visual cuando el jugador recibe daño
 */
void TriggerPlayerDamageFlash(const Player &player, int damage, DamageType damageType);

/**
 * 🎯 V3.2 - PULSE EN MONSTRUOS ELITE
 * Activa un pulse visual en monstruos elite para destacarlos
 */
void TriggerEliteMonsterPulse(const Monster &monster);

/**
 * 🎯 V3.3 - FEEDBACK REACTIVO CONTEXTUAL
 * Activa efectos contextuales basados en el estado del juego
 */
void TriggerContextualFeedback(VisualFeedbackType type, Point position, float intensity = 1.0f);

/**
 * 🎯 V3.4 - PULSE DE VIDA BAJA
 * Activa un pulse cuando la vida del jugador está baja
 */
void UpdateLowHealthPulse(const Player &player);

/**
 * 🎯 V3.5 - FLASH DE GOLPE CRÍTICO
 * Activa un flash cuando se produce un golpe crítico
 */
void TriggerCriticalHitFlash(Point position, int damage);

/**
 * 🎯 V3.6 - BRILLO DE HECHIZO
 * Activa un brillo cuando se lanza un hechizo
 */
void TriggerSpellCastGlow(const Player &player, SpellID spellId);

/**
 * 🎯 V3.7 - FLASH DE MUERTE
 * Efecto dramático cuando el jugador muere
 */
void TriggerDeathFlash(const Player &player);

/**
 * 🎯 V3.8 - PULSE DE JEFE
 * Efecto especial al encontrar jefes únicos
 */
void TriggerBossEncounterPulse(const Monster &boss);

/**
 * 🎯 V3.9 - BRILLO DE ITEM
 * Efecto al recoger items importantes
 */
void TriggerItemPickupGlow(Point position, const Item &item);

/**
 * 🎯 V3.10 - FLASH DE NIVEL
 * Efecto celebratorio al subir de nivel
 */
void TriggerLevelUpFlash(const Player &player);

// ============================================================================
// 🎨 INTEGRACIÓN CON SISTEMA DE PALETAS
// ============================================================================

/**
 * Aplica efectos visuales activos a la paleta del sistema
 * Se integra con el sistema de paletas de FASE V2
 */
void ApplyVisualFeedbackToPalette(SDL_Color *palette);

/**
 * Calcula la intensidad del efecto en una posición específica
 */
float GetEffectIntensityAtPosition(Point position, const VisualEffect &effect);

/**
 * Mezcla múltiples efectos visuales de forma inteligente
 */
SDL_Color BlendVisualEffects(SDL_Color baseColor, Point position);

// ============================================================================
// 🔧 CONFIGURACIÓN Y PRESETS
// ============================================================================

/**
 * Preset conservador - efectos sutiles
 */
void ApplyConservativeVisualPreset();

/**
 * Preset balanceado - efectos moderados (recomendado)
 */
void ApplyBalancedVisualPreset();

/**
 * Preset intenso - efectos dramáticos
 */
void ApplyIntenseVisualPreset();

/**
 * Preset para debugging - efectos visibles y marcados
 */
void ApplyDebugVisualPreset();

// ============================================================================
// 📊 ESTADÍSTICAS Y MÉTRICAS
// ============================================================================

/**
 * Estadísticas del sistema de feedback visual
 */
struct VisualFeedbackStats {
    uint32_t totalEffectsTriggered = 0;
    uint32_t activeEffects = 0;
    uint32_t damageFlashesTriggered = 0;
    uint32_t elitePulsesTriggered = 0;
    uint32_t contextualEffectsTriggered = 0;
    float averageIntensity = 0.0f;
    uint32_t averageProcessingTimeUs = 0;
};

/**
 * Obtiene estadísticas del sistema
 */
VisualFeedbackStats GetVisualFeedbackStats();

/**
 * Resetea estadísticas del sistema
 */
void ResetVisualFeedbackStats();

// ============================================================================
// 🔍 DEBUGGING Y DIAGNÓSTICO
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime estadísticas del sistema de feedback visual
 */
void DebugPrintVisualFeedbackStats();

/**
 * Habilita/deshabilita logging detallado
 */
void SetVisualFeedbackDebugLogging(bool enabled);

/**
 * Ejecuta test de stress del sistema
 */
void RunVisualFeedbackStressTest();
#endif

} // namespace devilution