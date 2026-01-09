#pragma once

/**
 * @file enhanced_audio.h
 * 
 * AUDIO ENHANCEMENT - Enhanced Audio System with Throttling
 * 
 * 🔊 SISTEMA DE AUDIO MEJORADO que integra:
 * - Throttling inteligente
 * - Spatial audio fake
 * - Variación de pitch/volume
 * - Priorización automática
 * 
 * Este header proporciona wrappers mejorados para las funciones de audio existentes.
 */

#include "audio_throttling.h"
#include "effects.h"
#include "engine/point.hpp"

namespace devilution {

// ============================================================================
// 🔊 API DE AUDIO MEJORADO
// ============================================================================

/**
 * Inicializa el sistema de audio mejorado
 * Debe llamarse después de la inicialización normal de audio
 */
void InitEnhancedAudio();

/**
 * Actualiza el sistema de audio mejorado (llamar una vez por frame)
 */
void UpdateEnhancedAudio();

/**
 * Limpia el sistema de audio mejorado
 */
void CleanupEnhancedAudio();

// ============================================================================
// 🎯 WRAPPERS MEJORADOS PARA FUNCIONES DE AUDIO
// ============================================================================

/**
 * Versión mejorada de PlaySFX con throttling automático
 * 
 * @param soundId ID del sonido a reproducir
 * @return true si el sonido se reprodujo, false si fue throttled
 */
bool EnhancedPlaySFX(SfxID soundId);

/**
 * Versión mejorada de PlaySfxLoc con throttling y spatial audio fake
 * 
 * @param soundId ID del sonido a reproducir
 * @param position Posición del sonido en el mundo
 * @param randomizeByCategory Si randomizar por categoría (parámetro original)
 * @return true si el sonido se reprodujo, false si fue throttled
 */
bool EnhancedPlaySfxLoc(SfxID soundId, Point position, bool randomizeByCategory = true);

// ============================================================================
// 🎵 SPATIAL AUDIO FAKE (FASE A2)
// ============================================================================

/**
 * Configuración para spatial audio fake
 */
struct SpatialAudioConfig {
    int maxDistance = 25;        // Distancia máxima audible
    int volumeFalloff = 64;      // Factor de caída de volumen
    int panFactor = 256;         // Factor de paneo estéreo
    bool enabled = true;         // Habilitar spatial audio
};

/**
 * Configura el sistema de spatial audio fake
 */
void SetSpatialAudioConfig(const SpatialAudioConfig& config);

/**
 * Obtiene la configuración actual de spatial audio
 */
SpatialAudioConfig GetSpatialAudioConfig();

/**
 * Calcula volumen y paneo basado en posición (spatial audio fake)
 * 
 * @param soundPosition Posición del sonido
 * @param playerPosition Posición del jugador
 * @param baseVolume Volumen base del sonido
 * @param outVolume [out] Volumen calculado
 * @param outPan [out] Paneo calculado
 * @return true si el sonido debe reproducirse, false si está fuera de rango
 */
bool CalculateSpatialAudio(Point soundPosition, Point playerPosition, int baseVolume, int* outVolume, int* outPan);

// ============================================================================
// 🎶 VARIACIÓN DE AUDIO (FASE A3)
// ============================================================================

/**
 * Configuración para variación de audio
 */
struct AudioVariationConfig {
    int pitchVariationRange = 4;    // Rango de variación de pitch (-N a +N)
    int volumeVariationRange = 10;  // Rango de variación de volumen (-N a +N)
    bool enabled = true;            // Habilitar variación
    float probability = 0.7f;       // Probabilidad de aplicar variación (0.0-1.0)
};

/**
 * Configura el sistema de variación de audio
 */
void SetAudioVariationConfig(const AudioVariationConfig& config);

/**
 * Obtiene la configuración actual de variación de audio
 */
AudioVariationConfig GetAudioVariationConfig();

/**
 * Aplica variación aleatoria a volumen y pitch
 * 
 * @param baseVolume Volumen base
 * @param basePitch Pitch base (no usado en implementación actual)
 * @param outVolume [out] Volumen con variación aplicada
 * @param outPitch [out] Pitch con variación aplicada (para futuro uso)
 */
void ApplyAudioVariation(int baseVolume, int basePitch, int* outVolume, int* outPitch);

// ============================================================================
// 🎮 INTEGRACIÓN CON SISTEMA EXISTENTE
// ============================================================================

/**
 * Habilita/deshabilita el sistema de audio mejorado globalmente
 * Cuando está deshabilitado, usa las funciones originales
 */
void SetEnhancedAudioEnabled(bool enabled);

/**
 * Verifica si el sistema de audio mejorado está habilitado
 */
bool IsEnhancedAudioEnabled();

// ============================================================================
// 📊 ESTADÍSTICAS Y MÉTRICAS
// ============================================================================

/**
 * Estadísticas del sistema de audio mejorado
 */
struct EnhancedAudioStats {
    // Throttling
    uint32_t totalSoundsRequested = 0;
    uint32_t totalSoundsPlayed = 0;
    uint32_t totalSoundsThrottled = 0;
    float throttlingPercentage = 0.0f;
    
