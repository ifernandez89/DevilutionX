#pragma once

/**
 * @file audio_throttling.h
 * 
 * AUDIO ENHANCEMENT - Phase A1: Audio Throttling and Priority System
 * 
 * 🔊 OBJETIVO: Eliminar spam, mejorar claridad, subir FPS indirectamente
 * 
 * Sistema inteligente de throttling de audio que:
 * - Elimina spam de sonidos (especialmente Inferno)
 * - Mejora claridad auditiva
 * - Reduce carga de CPU
 * - Mantiene feedback importante
 * 
 * FILOSOFÍA: "Audio debe degradar antes que gameplay"
 */

#include "sound_effect_enums.h"
#include <cstdint>
#include <unordered_map>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// 🔊 SISTEMA DE THROTTLING DE AUDIO
// ============================================================================

/**
 * Estructura para controlar cooldowns de sonidos específicos
 */
struct SoundCooldown {
    uint32_t lastPlayTime = 0;    // Último tick cuando se reprodujo
    uint32_t cooldownMs = 0;      // Cooldown en milisegundos
    uint8_t priority = 0;         // Prioridad del sonido (0-255)
    uint8_t maxPerFrame = 255;    // Máximo por frame (255 = sin límite)
    uint8_t currentFrameCount = 0; // Contador actual del frame
};

/**
 * Categorías de prioridad de sonidos
 */
enum class AudioPriority : uint8_t {
    CRITICAL = 255,    // UI, muerte del jugador, eventos críticos
    HIGH = 200,        // Ataques del jugador, daño recibido
    MEDIUM = 150,      // Spells, efectos importantes
    LOW = 100,         // Ambiente, pasos, efectos menores
    SPAM = 50          // Efectos repetitivos que pueden ser throttled
};

/**
 * Configuración predefinida para sonidos específicos
 */
struct AudioThrottleConfig {
    SfxID soundId;
    uint32_t cooldownMs;
    AudioPriority priority;
    uint8_t maxPerFrame;
};

// ============================================================================
// 🎯 CONFIGURACIONES ESPECÍFICAS POR SONIDO
// ============================================================================

/**
 * Configuraciones de throttling para sonidos problemáticos
 */
constexpr AudioThrottleConfig AUDIO_THROTTLE_CONFIGS[] = {
    // 🔥 INFERNO - El más problemático
    { SfxID::SpellInferno, 200, AudioPriority::MEDIUM, 1 },
    
    // ⚡ LIGHTNING - Puede hacer spam
    { SfxID::SpellLightning, 150, AudioPriority::MEDIUM, 2 },
    
    // 🏹 PROYECTILES - Múltiples simultáneos
    { SfxID::ShootBow, 100, AudioPriority::LOW, 3 },
    { SfxID::ShootBow2, 100, AudioPriority::LOW, 3 },
    
    // ⚔️ COMBATE - Importante pero puede hacer spam
    { SfxID::Swing, 80, AudioPriority::HIGH, 4 },
    
    // 👣 PASOS - Muy frecuentes
    { SfxID::Walk, 50, AudioPriority::LOW, 2 },
    
    // 💥 IMPACTOS - Múltiples en combate
    { SfxID::SpellFireHit, 60, AudioPriority::MEDIUM, 3 },
    
    // 🎯 TRAPS - Pueden activarse en masa
    { SfxID::TriggerTrap, 150, AudioPriority::HIGH, 2 },
    
    // 🔮 OTROS SPELLS PROBLEMÁTICOS
    { SfxID::SpellFirebolt, 120, AudioPriority::MEDIUM, 2 },
    { SfxID::SpellFireWall, 180, AudioPriority::MEDIUM, 1 },
    { SfxID::SpellLightningHit, 150, AudioPriority::MEDIUM, 2 },
};

// ============================================================================
// 🧠 API DEL SISTEMA DE THROTTLING
// ============================================================================

/**
 * Inicializa el sistema de throttling de audio
 */
void InitAudioThrottling();

/**
 * Verifica si un sonido puede ser reproducido según las reglas de throttling
 * 
 * @param soundId ID del sonido a verificar
 * @return true si puede reproducirse, false si debe ser throttled
 */
bool CanPlaySound(SfxID soundId);

/**
 * Registra que un sonido fue reproducido (actualiza cooldowns)
 * 
 * @param soundId ID del sonido que se reprodujo
 */
void RegisterSoundPlayed(SfxID soundId);

/**
 * Actualiza contadores por frame (llamar una vez por frame)
 */
void UpdateAudioThrottlingFrame();

/**
 * Obtiene estadísticas del sistema de throttling
 */
struct AudioThrottlingStats {
    uint32_t totalSoundsRequested = 0;
    uint32_t totalSoundsBlocked = 0;
    uint32_t totalSoundsPlayed = 0;
    float blockingPercentage = 0.0f;
};

/**
 * Obtiene estadísticas actuales del throttling
 */
AudioThrottlingStats GetAudioThrottlingStats();

/**
 * Resetea estadísticas del throttling
 */
void ResetAudioThrottlingStats();

// ============================================================================
// 🔧 CONFIGURACIÓN AVANZADA
// ============================================================================

/**
 * Configura throttling personalizado para un sonido específico
 * 
 * @param soundId ID del sonido
 * @param cooldownMs Cooldown en milisegundos
 * @param priority Prioridad del sonido
 * @param maxPerFrame Máximo por frame
 */
void SetSoundThrottling(SfxID soundId, uint32_t cooldownMs, AudioPriority priority, uint8_t maxPerFrame = 255);

/**
 * Obtiene la configuración actual de throttling para un sonido
 * 
 * @param soundId ID del sonido
 * @return Configuración actual o nullptr si no está configurado
 */
const SoundCooldown* GetSoundThrottling(SfxID soundId);

/**
 * Habilita/deshabilita el sistema de throttling globalmente
 * 
 * @param enabled true para habilitar, false para deshabilitar
 */
void SetAudioThrottlingEnabled(bool enabled);

/**
 * Verifica si el sistema de throttling está habilitado
 */
bool IsAudioThrottlingEnabled();

// ============================================================================
// 🎮 INTEGRACIÓN CON SAFETY LAYER
// ============================================================================

/**
 * Registra evento de throttling para métricas del Safety Layer
 */
void RecordAudioThrottlingEvent(SfxID soundId, bool wasBlocked);

/**
 * Obtiene métricas de audio para el Safety Layer
 */
struct AudioSafetyMetrics {
    uint32_t currentActiveSounds = 0;
    uint32_t soundsBlockedThisFrame = 0;
    uint32_t totalSoundsThisFrame = 0;
    bool isAudioOverloaded = false;
};

/**
 * Obtiene métricas de audio para integración con Safety Layer
 */
AudioSafetyMetrics GetAudioSafetyMetrics();

// ============================================================================
// 🔍 DEBUGGING Y LOGGING
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime estadísticas de throttling (solo en debug)
 */
void DebugPrintAudioThrottlingStats();

/**
 * Imprime configuración de throttling para un sonido (solo en debug)
 */
void DebugPrintSoundConfig(SfxID soundId);

/**
 * Habilita/deshabilita logging detallado de throttling
 */
void SetAudioThrottlingDebugLogging(bool enabled);
#endif

/**
 * Inicialización completa con todas las configuraciones recomendadas
 */
void InitRecommendedAudioThrottling();

} // namespace devilution