    // Spatial audio
    uint32_t spatialSoundsProcessed = 0;
    uint32_t spatialSoundsOutOfRange = 0;
    
    // Variación
    uint32_t variationsApplied = 0;
    
    // Performance
    uint32_t averageProcessingTimeUs = 0;  // Microsegundos
    uint32_t peakProcessingTimeUs = 0;
};

/**
 * Obtiene estadísticas del sistema de audio mejorado
 */
EnhancedAudioStats GetEnhancedAudioStats();

/**
 * Resetea estadísticas del sistema de audio mejorado
 */
void ResetEnhancedAudioStats();

// ============================================================================
// 🔧 CONFIGURACIÓN AVANZADA
// ============================================================================

/**
 * Configuración completa del sistema de audio mejorado
 */
struct EnhancedAudioConfig {
    bool throttlingEnabled = true;
    bool spatialAudioEnabled = true;
    bool audioVariationEnabled = true;
    
    SpatialAudioConfig spatialConfig;
    AudioVariationConfig variationConfig;
    
    // Configuración de performance
    bool enablePerformanceMetrics = false;
    uint32_t maxProcessingTimeUs = 1000;  // Máximo tiempo de procesamiento por sonido
};

/**
 * Configura el sistema completo de audio mejorado
 */
void ConfigureEnhancedAudio(const EnhancedAudioConfig& config);

/**
 * Obtiene la configuración actual del sistema
 */
EnhancedAudioConfig GetEnhancedAudioConfig();

// ============================================================================
// 🎯 PRESETS DE CONFIGURACIÓN
// ============================================================================

/**
 * Preset conservador - throttling mínimo, máxima compatibilidad
 */
void ApplyConservativeAudioPreset();

/**
 * Preset balanceado - configuración recomendada para la mayoría de casos
 */
void ApplyBalancedAudioPreset();

/**
 * Preset agresivo - throttling máximo para sistemas con problemas de performance
 */
void ApplyAggressiveAudioPreset();

/**
 * Preset para debugging - logging detallado, métricas habilitadas
 */
void ApplyDebugAudioPreset();

// ============================================================================
// 🔍 DEBUGGING Y DIAGNÓSTICO
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime estadísticas completas del sistema de audio mejorado
 */
void DebugPrintEnhancedAudioStats();

/**
 * Imprime configuración actual del sistema
 */
void DebugPrintEnhancedAudioConfig();

/**
 * Habilita/deshabilita logging detallado del sistema
 */
void SetEnhancedAudioDebugLogging(bool enabled);

/**
 * Ejecuta test de stress del sistema de audio
 */
void RunEnhancedAudioStressTest();
#endif

} // namespace devilution